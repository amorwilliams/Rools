# firmware/

libDaisy + DaisySP 固件。

## 构建

需安装 [Daisy Toolchain](https://daisy.audio/software/)（`arm-none-eabi-gcc`、`dfu-util`）。

依赖在 `lib/` submodule。推荐在仓库根目录：

```bash
./build.sh          # 编译
./build.sh flash    # 编译 + 刷机
./build.sh clean    # 清理
```

或在 `firmware/` 下手动：

```bash
git submodule update --init --recursive
make -C ../lib/libDaisy
make -C ../lib/DaisySP
make
```

## 产物

| 文件 | 说明 |
|------|------|
| **`build/rools.bin`** | 最终固件，刷机 / OTA 用这个 |
| `build/rools.elf` | 调试用，含符号 |
| `build/rools.hex` | Intel HEX，Daisy 一般不用 |

## 刷机

`make program-dfu` 通过 `dfu-util` 将 `build/rools.bin` 写入 Daisy Seed。

1. Micro USB 连接 Seed
2. 按住 **BOOT**，再按 **RESET**（进入 DFU 模式）
3. 运行 `./build.sh flash` 或 `make program-dfu`

## IDE

`./build.sh` 编译成功后，若已安装 `compiledb`，会自动生成 `firmware/compile_commands.json`（供 clangd，已 gitignore）。

macOS 不要用 `pip3 install`（PEP 668 会报错），用 pipx：

```bash
brew install pipx
pipx ensurepath
pipx install compiledb
```

重开终端后 `./build.sh` 即可。不装也不影响编译，`.vscode/c_cpp_properties.json` 已覆盖基本 IntelliSense。

## 目录

```
src/
├── main.cpp       # 入口
├── app_shell.h    # App 接口与 AppShell
└── apps/          # 各 App 实现（后续）
```

## 配置

| 宏 | 含义 |
|----|------|
| `HAS_EXP` | 启用 MIDI + USB Host |

## USB 路径（HAS_EXP）

- `/rools/samples/`
- `/rools/presets/`
- `/rools/_updater/rools_firmware.bin`
