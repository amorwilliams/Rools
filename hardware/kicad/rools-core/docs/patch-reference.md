# Patch Rev8 对照（Core 音频/电源）

| 项目 | 说明 |
|------|------|
| 文件 | `ES_Daisy_Patch_Rev8.pdf` — Electro-Smith **Daisy Patch Rev8** 官方原理图 |
| 用途 | **仅参考** Eurorack 电源、±10V 音频调理（TL074 类）、jack 区 |
| 非目标 | 不照搬整板；Rools **不用 PCM3060**（[ADR-016](../../../docs/decisions/ADR-016-onboard-audio-no-pcm3060.md)） |

## Rools Core 与 Patch 差异

| 区块 | Patch Rev8 | Rools Core |
|------|------------|------------|
| MCU | Daisy Seed | 同 |
| 音频 Codec | PCM3060 + SAI2 | **Seed 板载** pin 16–19 + TL074 |
| 电源 | Eurorack → 稳压 | Buck +5V（ADR-011） |
| 显示 | 无 | ST7735 1.77" SPI |
| CV In | Patch 自有 | 4 CV + 4 Knob **分路 ADC**（ADR-017） |
| CV Out | Patch 自有 | **DAC8565 + OPA4171** |
| UI | 旋钮/开关 | Enc×2 + Btn + K1–K4 |
| USB/MIDI | Patch 板载 | **Exp 2HP**（本工程不含） |

## 建议抄参考的页/区块

1. **Eurorack 16-pin 电源输入** + 保护
2. **音频 In/Out** 运放 ±10V 调理（非 PCM3060 部分）
3. **PJ398/Thonkiconn** jack + normalled（IN_R→IN_L，ADR-010）

## 本地放置

将 PDF 复制到 `references/`（不纳入 git）：

```bash
cp ~/Downloads/ES_Daisy_Patch_Rev8.pdf hardware/kicad/rools-core/references/
```

官方下载：<https://github.com/electro-smith/DaisyPatch/tree/main/hardware>
