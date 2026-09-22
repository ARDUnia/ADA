/*
 * internet.cpp - پیاده‌سازی مدیریت اتصال به اینترنت با اسکن شبکه
 * نسخه: ۱.۶ (EEPROM ایمن، بازیابی اتصال و NTP صحیح)
 */

#include "internet.h"
#include "Display.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// ============================================================
//  تنظیمات اولیه
// ============================================================
#define WIFI_RETRY_INTERVAL 10000
#define MAX_RETRY_COUNT 5

// Credential record: magic, lengths, fixed payload, CRC16.
const int WIFI_MAGIC_ADDR = 0;
const int WIFI_SSID_LEN_ADDR = 4;
const int WIFI_PASS_LEN_ADDR = 5;
const int WIFI_SSID_ADDR = 6;
const int WIFI_SSID_MAX = 32;
const int WIFI_PASS_ADDR = WIFI_SSID_ADDR + WIFI_SSID_MAX;
const int WIFI_PASS_MAX = 64;
const int WIFI_CRC_ADDR = WIFI_PASS_ADDR + WIFI_PASS_MAX;

// ============================================================
//  متغیرهای داخلی
// ============================================================
static ESP8266WebServer server(80);
static WiFiUDP udp;
static NTPClient ntpClient(udp, "pool.ntp.org", NTP_UTC_OFFSET_SECONDS,
                           NTP_UPDATE_INTERVAL_MS);

static bool wifiConnected = false;
static unsigned long lastWifiAttempt = 0;
static bool apModeActive = false;
static int retryCount = 0;
static bool ntpStarted = false;
static bool ntpTimeValid = false;
static bool routesRegistered = false;

// ============================================================
//  پرتوتایپ توابع داخلی
// ============================================================
void startAPMode();
void handleRoot();
void handleScan();
void handleSave();
void handleNotFound();
void readCredentialsFromEEPROM(String &ssid, String &pass);
void saveCredentialsToEEPROM(const String &ssid, const String &pass);
uint16_t credentialCrc(uint8_t ssidLen, uint8_t passLen);
bool readLegacyCredentials(String &ssid, String &pass);
String jsonEscape(const String &value);

// ============================================================
//  توابع عمومی
// ============================================================

void initInternet() {
    EEPROM.begin(EEPROM_SIZE);
    WiFi.mode(WIFI_AP_STA);
    WiFi.persistent(false);

    String ssid, pass;
    readCredentialsFromEEPROM(ssid, pass);

    if (ssid.length() > 0) {
        Serial.println("Found stored credentials. Trying to connect...");
        WiFi.begin(ssid.c_str(), pass.c_str());
        lastWifiAttempt = millis();
        retryCount = 0;
    } else {
        Serial.println("No valid credentials. Starting AP mode...");
        startAPMode();
    }
}

void handleInternet() {
    if (apModeActive) {
        server.handleClient();   // ← این خط برای پاسخ به درخواست‌ها ضروری است
        return;
    }

    unsigned long now = millis();

    if (WiFi.status() == WL_CONNECTED) {
        if (!wifiConnected) {
            wifiConnected = true;
            isNetworkConnected = true;
            retryCount = 0;
            Serial.print("Connected to WiFi: ");
            Serial.println(WiFi.SSID());
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
            if (!ntpStarted) {
                ntpClient.begin();
                ntpStarted = true;
            }
            ntpClient.setUpdateInterval(NTP_UPDATE_INTERVAL_MS);
            ntpTimeValid = ntpClient.forceUpdate() || ntpTimeValid;
        } else {
            ntpTimeValid = ntpClient.update() || ntpTimeValid;
        }
        return;
    }

    if (wifiConnected) {
        wifiConnected = false;
        isNetworkConnected = false;
        Serial.println("WiFi connection lost.");
    }

    if (now - lastWifiAttempt > WIFI_RETRY_INTERVAL && retryCount < MAX_RETRY_COUNT) {
        lastWifiAttempt = now;
        String ssid, pass;
        readCredentialsFromEEPROM(ssid, pass);
        
        if (ssid.length() > 0) {
            Serial.println("Retrying connection... (Attempt " + String(retryCount + 1) + "/" + String(MAX_RETRY_COUNT) + ")");
            WiFi.begin(ssid.c_str(), pass.c_str());
            retryCount++;
        } else {
            Serial.println("No credentials found. Starting AP mode...");
            startAPMode();
            return;
        }
    }

    if (retryCount >= MAX_RETRY_COUNT && now - lastWifiAttempt > WIFI_RETRY_INTERVAL) {
        Serial.println("Max retry attempts reached. Starting AP mode...");
        startAPMode();
    }
}

bool isWiFiConnected() {
    return wifiConnected;
}

String getWiFiStatus() {
    if (wifiConnected) {
        return "Connected to " + WiFi.SSID() + " (" + WiFi.localIP().toString() + ")";
    } else {
        return "Not connected";
    }
}

String getFormattedTime() {
    if (!wifiConnected || !ntpTimeValid) return "--:--:--";
    return ntpClient.getFormattedTime();
}

void startWiFiConfigPortal() {
    startAPMode();
}

// ============================================================
//  توابع داخلی
// ============================================================

void startAPMode() {
    Serial.println("📡 Starting AP mode...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    apModeActive = true;
    wifiConnected = false;
    isNetworkConnected = false;
    retryCount = 0;

    // تنظیم مسیرهای وب‌سرور
    if (!routesRegistered) {
        server.on("/", handleRoot);
        server.on("/scan", handleScan);
        server.on("/save", handleSave);
        server.onNotFound(handleNotFound);
        routesRegistered = true;
    }

    // شروع وب‌سرور
    server.begin();
    
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("🔑 Connect to WiFi: " + String(AP_SSID) + " and visit http://192.168.4.1");
}

void handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>ADA WiFi Config</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin: 20px auto; max-width: 500px; padding: 20px; background-color: #f5f5f5; border-radius: 10px; }
        h1 { color: #333; }
        input, select { width: 90%; padding: 12px; margin: 8px auto; border: 1px solid #ccc; border-radius: 5px; font-size: 16px; display: block; }
        button { padding: 12px 30px; font-size: 18px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; width: 100%; }
        button:hover { background-color: #45a049; }
        .network-list { max-height: 200px; overflow-y: auto; border: 1px solid #ccc; border-radius: 5px; margin: 10px 0; background: white; text-align: left; }
        .network-item { padding: 10px; border-bottom: 1px solid #eee; cursor: pointer; }
        .network-item:hover { background-color: #e0f7fa; }
        .network-item strong { display: block; }
        .network-item small { color: #666; }
        .status { margin-top: 20px; font-size: 14px; color: #666; }
        .manual-link { margin-top: 10px; font-size: 14px; }
        .manual-link a { color: #2196F3; cursor: pointer; }
        #manualInput { display: none; margin-top: 10px; }
        .loading { color: #888; font-style: italic; }
    </style>
</head>
<body>
    <h1>🔧 ADA WiFi Setup</h1>
    <p>Select your WiFi network from the list:</p>
    <div id="networkList" class="network-list"><div class="loading">Scanning networks...</div></div>
    <div class="manual-link"><a onclick="toggleManual()">+ Enter SSID manually</a></div>
    <div id="manualInput">
        <input type="text" id="ssidManual" maxlength="32" placeholder="WiFi SSID (manual)">
        <input type="password" id="passManual" maxlength="64" placeholder="Password">
        <button onclick="saveManual()">Save & Connect</button>
    </div>
    <div class="status"><span id="statusMsg"></span></div>
    <script>
        window.onload = function() { scanNetworks(); };
        function scanNetworks() {
            const list = document.getElementById('networkList');
            list.innerHTML = '<div class="loading">Scanning networks...</div>';
            fetch('/scan').then(r => r.json()).then(data => {
                if (data.length === 0) { list.innerHTML = '<div style="padding:10px;color:#888;">No networks found.</div>'; return; }
                list.textContent = '';
                data.forEach(net => {
                    const secure = net.encryptionType !== 0 ? '🔒' : '🔓';
                    const item = document.createElement('div');
                    item.className = 'network-item';
                    const name = document.createElement('strong');
                    name.textContent = net.ssid;
                    const details = document.createElement('small');
                    details.textContent = `${secure} ${net.rssi} dBm`;
                    item.append(name, details);
                    item.addEventListener('click', () => selectNetwork(net.ssid));
                    list.appendChild(item);
                });
            }).catch(() => { list.innerHTML = '<div style="padding:10px;color:red;">Error scanning.</div>'; });
        }
        function selectNetwork(ssid) { document.getElementById('ssidManual').value = ssid; toggleManual(); document.getElementById('passManual').focus(); }
        function toggleManual() { const d = document.getElementById('manualInput'); d.style.display = (d.style.display === 'none' || d.style.display === '') ? 'block' : 'none'; }
        function saveManual() {
            const ssid = document.getElementById('ssidManual').value;
            const pass = document.getElementById('passManual').value;
            if (!ssid) { document.getElementById('statusMsg').innerHTML = '⚠️ Enter SSID'; return; }
            document.getElementById('statusMsg').innerHTML = '⏳ Saving...';
            fetch('/save', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: 'ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass) })
            .then(() => { document.getElementById('statusMsg').innerHTML = '✅ Saved! Restarting...'; setTimeout(() => { window.location.href = '/'; }, 3000); })
            .catch(() => { document.getElementById('statusMsg').innerHTML = '❌ Error!'; });
        }
    </script>
</body>
</html>
)rawliteral";
    server.send(200, "text/html", html);
}

void handleScan() {
    int n = WiFi.scanComplete();
    if (n == -2) {
        WiFi.scanNetworks(true);
        server.send(200, "application/json", "[]");
        return;
    } else if (n >= 0) {
        String json = "[";
        for (int i = 0; i < n; ++i) {
            if (i > 0) json += ",";
            json += "{\"ssid\":\"" + jsonEscape(WiFi.SSID(i)) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + ",\"encryptionType\":" + String(WiFi.encryptionType(i)) + "}";
        }
        json += "]";
        WiFi.scanDelete();
        server.send(200, "application/json", json);
    } else {
        server.send(200, "application/json", "[]");
    }
}

void handleSave() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    if (ssid.length() > 0 && ssid.length() <= WIFI_SSID_MAX && pass.length() <= WIFI_PASS_MAX) {
        saveCredentialsToEEPROM(ssid, pass);
        Serial.println("Credentials saved. Restarting...");
        server.send(200, "text/html", "<h2>✅ Saved! Restarting...</h2>");
        delay(1000);
        ESP.restart();
    } else {
        server.send(400, "text/html", "<h2>Invalid SSID or password length.</h2>");
    }
}

void handleNotFound() {
    server.send(404, "text/html", "<h1>404 Not Found</h1>");
}

void readCredentialsFromEEPROM(String &ssid, String &pass) {
    ssid = "";
    pass = "";

    const char magic[4] = {'A', 'D', 'A', '2'};
    bool magicOk = true;
    for (int i = 0; i < 4; i++) {
        if (EEPROM.read(WIFI_MAGIC_ADDR + i) != (uint8_t)magic[i]) magicOk = false;
    }

    uint8_t ssidLen = EEPROM.read(WIFI_SSID_LEN_ADDR);
    uint8_t passLen = EEPROM.read(WIFI_PASS_LEN_ADDR);
    uint16_t storedCrc = (uint16_t)EEPROM.read(WIFI_CRC_ADDR) |
                         ((uint16_t)EEPROM.read(WIFI_CRC_ADDR + 1) << 8);
    if (magicOk) {
        if (ssidLen > 0 && ssidLen <= WIFI_SSID_MAX && passLen <= WIFI_PASS_MAX &&
            storedCrc == credentialCrc(ssidLen, passLen)) {
            for (uint8_t i = 0; i < ssidLen; i++) ssid += (char)EEPROM.read(WIFI_SSID_ADDR + i);
            for (uint8_t i = 0; i < passLen; i++) pass += (char)EEPROM.read(WIFI_PASS_ADDR + i);
        } else {
            Serial.println("Stored WiFi credentials failed validation.");
        }
        return;
    }

    // One-time safe migration from the old null-terminated variable layout.
    if (readLegacyCredentials(ssid, pass)) {
        saveCredentialsToEEPROM(ssid, pass);
        Serial.println("Legacy WiFi credentials migrated to validated storage.");
    }
}

void saveCredentialsToEEPROM(const String &ssid, const String &pass) {
    uint8_t ssidLen = (uint8_t)min((unsigned int)ssid.length(), (unsigned int)WIFI_SSID_MAX);
    uint8_t passLen = (uint8_t)min((unsigned int)pass.length(), (unsigned int)WIFI_PASS_MAX);
    const char magic[4] = {'A', 'D', 'A', '2'};
    for (int i = 0; i < 4; i++) EEPROM.write(WIFI_MAGIC_ADDR + i, magic[i]);
    EEPROM.write(WIFI_SSID_LEN_ADDR, ssidLen);
    EEPROM.write(WIFI_PASS_LEN_ADDR, passLen);
    for (int i = 0; i < WIFI_SSID_MAX; i++)
        EEPROM.write(WIFI_SSID_ADDR + i, i < ssidLen ? ssid[i] : 0);
    for (int i = 0; i < WIFI_PASS_MAX; i++)
        EEPROM.write(WIFI_PASS_ADDR + i, i < passLen ? pass[i] : 0);
    uint16_t crc = credentialCrc(ssidLen, passLen);
    EEPROM.write(WIFI_CRC_ADDR, crc & 0xFF);
    EEPROM.write(WIFI_CRC_ADDR + 1, crc >> 8);
    EEPROM.commit();
}

uint16_t credentialCrc(uint8_t ssidLen, uint8_t passLen) {
    uint16_t crc = 0xFFFF;
    crc ^= ssidLen;
    crc = (crc >> 1) | (crc << 15);
    crc ^= passLen;
    for (uint8_t i = 0; i < ssidLen; i++) {
        crc ^= EEPROM.read(WIFI_SSID_ADDR + i);
        crc = (crc >> 1) | (crc << 15);
    }
    for (uint8_t i = 0; i < passLen; i++) {
        crc ^= EEPROM.read(WIFI_PASS_ADDR + i);
        crc = (crc >> 1) | (crc << 15);
    }
    return crc;
}

bool readLegacyCredentials(String &ssid, String &pass) {
    ssid = "";
    pass = "";
    int addr = 0;
    bool ssidTerminated = false;
    for (int i = 0; i <= WIFI_SSID_MAX; i++) {
        uint8_t value = EEPROM.read(addr++);
        if (value == 0) { ssidTerminated = true; break; }
        if (value == 0xFF || i == WIFI_SSID_MAX) return false;
        ssid += (char)value;
    }
    if (!ssidTerminated || ssid.length() == 0) return false;

    bool passTerminated = false;
    for (int i = 0; i <= WIFI_PASS_MAX; i++) {
        uint8_t value = EEPROM.read(addr++);
        if (value == 0) { passTerminated = true; break; }
        if (value == 0xFF || i == WIFI_PASS_MAX) return false;
        pass += (char)value;
    }
    return passTerminated;
}

String jsonEscape(const String &value) {
    String escaped;
    for (unsigned int i = 0; i < value.length(); i++) {
        char c = value.charAt(i);
        if (c == '\\' || c == '"') escaped += '\\';
        if ((uint8_t)c >= 0x20) escaped += c;
    }
    return escaped;
}

