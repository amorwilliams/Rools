# Rools

10HP+2HP 可选扩展 Eurorack 多功能可视化 DSP 模块。Daisy Seed + 1.77" 横屏 IPS，单 App 运行模式。

## 文档索引

| 文档 | 说明 |
|------|------|
| [docs/00-overview.md](docs/00-overview.md) | 项目概述 |
| [docs/01-product-spec.md](docs/01-product-spec.md) | 产品规格 |
| [docs/02-panel-layout.md](docs/02-panel-layout.md) | 面板布局 |
| [docs/03-io-spec.md](docs/03-io-spec.md) | I/O 定义 |
| [docs/04-hardware-architecture.md](docs/04-hardware-architecture.md) | 硬件架构 |
| [docs/05-software-architecture.md](docs/05-software-architecture.md) | 软件架构 |
| [docs/06-ui-ux.md](docs/06-ui-ux.md) | 交互规范 |
| [docs/07-roadmap.md](docs/07-roadmap.md) | 开发里程碑 |
| [docs/decisions/](docs/decisions/) | ADR 决策记录 |
| [docs/references/modules-comparison.md](docs/references/modules-comparison.md) | 参考模块对比 |

## 快速开始

```bash
git clone --recursive <repo>
./build.sh          # 编译
./build.sh flash    # 刷机（Seed DFU 模式）
```

详见 [firmware/README.md](firmware/README.md)。

## 依赖版本（git submodule，固定 tag）

| 库 | Tag | Commit |
|----|-----|--------|
| [libDaisy](https://github.com/electro-smith/libDaisy) | **v8.1.0** | `9498417` |
| [DaisySP](https://github.com/electro-smith/DaisySP) | **V1.0.0** | `a0494a3` |

克隆务必带 submodule：`git clone --recursive`。升级依赖时在 `lib/` 内 checkout 新 tag，编译验证后 commit 父仓库的 submodule 指针。

## 目录结构

```
Rools/
├── lib/            # libDaisy / DaisySP submodule
├── docs/           # 设计文档
├── hardware/       # 面板规格、BOM（KiCad 后续）
├── firmware/       # libDaisy 固件
└── build.sh        # 编译脚本
```

## 参考

- [Daisy Seed](https://daisy.audio/hardware/Seed/)
- [O_C 4.1 O.R.N.8](https://modulargrid.net/e/tunefish-modular-ornament-crime-o-c-t4-1-o-r-n-8)
