# ESP32-S3 固件归档

## 当前归档

- `firmware/esp32_ads8688_http_forwarder.ino`
- 来源：用户提供的 `esp32_step2(6).ino`
- 固件标识：`BP-CTRL-2026-10-09-RECOVER`
- 归档日期：2026-10-13

该固件控制测量按键、激光 MOS、WS2812 状态灯，通过 UART 启动 STM32 采集并接收 640,000 字节 ADS8688 负载，再补齐声明 `ch=2` 的帧头/帧尾并上传服务器。`ch=2` 是传输协议声明；当前归档 STM32 源码尚不能证明两个物理 ADC 通道确实交替采样，详见帧协议文档。

为适合公开 GitHub，归档版本已将原文件中的 Wi-Fi 名称和密码替换为 `REPLACE_WITH_...` 占位符；服务器地址和数据协议保持不变。烧录前只在本机填写网络配置，不要提交真实密码。

帧结构、双通道含义与服务器拆帧方式见 [`../../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](../../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。

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
