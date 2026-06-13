#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

init_submodules() {
  git -C "$ROOT" submodule update --init --recursive
}

build_libs() {
  make -C "$ROOT/lib/libDaisy"
  make -C "$ROOT/lib/DaisySP"
}

build_firmware() {
  make -C "$ROOT/firmware" "$@"
}

# clangd / IDE：从 make 导出 compile_commands.json（需 pip install compiledb）
update_ide_db() {
  if command -v compiledb >/dev/null; then
    (cd "$ROOT/firmware" && compiledb -n make)
  fi
}

case "${1:-build}" in
  build)
    init_submodules
    build_libs
    build_firmware
    update_ide_db
    ;;
  flash|program-dfu)
    init_submodules
    build_libs
    build_firmware
    update_ide_db
    make -C "$ROOT/firmware" program-dfu
    ;;
  clean)
    make -C "$ROOT/firmware" clean
    make -C "$ROOT/lib/DaisySP" clean
    make -C "$ROOT/lib/libDaisy" clean
    ;;
  *)
    init_submodules
    build_libs
    build_firmware "$@"
    update_ide_db
    ;;
esac
