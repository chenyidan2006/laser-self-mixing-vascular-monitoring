# `project` 公开接口壳

此目录保存现役 Python 接口、锁定依赖与合成回归测试。核心算法与模型为私有资产，不随公开仓库分发。

| 文件 | 作用 | 当前状态 |
| --- | --- | --- |
| `frame_pipeline.py` | 连续单通道40→20 kHz解析、调用预测器、提交后端 | 已修复，支持 --parse-only |
| `predictor.py` | 将波形写入临时文件，调用私有 MATLAB EXE，并解析结果 | 现役桥接 |

已删除的旧/私有内容包括：`main.py`、离线旧解析脚本、MATLAB 源码、模型 `.mat`、预测 EXE 和两个重复 MCR 展开目录。这些不再是公开仓库入口。

## 私有 EXE 配置

将保密构建产物放在公开 Git 工作树之外，或放入被 `.gitignore` 排除的本地位置，然后设置：

```powershell
$env:PREDICTOR_EXE = 'D:\project\PPG\算法\服务器部署\prodict_algorithm_infuwuqi\project\callPredictFixed.exe'
$env:PREDICTOR_TIMEOUT_SECONDS = '60'
$env:ENABLE_FAILURE_FALLBACK = '1' # 比赛演示；保留默认兜底
```

接收服务调用：

```powershell
python .\prodict_algorithm_infuwuqi\project\frame_pipeline.py <raw-frame.bin> --device-id ESP32_001
```

## Python 与私有 EXE 的历史文件协议

临时文件由 `predictor.py` 按小端写入：

```text
float64 fs
uint32  point_count
float64 signal[point_count]
```

已知 EXE 可能在同一文件中写回或追加三个 `float64`：`DBP, SBP, HR`，桥接层也兼容 stdout 的 `DBP=...`、`SBP=...` 文本。本协议必须在下一次重新编译时锁定；现有可读 MATLAB 归档入口读取 CSV，不能原样编译成与该二进制协议兼容的 EXE。

## 环境变量

| 变量 | 默认值 | 说明 |
| --- | --- | --- |
| `PREDICTOR_EXE` | 同目录 `callPredictFixed.exe` | 私有 EXE 路径；默认文件在公开仓库中不存在 |
| `PREDICTOR_TIMEOUT_SECONDS` | 60，最大 60 | 单次预测超时 |
| `DEVICE_ID` | `ESP32_001` | 默认设备号 |
| `HEALTH_BACKEND_URL` | `http://127.0.0.1:3002/api/upload/health-data` | 后端写库接口 |
| `ENABLE_FAILURE_FALLBACK` | `1` | 演示兜底；正式数据必须设为 `0` |

## 解析状态

2026-10-15 改为完整连续流 → resample_poly FIR抗混叠 → 20 kHz → 前100,000点。保留首点0xFFFF剔除及既有EXE幅值契约。通过 requirements.txt 安装依赖；使用 --parse-only 检查时不会运行EXE或写库。完整命令、滤波选择、回归与回退见 [解析与回退手册](../../docs/PARSER_REPAIR_AND_ROLLBACK.md)。训练期具体滤波和模型精度未在此次修复中确认，演示fallback不能作为真值。
