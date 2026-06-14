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

# clangd / IDE：从 make 导出 compile_commands.json（需 pipx install compiledb）
sanitize_ide_db() {
  python3 - "$ROOT/firmware/compile_commands.json" <<'PY'
import json, sys
from pathlib import Path

p = Path(sys.argv[1])
if not p.is_file():
    sys.exit(0)

REMOVE = {
    "-fno-move-loop-invariants", "-mthumb", "-MMD", "-MP",
    "-fasm", "-finline", "-finline-functions-called-once",
    "-fshort-enums", "-fno-unwind-tables", "-ggdb",
}
PREFIX = ("-mcpu=", "-mfpu=", "-mfloat-abi=", "-MF", "-Wa,", "-alms=")

def keep(arg: str) -> bool:
    if arg in REMOVE:
        return False
    return not any(arg.startswith(p) for p in PREFIX)

data = json.loads(p.read_text())
for ent in data:
    if "arguments" in ent:
        ent["arguments"] = [a for a in ent["arguments"] if keep(a)]
p.write_text(json.dumps(data, indent=1) + "\n")
PY
}

update_ide_db() {
  if command -v compiledb >/dev/null; then
    (cd "$ROOT/firmware" && compiledb -n make)
    sanitize_ide_db
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
