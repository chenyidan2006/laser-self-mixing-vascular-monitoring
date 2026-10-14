# 服务器算法接口（公开仓库）

> `prodict_algorithm_infuwuqi` 是历史部署路径。公开仓库只保留可审计的帧协议和 Python 接口壳，不再分发核心 MATLAB 源码、模型、EXE、MCR 或训练数据。

## 公开内容

| 路径 | 作用 |
| --- | --- |
| `FRAME_PROTOCOL.md` | 单通道 40 kHz 帧、历史 `ch=2` 字段及 20 kHz 模型关系 |
| `project/frame_pipeline.py` | 帧解析、调用预测桥、向后端提交结果的接口壳 |
| `project/predictor.py` | 调用本机私有预测 EXE 的桥接代码 |
| `project/README.md` | 私有资产注入、环境变量与已知风险 |

## 私有资产

以下内容保存在本地 `D:\project\PPG\算法`，不进入公开 Git：MATLAB 算法源码、DBP/SBP 模型和预处理参数、Compiler EXE/CTF/MCR、重训练代码、训练表、原始帧、参考血压标签，以及重新编译与逐文件解读文档。

公开仓库中的 Python 代码不能单独完成血压预测。服务器须通过 `PREDICTOR_EXE` 指向私有 `callPredictFixed.exe`，并安装匹配的 MATLAB Runtime。

## 调用链

```text
STM32 + ADS8688（单通道 40 kHz）
  -> ESP32-S3（UART 1,152,000 baud）
  -> HTTP POST :3000/api/upload/raw
  -> raw-receiver/server.js
  -> project/frame_pipeline.py
  -> project/predictor.py
  -> 私有 PREDICTOR_EXE
  -> health-backend :3002
```

## 已确认风险

- 帧头 `ch=2` 是历史硬编码，不代表双物理通道。
- 当前解析器仍按旧 2,000 点块规则丢弃交替块，不是正确的 40→20 kHz 均匀降采样。
- 默认失败兜底会生成演示 DBP/SBP/HR，且 SpO2 固定为 98；正式采集必须关闭。
- 私有可读 CSV 入口与现役 EXE 的二进制临时文件协议存在版本冲突，重新编译前必须统一入口。

详见 [帧协议](FRAME_PROTOCOL.md)、[部署端口与数据流](../docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md) 和 [算法状态与已知问题](../docs/KNOWN_ISSUES_AND_ALGORITHM_STATUS.md)。
