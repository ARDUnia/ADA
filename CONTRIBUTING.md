# Contributing

Contributions that improve reliability, documentation, tests or hardware compatibility are welcome.

1. Open an issue describing the problem or proposed change.
2. Create a focused branch from `main`.
3. Keep hardware pin changes centralized in `Config.h`.
4. Avoid blocking delays in the normal main loop.
5. Update documentation when behavior, wiring or file layout changes.
6. Run `tests/host/run_host_tests.sh` before opening a pull request.
7. Include physical test details when a change affects real hardware.

Do not commit Wi-Fi credentials, personal data, API keys, compiled binaries or local IDE configuration.
