# hardware/

## 子目录

| 路径 | 内容 |
|------|------|
| [kicad/rools-core/](kicad/rools-core/) | **M2** KiCad 工程（10HP Core） |
| [panel/rools-core-10hp-spec.md](panel/rools-core-10hp-spec.md) | 10HP 面板开孔/丝印 |
| [panel/rools-exp-2hp-spec.md](panel/rools-exp-2hp-spec.md) | 2HP Exp 面板 |
| [bom/bom-draft.md](bom/bom-draft.md) | 初步 BOM |

## KiCad

打开 `hardware/kicad/rools-core/rools-core.kicad_pro`（KiCad 10.x）。

层次：`Power` / `Seed` / `Audio` / `CV` / `UI` — 不含 Exp（USB/MIDI）。

音频/电源对照 Daisy Patch Rev8，见 [kicad/rools-core/docs/patch-reference.md](kicad/rools-core/docs/patch-reference.md)。
