# Rools Core — 10HP 面板规格

## 外形

| 项目 | 值 |
|------|-----|
| 宽度 | 10 HP = 50.3 mm（Doepfer 实际宽度 ~50.0 mm） |
| 高度 | 128.5 mm |
| 厚度 | 2 mm 铝 |
| 安装孔 | 上下各 1× M3（8–10 HP 标准） |

## 开孔区（自上而下）

### 1. 显示屏窗口

| 项目 | 值 |
|------|-----|
| 模块 | 1.77" IPS ST7735 |
| Outline | 34.0 × 43.8 mm（横屏：宽 43.8，高 34） |
| 建议窗口 | 比 outline 大 0.5 mm/边 |
| 位置 | 水平居中，距顶 ~8 mm |

### 2. Encoder + 按钮行（~34 mm 以下）

| 件 | 数量 | 孔径/说明 |
|----|------|----------|
| Rotary Encoder | 2 | 6 mm 轴；Enc A 列 2，Enc B 列 3 附近 |
| Tact Switch | 1 | 6 mm；两 Encoder 中间 |
| 行高 | ~14 mm | |

### 3. Potentiometer 行

| 件 | 数量 | 说明 |
|----|------|------|
| 9 mm 或 16 mm 轴 Knob | 4 | 与 CV1–4 列对齐；轴间距 ~12.5 mm |
| 行高 | ~16 mm | |

### 4. 插孔区（3 行 × 4 列）

| 行 | 标签（丝印） | 孔 |
|----|-------------|-----|
| 1 | CV IN: CV1 CV2 CV3 CV4 | 4× 3.5 mm |
| 2 | CV OUT: A B C D | 4× 3.5 mm |
| 3 | AUDIO: IN_L OUT_L IN_R OUT_R | 4× 3.5 mm |

- 孔型：Thonkiconn/PJ398 标准 7.8 mm 安装孔
- 列 pitch：~12.5 mm（50.3 mm 四等分减边距）
- 行 pitch：~9–10 mm

## 丝印

- 顶：ROOLS
- 分区：CV IN / CV OUT / AUDIO
- Enc A / Enc B 可选

## PCB 对位

- 面板与 PCB 用 2× M3 尼龙柱或直接 PCB 装孔对齐
- 深度预算：屏 + 编码器 + 竖插 jack ≈ 35 mm

## 待 KiCad 验证

- [ ] 4× Knob 9 mm 轴间距与帽干涉
- [ ] 1.77" 屏 bezel 与 Encoder 行垂直间距
- [ ] Mounting hole 与 jack 无干涉
