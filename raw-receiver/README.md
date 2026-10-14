# `raw-receiver` 原始 SMI 帧接收服务

这是服务器链路的第一站。它默认监听 `3000` 端口，接收 ESP32 上传的 ADS8688 完整二进制帧，校验、去重、落盘，然后串行调用相邻的 `prodict_algorithm_infuwuqi/project/frame_pipeline.py`。注意：接收和保存流程可用，但当前算法工作器仍错误按遗留 `ch=2` 拆块；现役硬件实际是单通道连续 40 kHz，详见帧协议文档。

它不负责登录、App 查询或数据库 API；这些功能由独立的 `health-backend` 在 `3002` 端口提供。

## 启动

```powershell
cd raw-receiver
npm install
npm start
```

启动后验证：

```powershell
Invoke-RestMethod http://127.0.0.1:3000/api/upload/raw
```

正常返回：

```json
{"errcode":0,"msg":"raw receiver is running","port":3000}
```

## ESP32 请求格式

```http
POST /api/upload/raw HTTP/1.1
Content-Type: application/octet-stream
Content-Length: <完整帧字节数>
X-Device-Id: ESP32_001

<以 #ADS8688 开头、以 #END 行结束的完整二进制帧>
```

固件代码见 [`../硬件/ESP32/firmware/esp32_ads8688_http_forwarder.ino`](../硬件/ESP32/firmware/esp32_ads8688_http_forwarder.ino)。`X-Device-Id` 必须和 App 账户绑定的设备号一致。

## 接收与处理步骤

1. Express 以 `application/octet-stream` 读取请求，大小上限为 4 MB。
2. 校验 `#ADS8688` 头、`blk/nblk` 参数、负载长度和 `#END ...\r\n` 尾。
3. 计算整帧 SHA-256；同一设备两分钟内重复上传同一帧时直接返回成功，不重复预测。
4. 保存原始 `.bin` 和同名 `.bin.json` 元数据。
5. 向 `training_manifest.ndjson` 追加一行机器可读记录。
6. 将 Python 算法放入串行队列，避免多个 MATLAB 进程竞争资源。
7. 立即向 ESP32 返回 HTTP 200；算法在后台继续运行。

## 保存内容

默认目录为 `raw-receiver/received/`：

| 文件 | 作用 |
| --- | --- |
| `raw_<时间>_<长度>.bin` | 完整、未经改写的 ADS8688 帧 |
| `*.bin.json` | 设备号、接收时间、来源 IP、SHA-256 和帧参数 |
| `training_manifest.ndjson` | 所有原始帧的追加式索引 |
| `reference_labels.csv` | 预留的真值标签清单；当前 App 尚未接通自动录入 |

`received/` 保存真实波形，`node_modules/` 保存本机依赖；二者都不会上传 GitHub。

## 环境变量

| 名称 | 默认值 | 说明 |
| --- | --- | --- |
| `RAW_RECEIVER_PORT` | `3000` | HTTP 监听端口 |
| `RAW_DATA_DIR` | 本目录下 `received` | 原始帧和元数据保存目录 |
| `PYTHON_BIN` | `python` | Python 命令或绝对路径 |
| `ALGORITHM_WORKER` | 相邻 `prodict_algorithm_infuwuqi/project/frame_pipeline.py` | Python 算法入口 |

PowerShell 示例：

```powershell
$env:RAW_DATA_DIR = 'D:\smi-data\received'
$env:PYTHON_BIN = 'C:\Python311\python.exe'
$env:ENABLE_FAILURE_FALLBACK = '0'
npm start
```

`ENABLE_FAILURE_FALLBACK` 由子进程继承。正式采集务必设为 `0`，防止算法失败时把演示值写入数据库。

## HTTP 返回码

| 状态码 | 情况 |
| --- | --- |
| `200` | 保存并排队成功，或是已接收过的重复帧 |
| `400` | 空请求体 |
| `422` | 帧头、负载长度或帧尾不合法 |
| `500` | 原始帧落盘失败 |

## 算法调用关系

```text
server.js
  └─ frame_pipeline.py <bin> --device-id <X-Device-Id>
       └─ predictor.py
            └─ callPredictFixed.exe
       └─ POST http://127.0.0.1:3002/api/upload/health-data
```

接收器本身只开放 `3000`；Python 和 MATLAB EXE 都是本地子进程，不监听额外端口。算法拆帧细节见 [`../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。全链路端口、启动顺序和故障定位见 [`../docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md`](../docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md)。

## 安全与生产部署

- 当前 ESP32 上传接口没有设备密钥，只使用 `X-Device-Id` 标识设备；正式公网部署应增加设备签名或令牌、限流和来源限制。
- `3000` 若直接暴露公网，应在云防火墙限制来源；更推荐由 HTTPS 反向代理转发。
- 不要把 `received/`、日志、Wi-Fi 密码或任何真实健康数据提交到 Git。
- 备份原始帧时同时备份 `.bin.json` 和 `training_manifest.ndjson`，保持训练数据可追溯。
