# ADR-008: USB OTA 固件更新

## 状态

已接受

## 背景

用户希望现场刷固件，无需 PC 线缆；参考 arbhar `_updater` 文件夹。

## 决策

U 盘路径 `/rools/_updater/rools_firmware.bin`：

1. 上电扫描
2. 发现 bin → 屏显 Updating → 写 Flash
3. **成功**：删除 bin；`update.log` 追加 OK 行
4. **失败**：保留 bin；log 追加 FAIL 原因
5. 重启

Micro USB（Seed 板载）作开发/救砖 fallback。

## 理由

- 与样本/预设共用 U 盘，目录隔离
- 用户流程：拷 bin → 插 U 盘 → 上电

## 后果

- Bootloader 或 early init 需 USB Host + FAT 支持
- bin 格式与 CRC 规范需在 M4 定稿
