# firmware/

libDaisy + DaisySP 固件。

## 构建

需安装 [Daisy Toolchain](https://daisy.audio/software/)。依赖在 `lib/` submodule：

```bash
git submodule update --init --recursive
make -C lib/libDaisy
make -C lib/DaisySP
make
make program-dfu   # Micro USB 刷机
```

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
