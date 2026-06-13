# firmware/

libDaisy + DaisySP 固件。当前为 M0 占位，不可编译。

## 构建（M1 起）

需安装 [Daisy Toolchain](https://daisy.audio/software/)，并设置 `LIBDAISY_PATH` / `DAISYSP_PATH`。

```bash
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
