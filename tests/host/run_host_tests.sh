#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd "$(dirname "$0")/../.." && pwd)"
firmware_dir="$project_dir/firmware/Ada"
audio_dir="$project_dir/assets/audio/MP3"
version_file="$project_dir/VERSION"
build_dir="$(mktemp -d)"
trap 'rm -rf "$build_dir"' EXIT

if [[ ! -s "$version_file" ]]; then
  echo "Missing or empty VERSION file" >&2
  exit 1
fi

project_version="$(tr -d '[:space:]' < "$version_file")"
if [[ ! "$project_version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Invalid semantic version: $project_version" >&2
  exit 1
fi

if ! grep -Fq "#define ADA_VERSION \"$project_version\"" "$firmware_dir/Version.h"; then
  echo "VERSION and ADA_VERSION do not match" >&2
  exit 1
fi

required_tracks=(
  0001 0002 0003 0004 0005 0006 0007
  0008 0009 0010 0011 0012 0013 0014 0020
)

for track in "${required_tracks[@]}"; do
  audio_file="$audio_dir/$track.mp3"
  if [[ ! -s "$audio_file" ]]; then
    echo "Missing or empty audio asset: $audio_file" >&2
    exit 1
  fi
done

audio_count="$(find "$audio_dir" -maxdepth 1 -type f -name '*.mp3' | wc -l)"
if [[ "$audio_count" -ne "${#required_tracks[@]}" ]]; then
  echo "Expected ${#required_tracks[@]} MP3 files, found $audio_count" >&2
  exit 1
fi

includes=("-I$project_dir/tests/host/include" "-I$firmware_dir")
sources=(
  "$project_dir/tests/host/mock_runtime.cpp"
  "$firmware_dir/Touch.cpp"
  "$firmware_dir/Audio.cpp"
  "$firmware_dir/Display.cpp"
  "$firmware_dir/Head.cpp"
  "$firmware_dir/Setup.cpp"
  "$firmware_dir/internet.cpp"
  "$project_dir/tests/host/test_main.cpp"
)

g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -fno-omit-frame-pointer "${includes[@]}" -x c++ \
  "$firmware_dir/Ada.ino" "${sources[@]}" -o "$build_dir/ada_host_tests"
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 "$build_dir/ada_host_tests"
echo "Project version: $project_version"
echo "Audio assets: ${#required_tracks[@]}/${#required_tracks[@]}"
