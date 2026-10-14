# 激光自混合干涉血管参数测量系统

一套基于激光自混合干涉（Laser Self-Mixing Interferometry，LSMI）的无袖带血管参数测量原型系统，覆盖激光微振信号采集、ESP32/STM32 传输、服务器算法、数据库和移动端 App。项目曾获第十四届全国大学生光电设计竞赛华北区一等奖，并获批全国大学生创新创业训练计划项目（尚未结题）。

本项目是在已有科研成果基础上的产品化与工程化实践。本人负责 App 前后端、ESP32 到服务器的通信，以及算法重训练、优化和部署联调。本仓库保留可公开的 App、硬件、通信协议和服务器算法快照；后端因部署与密钥管理需要独立维护。

## 理论来源与产品化关系

- Zhao 等人在 *Optics Express* 2024 论文中提出可穿戴自混合干涉传感器：激光照射桡动脉附近皮肤，动脉壁微弱形变改变外腔长度，反馈光进入激光腔形成自混合干涉信号；随后通过 CWT、相位处理/脉搏波重构和 CNN 估计 SBP、DBP。论文 DOI：[10.1364/OE.534566](https://doi.org/10.1364/OE.534566)。
- 后续研究采用 1310 nm DFB 激光器、ADS8688 与 STM32F407 形成小型化采集链路，并使用 RSAF 时频增强和 SA-CNN 直接从关键时频特征预测血压，减少完整相位解包裹和脉搏波重构步骤。
- 本仓库面向“从实验算法到可演示产品”的缺口，完成采集设备联网、二进制帧上传、服务器推理、数据库持久化、App 展示和部署文档。论文及毕业论文原文受版权和隐私约束，不随源码仓库分发。

## 当前版本

- App 源码：`src/`
- 后端独立仓库：[chenyidan2006/health-backend](https://github.com/chenyidan2006/health-backend)
- 服务器算法版本化快照：`prodict_algorithm_infuwuqi/`
- 原始波形接收服务快照：`raw-receiver/`
- 非生产串口/Flask 调试脚本：`server-tools/`
- ESP32-S3 固件：`硬件/ESP32/firmware/`
- 硬件总览、STM32/ADS8688、PCB 和联调文档：[`硬件/README.md`](硬件/README.md)
- MATLAB 原始帧转换、解析和 8 图诊断工具：[`tools/matlab/README.md`](tools/matlab/README.md)

服务器公开接口、私有算法资产注入方式见 [`prodict_algorithm_infuwuqi/README.md`](prodict_algorithm_infuwuqi/README.md)；现役单通道布局、旧版块交替格式和 40→20 kHz 降采样关系见 [`prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。ESP32 上传、服务器解析、服务端口和联调顺序统一见 [`docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md`](docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md)，冲突与剩余风险见 [`docs/KNOWN_ISSUES_AND_ALGORITHM_STATUS.md`](docs/KNOWN_ISSUES_AND_ALGORITHM_STATUS.md)。核心 MATLAB 源码、模型、EXE、MCR、训练代码和真实数据属于保密资产，不随本公开仓库分发。

数据库的创建、已有库迁移、环境变量、验证和备份说明见后端仓库的 [`docs/DATABASE_SETUP_AND_USAGE.md`](https://github.com/chenyidan2006/health-backend/blob/master/docs/DATABASE_SETUP_AND_USAGE.md)。

## 数据链路

```text
SMI 模拟前端与 ADS8688/STM32 采集 → ESP32-S3 → HTTP 原始帧接收 → MATLAB 编译算法 → 后端数据库 → App 展示
```

ESP32 每次上传一帧原始 SMI 数据。学长已确认现役硬件只采一个物理通道：STM32 以 40 kHz 连续采样，模型则按 20 kHz 训练，因此模型前应对连续流做 2 倍降采样。帧头中的 `ch=2` 是 ESP32 硬编码的历史兼容字段，不代表现役物理双通道。旧版曾讨论/实现过按 2,000 点块交替的双通道格式，只用于恢复来源明确的旧文件。完整解释见 [`prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。

现役网络端口只有两个项目服务端口：ESP32 向 `3000` 端口上传原始帧；App 和算法向 `3002` 端口访问业务后端。MySQL 默认使用 `3306`，只供后端在服务器内部访问。算法由接收服务启动为本地子进程，本身不监听网络端口。

## 采样率、串口速率与 8 秒数据量

这几个数字属于不同层，不能都叫“采样率”：

| 参数 | 当前代码值 | 含义与代码依据 |
| --- | ---: | --- |
| ADC 触发频率 | `40,000 Hz` | STM32 TIM2：`84 MHz ÷ (83+1) ÷ (24+1) = 40,000 Hz` |
| STM32 → ESP32 UART | `1,152,000 baud`（正式值） | 项目负责人确认；STM32 `usart.c` 和 ESP32 固件两端一致，8N1、无流控 |
| SPI1 时钟 | 约 `5.25 MHz` | APB2 为 84 MHz，SPI1 预分频为 16 |
| 采集时间 | `8 s` | STM32 `SAMPLE_SECONDS` 与 ESP32 帧头 `secs=8` |
| 总 ADC 码数 | `320,000` | `40,000 × 8` |
| 二进制负载 | `640,000 bytes` | `320,000 × uint16 的 2 bytes` |

### 为什么程序设置为 40 kHz

学长已确认：模型使用 `20 kHz` 数据训练，但采集端为了获得更好的原始数据改用 `40 kHz`。现役系统仍是单物理通道；40 kHz 不是由“双通道各 20 kHz”相加得到的。

仓库代码可以直接证实 STM32 TIM2 产生 `40,000 Hz` 触发；模型训练采样率 `20,000 Hz` 来自学长确认。训练期具体采用了哪一种抗混叠滤波/重采样方法仍需从训练脚本或训练数据生成记录中核实，不能自行假设。

还要避免把两个“乘 2”混为一谈：

- 奈奎斯特的 `×2`：`采样率 > 2 × 信号最高频率`，是避免混叠的理论下限；实际系统通常还要为模拟滤波器过渡带和噪声留余量。
- 模型适配的 `÷2`：现役连续单通道流从 40 kHz 降采样到 20 kHz，这是采样率转换；应均匀降采样并采用与训练流程一致的抗混叠处理。

因此最准确的表述是：“现役 ADS8688 单通道以 40 kHz 连续采集；旧模型按 20 kHz 训练，推理前需要将连续流降采样到 20 kHz。”不要再写成“两通道各 20 kHz”。

现役单通道的数据量与模型输入关系为：

```text
40000 点/秒 × 8 秒 = 320000 个单通道原始点
320000 点 × 2 字节/点 = 640000 字节
连续流按 2 倍降采样后 = 160000 点（20 kHz、8 秒）
模型取前 100000 点 = 20 kHz、5 秒
```

这里的“2 字节/点”和“2 倍降采样”都不代表两个通道。16 位采样在 `.bin` 中按两个二进制字节保存；十六进制只是人工显示方式，转换成带空格的十六进制文本反而更占空间。

当前仓库存在一个已经确认的解析错误：`frame_pipeline.py` 仍信任过时的 `ch=2`，隔一个 2,000 点块取一块。这会把现役连续流变成“保留 50 ms、丢掉 50 ms”的断续数据，并不等于 40→20 kHz 均匀降采样。旧离线回放脚本曾复用同一错误逻辑，现已从公开现役入口中删除。代码正式修复前，不应把预测或 fallback 值当作可靠测量/训练数据。

### 正式串口速率：1,152,000 baud

项目负责人已确认 STM32 → ESP32 的正式 UART 速率是 `1,152,000 baud`。这也与 STM32 `usart.c` 和 ESP32 固件常量完全一致。`2,000,000`、`4,000,000` 只作为历史讨论/吞吐对比值保留，不是本项目现役配置，也不是 ADC 采样率。

UART 使用 8N1，每个数据字节在线路上约占 10 bit。只计算 640,000 字节负载时：

| UART 波特率 | 理论有效字节率 | 640,000 字节最短传输时间 | 4,000 字节块传输时间 |
| ---: | ---: | ---: | ---: |
| `1,152,000` | `115,200 B/s` | `5.56 s` | `34.72 ms` |
| `2,000,000` | `200,000 B/s` | `3.20 s` | `20.00 ms` |
| `4,000,000` | `400,000 B/s` | `1.60 s` | `10.00 ms` |

40 kHz 采样下每 2,000 点产生一个块，间隔为 `2,000 ÷ 40,000 = 50 ms`。当前 1,152,000 baud 发送一块约需 34.72 ms，理论上能在下一个块产生前发完；实际还要考虑 DMA 调度、中断、线缆质量和时钟误差。2 M/4 M 只能增加串口余量，不能改变 TIM2 的 40 kHz ADC 触发频率。

更完整的逐项计算与验证方法见 [`prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。

## 手工部署文档

本项目不提供一键部署脚本。所有部署步骤都写成可以逐条执行、逐条验证的命令：

- [ESP32、算法、后端部署与端口说明](docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md)
- [算法状态、已解决冲突与剩余问题](docs/KNOWN_ISSUES_AND_ALGORITHM_STATUS.md)
- [原始 SMI 帧接收服务](raw-receiver/README.md)
- [服务器调试工具](server-tools/README.md)
- [MySQL 讲解、建表与使用](https://github.com/chenyidan2006/health-backend/blob/master/docs/DATABASE_SETUP_AND_USAGE.md)
- [后端端口与接口关系](https://github.com/chenyidan2006/health-backend/blob/master/docs/PORTS_AND_INTEGRATION.md)

## 重要说明

这是竞赛和科研原型，不是医疗器械，输出结果不能用于诊断。App 中的 `reference-bp.vue` 是尚未接通的“袖带血压真值录入”页面，当前不能作为已经可用的训练数据入口。服务器算法失败时还存在默认兜底值写库行为；在用于正式数据采集或训练前，请先处理审计文档中的数据可信度问题。
