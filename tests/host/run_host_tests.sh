#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd "$(dirname "$0")/../.." && pwd)"
firmware_dir="$project_dir/firmware/Ada"
build_dir="$(mktemp -d)"
trap 'rm -rf "$build_dir"' EXIT

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
