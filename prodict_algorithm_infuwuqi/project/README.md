# `project` 现役部署目录

## 正确启动关系

此目录通常不单独启动常驻服务。现役常驻服务是相邻的 `raw-receiver/server.js`；收到合法帧后，它会启动：

```powershell
python .\prodict_algorithm_infuwuqi\project\frame_pipeline.py <raw-frame.bin> --device-id ESP32_001
```

离线调试且不写数据库：

```powershell
python .\prodict_algorithm_infuwuqi\project\replay_old_frame_no_upload.py <raw-frame.bin>
```

## 文件逐项说明

| 文件/目录 | 用途 | 现役状态 |
| --- | --- | --- |
| `frame_pipeline.py` | 当前按旧版块交替格式拆帧，再调用预测器和后端 | 部署入口，但与现役单通道连续 40 kHz 固件不匹配，必须修复 |
| `predictor.py` | 将采样率、点数和波形写入唯一临时 `.dat`，以文件路径调用 EXE，解析 stdout 或结果文件 | 现役桥接层 |
| `callPredictFixed.exe` | MATLAB Compiler 生成的预测程序 | 现役算法二进制 |
| `processOneSignal.m` | CWT、峰值/特征点检测、5 维特征、DBP/SBP 网络推理的可阅读源代码快照 | 算法参考/重编译源 |
| `Selfcnn_model_DBP.mat` | 舒张压网络 | 现役模型资产 |
| `Selfcnn_model_SBP.mat` | 收缩压网络 | 现役模型资产 |
| `data_preprocess_params_DBP.mat` | 舒张压输入/输出归一化参数 | 现役模型资产 |
| `data_preprocess_params_SBP.mat` | 收缩压输入/输出归一化参数 | 现役模型资产 |
| `callPredictFixed_mcr/` | EXE 展开的 MATLAB Runtime 资源/cache | 运行依赖缓存，不是入口 |
| `replay_old_frame_no_upload.py` | 对历史帧离线运行真实预测，明确跳过数据库上传 | 保留的调试工具 |
| `main.py` | 早期 TCP + WebSocket 方案 | 已废弃；缺少 `config.py` 与 `db_writer.py`，干净环境不能运行 |
| `smi_legacy_*.dat` | 异常中断后遗留的临时输入或结果 | 非源码，可清理 |
| `.deploy_lock` | 部署过程锁文件 | 非源码 |
| `__pycache__/` | Python 字节码缓存 | 非源码，可再生 |

## Python 与 EXE 的文件协议

`predictor.py` 不通过 stdin 传波形。临时文件内容为小端序：

```text
float64 fs
uint32  point_count
float64 signal[point_count]
```

EXE 的已知版本可能把结果文件截断为 24 字节，也可能追加 24 字节。三项均为 `float64`，顺序为 `DBP, SBP, HR`。桥接层同时兼容 stdout 中的 `DBP=...` / `SBP=...` 文本。

## MATLAB 算法做了什么

`processOneSignal.m` 的主要流程是：

1. 对单通道归一化脉搏波做连续小波变换（CWT）。
2. 平滑脊线并检测峰值。
3. 在峰值左右寻找三个特征点。
4. 计算相邻峰与特征点之间的 4 个时间差。
5. 加上由峰间期计算的心率，形成 5 维特征。
6. 分别归一化后输入 DBP、SBP 两个网络。
7. SBP 输出沿用历史脚本的 `× 1.1` 缩放。

注意：修改 `processOneSignal.m` 或 `.mat` 并不会自动修改现有 EXE，必须重新走 MATLAB Compiler 构建并替换 `callPredictFixed.exe`。

## 环境变量

| 环境变量 | 默认值 | 用途 |
| --- | --- | --- |
| `PREDICTOR_EXE` | 同目录 `callPredictFixed.exe` | 覆盖算法可执行文件路径 |
| `PREDICTOR_TIMEOUT_SECONDS` | 60，且最大 60 | 算法超时 |
| `DEVICE_ID` | `ESP32_001` | 默认设备编号 |
| `HEALTH_BACKEND_URL` | `http://127.0.0.1:3002/api/upload/health-data` | 结果写库接口 |
| `ENABLE_FAILURE_FALLBACK` | `1` | 算法失败时是否写演示值；正式采集必须设为 `0` |

## 已知问题

- `spo2` 固定写 98，不是传感器测量值。
- 默认 fallback 是演示数据，不是算法结果，却会进入后端；正式数据采集前必须关闭。
- 学长已确认现役硬件为单物理通道、连续 40 kHz，模型按 20 kHz 训练；帧头 `ch=2` 是遗留字段。
- 当前 `frame_pipeline.py` 仍强制 `ch=2` 并隔一个 2,000 点块取一块。这不是均匀 2 倍降采样，会把连续时间轴切断。
- 当前 `replay_old_frame_no_upload.py` 没有旧讨论中的 `--layout` 参数，也不存在已提交的 `strong-lag-alternation` 自动规则。
- 正确目标是将连续 40 kHz 单通道流按与训练流程一致的方法降到 20 kHz，再取前 100,000 点（5 秒）进入模型。训练期具体滤波/重采样方法仍需核实。
- 在布局解析修复并用已知样本回归前，当前预测结果不能作为真实测量或训练标签。
