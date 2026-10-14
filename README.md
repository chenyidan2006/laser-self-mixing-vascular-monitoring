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

服务器算法各文件、MATLAB `_mcr` 目录关系和现役入口见 [`prodict_algorithm_infuwuqi/README.md`](prodict_algorithm_infuwuqi/README.md)；双通道帧协议见 [`prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。ESP32 上传、服务器解析、服务端口和联调顺序统一见 [`docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md`](docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md)。

数据库的创建、已有库迁移、环境变量、验证和备份说明见后端仓库的 [`docs/DATABASE_SETUP_AND_USAGE.md`](https://github.com/chenyidan2006/health-backend/blob/master/docs/DATABASE_SETUP_AND_USAGE.md)。

## 数据链路

```text
双通道 LSMI 采集板 → ESP32-S3 → HTTP 原始帧接收 → MATLAB 编译算法 → 后端数据库 → App 展示
```

ESP32 每次上传双通道、分块交错的原始 SMI 帧。完整帧格式、采样点数和服务器拆帧逻辑见 [`prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。

现役网络端口只有两个项目服务端口：ESP32 向 `3000` 端口上传原始帧；App 和算法向 `3002` 端口访问业务后端。MySQL 默认使用 `3306`，只供后端在服务器内部访问。算法由接收服务启动为本地子进程，本身不监听网络端口。

## 重要说明

这是竞赛和科研原型，不是医疗器械，输出结果不能用于诊断。App 中的 `reference-bp.vue` 是尚未接通的“袖带血压真值录入”页面，当前不能作为已经可用的训练数据入口。服务器算法失败时还存在默认兜底值写库行为；在用于正式数据采集或训练前，请先处理审计文档中的数据可信度问题。
