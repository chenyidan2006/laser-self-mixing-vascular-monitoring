# PPG 血管参数测量 App

这是当前 App GitHub 仓库。2026-10-13 已同步最终服务器算法快照，并完成 App、后端、服务器接收、算法推理、训练数据和 MATLAB 编译方式的全链路审计。

## 当前版本

- App 源码：`src/`
- 后端独立仓库：`health-backend/`（GitHub：`chenyidan2006/health-backend`）
- 服务器算法版本化快照：`prodict_algorithm_infuwuqi/`
- 原始波形接收服务快照：`raw-receiver/`
- 非生产串口/Flask 调试脚本：`server-tools/`
- ESP32-S3 固件：`硬件/ESP32/firmware/`
- 本机实际服务器部署目录：`D:\project\PPG\算法\服务器部署\`

详细状态、版本关系和已知问题见 [`docs/PROJECT_AUDIT_2026-10-13.md`](docs/PROJECT_AUDIT_2026-10-13.md)。

服务器算法各文件、MATLAB `_mcr` 目录关系和现役入口见 [`prodict_algorithm_infuwuqi/README.md`](prodict_algorithm_infuwuqi/README.md)；双通道帧协议见 [`prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)；App 从比赛视频版到当前版的提交关系见 [`docs/APP_VERSION_HISTORY.md`](docs/APP_VERSION_HISTORY.md)。

数据库的创建、已有库迁移、环境变量、验证和备份说明见后端仓库的 [`health-backend/docs/DATABASE_SETUP_AND_USAGE.md`](health-backend/docs/DATABASE_SETUP_AND_USAGE.md)。

## 重要说明

App 中的 `reference-bp.vue` 是尚未接通的“袖带血压真值录入”页面，当前不能作为已经可用的训练数据入口。服务器算法失败时还存在默认兜底值写库行为；在用于正式数据采集或训练前，请先处理审计文档中的数据可信度问题。
