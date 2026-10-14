# ESP32、算法、后端部署与端口说明

本文只描述现役链路，不再整理 App 历史版本关系。

## 一张图看懂现役链路

```text
ADS8688 / STM32
  │ UART 1,152,000 baud，发送 640,000 字节 ADC 负载
  ▼
ESP32-S3
  │ HTTP POST http://<server>:3000/api/upload/raw
  │ Content-Type: application/octet-stream
  │ X-Device-Id: ESP32_001
  ▼
raw-receiver（Node.js，端口 3000）
  │ 校验帧 → received/*.bin + *.json → 串行启动 Python
  ▼
frame_pipeline.py（本地子进程，无网络监听端口）
  │ 拆双通道 → 取通道 1 前 100,000 点
  ▼
predictor.py → callPredictFixed.exe（本地进程，无网络监听端口）
  │ 得到 DBP / SBP / HR
  ▼
POST http://127.0.0.1:3002/api/upload/health-data
  ▼
health-backend（Node.js，端口 3002）
  │ MySQL 3306
  ▼
App 通过 http://<server>:3002/api/... 查询并展示
```

## 端口总表

| 端口/速率 | 使用方 | 用途 | 是否应公网开放 |
| --- | --- | --- | --- |
| UART `1,152,000 baud` | STM32 → ESP32 | 8N1、无流控，现役固件两端一致 | 不是网络端口 |
| TCP `3000` | ESP32 → `raw-receiver` | `POST /api/upload/raw` 上传完整 ADS8688 帧 | 当前固件直连时需要；最好限制来源或经反向代理保护 |
| TCP `3002` | App、算法 → `health-backend` | 登录、设备、健康数据、趋势与 AI 接口 | App 直连时需要；生产环境建议改为 HTTPS 443 反代 |
| TCP `3306` | `health-backend` → MySQL | 数据库连接 | 不应向公网开放，只允许本机或内网后端访问 |
| TCP `80/443` | 可选 Nginx | HTTP/HTTPS 反向代理 | 使用域名部署时开放，优先只公开 443 |
| TCP `5173/8080` | 本地前端开发 | `.env.example` 中允许的本地浏览器来源 | 不属于服务器生产端口，不应对公网开放 |

算法不是 Web 服务：`frame_pipeline.py`、`predictor.py` 和 `callPredictFixed.exe` 都由 `raw-receiver` 在服务器本机启动，不需要再开放“算法端口”。

注意：`1,152,000` 是 UART 波特率，`40,000 Hz` 才是当前 TIM2 触发频率。学长提到的 `2,000,000`、`4,000,000` 如果是 UART 候选速率，不能写成 ADC 采样率；仓库当前也没有启用这两个值。详细换算和物理双通道疑点见 [`../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。

## ESP32 如何上传

固件入口为 [`../硬件/ESP32/firmware/esp32_ads8688_http_forwarder.ino`](../硬件/ESP32/firmware/esp32_ads8688_http_forwarder.ino)。公开版烧录前必须在本机填写 Wi-Fi 和服务器地址，不要把密码提交到 Git。

固件的固定上传参数为：

```cpp
SERVER_PORT = 3000
SERVER_PATH = "/api/upload/raw"
X-Device-Id = "ESP32_001"
Content-Type = "application/octet-stream"
```

一次测量过程：

1. 按键打开激光，预热 5 秒。
2. ESP32 向 STM32 发送 `A5 5A 01`，以该命令作为新帧边界。
3. STM32 返回 640,000 字节原始负载。
4. ESP32 补齐 ASCII 帧头和帧尾，在 PSRAM 中形成完整帧。
5. ESP32 建立到服务器 `3000` 端口的 TCP 连接并发送 HTTP POST。
6. 收到 HTTP `2xx` 才释放上传槽；否则最多重试三次。

现役帧头：

```text
#ADS8688 v1 rate=40000 secs=8 col=10000 blk=2000 nblk=160 ch=2 range=0x02 code=u16le\r\n
```

`X-Device-Id` 应与 App 中绑定的设备号完全一致。服务器通过这个值决定结果最终属于哪个设备。

## `raw-receiver` 如何接收

启动位置：

```powershell
cd raw-receiver
npm install
npm start
```

可选环境变量：

| 名称 | 默认值 | 作用 |
| --- | --- | --- |
| `RAW_RECEIVER_PORT` | `3000` | 原始帧 HTTP 服务端口 |
| `RAW_DATA_DIR` | `raw-receiver/received` | `.bin`、元数据和训练清单保存目录 |
| `PYTHON_BIN` | `python` | Python 解释器命令或绝对路径 |
| `ALGORITHM_WORKER` | 相邻 `prodict_algorithm_infuwuqi/project/frame_pipeline.py` | 算法工作器路径 |

接收器按以下顺序处理：

1. 只接受 `application/octet-stream`，Express 上限为 4 MB。
2. 读取首行并验证 `#ADS8688`、`blk`、`nblk`。
3. 按 `blk × nblk × 2` 算出二进制负载字节数，并检查 `#END ...\r\n`。
4. 使用“设备号 + 整帧 SHA-256”进行两分钟去重。
5. 保存 `received/raw_<时间>_<字节数>.bin`。
6. 同时保存同名 `.bin.json`，并向 `training_manifest.ndjson` 追加元数据。
7. 将算法任务放入串行队列；HTTP 响应不等待 MATLAB 推理完成。

接口返回：

| 状态码 | 含义 |
| --- | --- |
| `200` | 帧已保存并进入算法队列，或被识别为重复重传 |
| `400` | 请求体为空 |
| `422` | 帧头、长度或帧尾不合法 |
| `500` | 保存文件失败 |

健康检查：

```powershell
Invoke-RestMethod http://127.0.0.1:3000/api/upload/raw
```

## 算法如何解析

算法入口为 [`../prodict_algorithm_infuwuqi/project/frame_pipeline.py`](../prodict_algorithm_infuwuqi/project/frame_pipeline.py)。流程如下：

1. 定位第一行换行符并解析 ASCII 帧头。
2. 强制要求 `ch=2`。
3. 以小端 `uint16` 读取 `blk × nblk` 个 ADC 码。
4. reshape 为 `160 × 2000`：偶数编号块属于通道 1，奇数编号块属于通道 2。
5. 当前模型只拼接通道 1，并取前 100,000 点，即 20 kHz 下 5 秒信号。
6. `predictor.py` 创建唯一临时文件，依次写入 `float64 fs`、`uint32 point_count`、`float64 samples[]`。
7. `callPredictFixed.exe` 读取文件并输出 `DBP、SBP、HR`。
8. `frame_pipeline.py` 做有限性和生理范围检查，再 POST 到后端 `3002`。

详细字节布局见 [`../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)，MATLAB EXE 与 `_mcr` 目录关系见 [`../prodict_algorithm_infuwuqi/README.md`](../prodict_algorithm_infuwuqi/README.md)。

正式采集必须关闭演示兜底：

```powershell
$env:ENABLE_FAILURE_FALLBACK = "0"
```

否则模型失败时会生成演示血压值并写库，这些值不能作为真实测量或训练标签。

## 后端如何接结果并供 App 查询

算法向以下接口发送 JSON：

```http
POST http://127.0.0.1:3002/api/upload/health-data
Content-Type: application/json

{
  "deviceId": "ESP32_001",
  "measureTime": "2026-10-14T12:00:00Z",
  "systolicBp": 125,
  "diastolicBp": 82,
  "heartRate": 78,
  "spo2": 98
}
```

其中 `spo2=98` 仍是占位值，不是传感器测量结果。后端把记录写入 MySQL；App 的现役配置 [`../src/config/api.js`](../src/config/api.js) 使用 `3002` 端口访问登录、设备绑定、实时数据、历史趋势和 AI 健康建议接口。

数据库创建与使用见独立后端仓库的 `docs/DATABASE_SETUP_AND_USAGE.md`。

## 推荐启动顺序

1. 启动 MySQL，确认只监听本机或内网。
2. 配置并启动 `health-backend`，检查 `GET :3002/api/health`。
3. 设置 `ENABLE_FAILURE_FALLBACK=0`、`HEALTH_BACKEND_URL` 等算法环境变量。
4. 启动 `raw-receiver`，检查 `GET :3000/api/upload/raw`。
5. 在 App 中注册/登录，并绑定与固件一致的 `ESP32_001`。
6. 烧录 ESP32，按键测量，依次观察 ESP32 串口、接收器日志、算法日志和数据库记录。

## 常见故障定位

| 现象 | 先检查 |
| --- | --- |
| ESP32 显示 TCP connection failed | 服务器 IP、3000 防火墙、`raw-receiver` 是否监听 `0.0.0.0` |
| HTTP 422 | 帧是否完整、`Content-Length`、帧头 `blk/nblk`、`#END` 尾 |
| 文件已保存但没有预测 | `PYTHON_BIN`、`ALGORITHM_WORKER`、Python 依赖、EXE 路径 |
| MATLAB 预测超时 | `PREDICTOR_TIMEOUT_SECONDS`、MATLAB Runtime、输入波形质量 |
| 算法成功但 App 无数据 | 3002 后端、MySQL、`X-Device-Id` 与 App 绑定设备是否一致 |
| 出现看似正常但固定的数值 | 查看日志是否出现 `FALLBACK`；正式采集应关闭兜底 |

