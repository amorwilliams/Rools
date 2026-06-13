# 02 — 面板布局

## Rools Core（10HP × 128.5 mm）v1.5

```
┌────────────────────────────────────────┐
│      1.77" IPS Landscape 160×128       │  ~34mm
├────────────────────────────────────────┤
│  Enc A+Push    [Btn]    Enc B+Push     │  ~14mm
├────────────────────────────────────────┤
│  (K1)      (K2)      (K3)      (K4)    │  ~16mm
├────────────────────────────────────────┤
│  CV IN:  CV1    CV2    CV3    CV4       │
│  CV OUT:  A      B      C      D        │
│  AUDIO: IN_L  OUT_L  IN_R  OUT_R        │
└────────────────────────────────────────┘
              ← 50.3mm (10HP) →
```

## 垂直对齐

| 列 | 1 | 2 | 3 | 4 |
|----|---|---|---|---|
| Knob | K1 | K2 | K3 | K4 |
| CV In | CV1 | CV2 | CV3 | CV4 |
| CV Out | A | B | C | D |
| Audio | IN_L | OUT_L | IN_R | OUT_R |

## Rools Exp（+2HP）

```
┌──────────┐
│ MIDI IN  │
│ MIDI OUT │
│ USB HOST │
└──────────┘
```

## 尺寸约束

| 项目 | 数值 |
|------|------|
| 10HP 面板宽 | ~50.3 mm |
| 1.77" 屏 outline 宽 | 43.8 mm（横屏） |
| 屏侧余量 | ~3 mm/侧（放不下 O_C 式屏侧小键） |
| 4 列插孔 pitch | ~12.5 mm |
| 接口区 | 3 行 × 4 列 = 12 孔 |
| 控件+屏+孔总高约 | ~97 mm（128.5 mm 内可行） |

## 与 O_C 对照

| O_C 4.1 | Rools |
|---------|-------|
| 小 OLED + 屏侧 A/B/X/Y | 1.77" 横屏 + 中间单键 |
| 双 Encoder | 双 Encoder + Push |
| 2× Knob | 4× Knob |
| CV IN 8 / CV OUT 8 | CV IN 4 / CV OUT 4 |
| AUDIO 底行 | 同序 IN_L OUT_L IN_R OUT_R |

## 丝印建议

- 分区标题：`CV IN` / `CV OUT` / `AUDIO`
- Audio 底行顺序与 O_C 一致，便于用户 muscle memory
- CV Out 标签 A–D（或 OUT A–D）
