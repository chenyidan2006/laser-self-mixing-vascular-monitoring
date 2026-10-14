# ESP32-S3 固件归档

## 当前归档

- `firmware/esp32_ads8688_http_forwarder.ino`
- 来源：用户提供的 `esp32_step2(6).ino`
- 固件标识：`BP-CTRL-2026-10-09-RECOVER`
- 归档日期：2026-10-13

该固件控制测量按键、激光 MOS、WS2812 状态灯，尝试通过 UART 命令启动 STM32，并接收 640,000 字节 ADS8688 负载，再补齐声明 `ch=2` 的帧头/帧尾并上传服务器。仓库现有 STM32 快照不解析该命令；`ch=2` 也只是传输协议声明，尚不能证明两个物理 ADC 通道确实交替采样。两项版本疑点见下文和帧协议文档。

为适合公开 GitHub，归档版本已将原文件中的 Wi-Fi 名称和密码替换为 `REPLACE_WITH_...` 占位符；服务器地址和数据协议保持不变。烧录前只在本机填写网络配置，不要提交真实密码。

帧结构、双通道含义与服务器拆帧方式见 [`../../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](../../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。

硬件总览、STM32 采集说明和联调清单分别见 [`../README.md`](../README.md)、[`../STM32_ADS8688.md`](../STM32_ADS8688.md) 和 [`../BRINGUP_CHECKLIST.md`](../BRINGUP_CHECKLIST.md)。

## 引脚与接线

| ESP32-S3 引脚 | 方向 | 用途 | 对端 |
| --- | --- | --- | --- |
| GPIO18 | 输入 | UART1 RX，接收 ADC 负载 | STM32 PA9 / USART1 TX |
| GPIO17 | 输出 | UART1 TX，发送开始命令 | STM32 PA10 / USART1 RX |
| GPIO5 | 输入 | 测量按键，按下为低 | 外部约 1 kΩ 上拉到 3.3 V，按键接 GND |
| GPIO7 | 输出 | WS2812 状态灯数据 | WS2812 DIN |
| GPIO47 | 输出 | 激光 MOS 控制，高电平打开 | MOS 驱动输入 |

两块板必须共地。STM32/ESP32 UART 都是 3.3 V TTL，不要连接 ±RS-232 电平。

## 手工烧录

1. 安装 Arduino IDE 与适配的 Arduino-ESP32 2.x 开发板包。
2. 打开 `firmware/esp32_ads8688_http_forwarder.ino`。
3. 选择实际 ESP32-S3 板型，并启用 PSRAM；本固件需要分配一个 704 KiB 帧槽。
4. 只在本机填写 `WIFI_SSID`、`WIFI_PASSWORD`、`SERVER_HOST`。
5. 编译并烧录，串口监视器设置为 `115200 baud`。
6. 启动日志必须显示 PSRAM 分配成功和 Wi-Fi 获得 IP，再连接 STM32。

项目负责人已确认，固件内部与 STM32 通信的正式 UART1 速率是 `1,152,000 baud`；USB 串口监视器的 `115,200 baud` 只用于日志，两者不要混淆。

## 指示灯状态

| 颜色 | 含义 |
| --- | --- |
| 蓝色 | 空闲、激光关闭 |
| 琥珀色 | 已按下测量键，激光预热 |
| 绿色 | 已发送 STM32 启动命令，等待首个数据字节 |
| 紫色 | 已收到 STM32 首个负载字节 |
| 红色 | 两秒内没有收到 STM32 数据或发生控制错误 |

## 与归档 STM32 的版本不一致

本固件把 `A5 5A 01` 当作一次采集的明确帧边界，并在发送命令前先准备 640,000-byte 接收状态。但仓库现有 STM32 快照上电后直接采样，USART1 RX 中断为空，不解析这条命令。

因此当前两份归档代码不能未经修改就视为完整配套固件。正式联调应优先补齐 STM32 命令状态机，或者找到与本 ESP32 固件同期的 STM32 版本。详见 [`../STM32_ADS8688.md`](../STM32_ADS8688.md)。

## 上传方法

烧录前修改固件顶部的三个配置：

```cpp
constexpr char WIFI_SSID[] = "你的 Wi-Fi";
constexpr char WIFI_PASSWORD[] = "你的密码";
constexpr char SERVER_HOST[] = "服务器 IP 或域名";
```

`SERVER_PORT` 保持为 `3000`，`SERVER_PATH` 保持为 `/api/upload/raw`。固件建立 TCP 连接后发送标准 HTTP 请求：

```http
POST /api/upload/raw HTTP/1.1
Host: <服务器>:3000
Content-Type: application/octet-stream
Content-Length: <完整帧字节数>
X-Device-Id: ESP32_001
Connection: close

<ADS8688 完整二进制帧>
```

`X-Device-Id` 必须与 App 用户绑定的设备号一致，否则预测结果虽然可以写库，但该用户无法查询到它。上传成功以服务器返回 HTTP `2xx` 为准；失败时固件按 1 秒、2 秒的间隔最多重试三次。接收器使用“设备号 + 完整帧 SHA-256”做两分钟去重，所以一次测量的网络重传不会重复预测或重复写库。

## 一次测量的时序

1. 按下 GPIO5 按键，GPIO47 打开激光，WS2812 显示琥珀色。
2. 等待 5 秒软启动后，通过 UART1 TX 向 STM32 发送 `A5 5A 01`。
3. STM32 通过 UART1 RX 返回 640,000 字节；串口参数为 1,152,000 baud、8N1、无流控。
4. ESP32 在 PSRAM 中缓存负载，并补上 `#ADS8688 ... ch=2` 帧头和 `#END ...` 帧尾。
5. 上传任务向服务器 `3000` 端口发送 `application/octet-stream` 请求。
6. 原始帧接收器校验、落盘并排队运行算法；算法再将结果发给 `3002` 端口的后端。

完整部署、端口和验证命令见 [`../../docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md`](../../docs/DEPLOYMENT_PORTS_AND_DATA_FLOW.md)。
