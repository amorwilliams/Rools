# ADR-003: 10HP Core + 2HP Exp I/O 拓扑

## 状态

已接受

## 背景

参考 O_C 4.1：12HP = ~10HP 功能面 + 2HP MIDI/USB 扩展。

## 决策

**Core 10HP**：
- 4 CV In + 4 CV Out + 4 Audio（O_C 分层：CV 上、AUDIO 底）
- 2× Encoder + 1× Btn + 4× Knob

**Exp +2HP（可选）**：
- MIDI IN/OUT
- USB-A Host

## 理由

- 10HP 放 4 列插孔 pitch ~12.5 mm 可接受
- 数字接口不占核心功能面
- 用户可只购 Core

## 后果

- 固件 `#ifdef HAS_EXP`
- Core PCB 需预留 Exp 连接器
