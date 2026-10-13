# 服务器算法目录说明

> 目录名 `prodict_algorithm_infuwuqi` 是早期部署遗留拼写。为避免服务器脚本路径失效暂不改名；它表示“服务器端预测算法包”。

## 先看结论

- **现役入口不是 `_mcr` 目录，也不是 `main.py`。** 原始帧接收服务 `raw-receiver/server.js` 保存文件后，调用 `project/frame_pipeline.py`。
- `project/` 是现役部署工作目录；里面的 `frame_pipeline.py`、`predictor.py`、`callPredictFixed.exe` 和模型文件共同完成预测。
- 根目录的 `callPredictFixed_mcr/` 与 `project/callPredictFixed_mcr/` 经逐文件 SHA-256 对比完全相同。它们是同一个 MATLAB 编译程序的运行时展开缓存，不是两套算法。
- 当前帧协议声明为 **双通道、按块交替**。服务器只取通道 1 的前 100,000 点进入旧模型。

完整帧说明见 [FRAME_PROTOCOL.md](FRAME_PROTOCOL.md)，现役部署文件逐项说明见 [project/README.md](project/README.md)。

## 现役数据链路

```text
STM32 / ADS8688
  └─ 640,000 字节 ADC 负载
      └─ ESP32-S3 补 #ADS8688 头和 #END 尾
          └─ HTTP POST :3000/api/upload/raw
              └─ raw-receiver/server.js 校验并保存 .bin
                  └─ project/frame_pipeline.py 拆双通道、取通道 1
                      └─ project/predictor.py 写临时输入文件
                          └─ project/callPredictFixed.exe
                              └─ Health-backend :3002 写数据库
                                  └─ App 查询并展示
```

## 两个 `callPredictFixed_mcr` 到底有什么区别

| 路径 | 性质 | 是否入口 | 是否应手工修改 |
| --- | --- | --- | --- |
| `callPredictFixed_mcr/` | MATLAB Compiler 解包缓存的历史副本 | 否 | 否 |
| `project/callPredictFixed_mcr/` | 与 EXE 放在同一部署目录的解包缓存 | 否 | 否 |
| `project/callPredictFixed.exe` | MATLAB 编译后的可执行入口 | 是，由 `predictor.py` 调用 | 否，应从 MATLAB 重新编译 |
| `project/processOneSignal.m` | 可阅读的核心算法源代码快照 | 参考/重编译源 | 可以，但修改后必须重新编译 EXE 才会影响部署 |

两个 `_mcr` 目录均为 62 个文件、229,200 字节，文件相对路径和 SHA-256 全部一致。根目录副本可视为重复归档；保留它只是为了不在本次文档提交里再次改变历史部署文件。将来要精简时，可在确认服务器未引用后删除根目录副本。

`_mcr/callPredictF/callPredictFixed.m` 看起来像 `.m` 文件，但当前内容是 MATLAB 编译打包资源，不是便于维护的普通文本源文件。算法修改应以可阅读源代码和 MATLAB 工程为准，然后重新生成 EXE。

## 目录状态

| 项目 | 状态 | 说明 |
| --- | --- | --- |
| `project/` | 现役 | 服务器预测工作目录 |
| `project/frame_pipeline.py` | 现役入口 | 解析完整 ADS8688 帧、选择通道、预测、上传结果 |
| `project/predictor.py` | 现役桥接 | 按旧 EXE 的文件协议传参并解析结果 |
| `project/callPredictFixed.exe` | 现役算法二进制 | MATLAB Compiler 产物 |
| `project/main.py` | 废弃/不可独立运行 | 旧 WebSocket/TCP 方案，依赖已不存在的 `config.py`、`db_writer.py` |
| `project/replay_old_frame_no_upload.py` | 调试工具 | 离线重放一帧，不写数据库 |
| `project/ppg_legacy_*.dat` | 临时残留 | `predictor.py` 的临时输入/结果文件；无进程运行时可删除 |
| `project/.deploy_lock` | 部署辅助文件 | 空锁文件，不是算法 |
| `project/__pycache__/` | Python 缓存 | 可再生，不是源代码 |

## 当前必须知道的数据风险

`frame_pipeline.py` 默认允许算法失败后生成可重复的演示血压值，并继续写入数据库；`spo2` 也固定写为 98。这些值只能用于演示链路，不能当成真实测量或训练标签。正式采集前应设置：

```powershell
$env:ENABLE_FAILURE_FALLBACK = "0"
```

同时应在后端数据表中增加 `prediction_source` 或 `is_fallback` 字段，明确区分模型结果和演示值。

## 维护规则

1. 改 Python 解析或上传逻辑：修改 `project/frame_pipeline.py` / `project/predictor.py`。
2. 改特征提取或神经网络：修改 MATLAB 源码与模型后重新编译 `callPredictFixed.exe`。
3. 不要直接编辑 `_mcr` 内文件；它们可能在下一次运行时被重新生成。
4. 不要把 `ppg_legacy_*.dat`、`.deploy_lock`、`__pycache__` 当成算法版本。
5. 固件帧契约改变时，必须同步修改 [FRAME_PROTOCOL.md](FRAME_PROTOCOL.md)、`raw-receiver/server.js` 和 `project/frame_pipeline.py`。
