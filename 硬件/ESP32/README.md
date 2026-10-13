# ESP32-S3 固件归档

## 当前归档

- `firmware/esp32_ads8688_http_forwarder.ino`
- 来源：用户提供的 `esp32_step2(6).ino`
- 固件标识：`BP-CTRL-2026-10-09-RECOVER`
- 归档日期：2026-10-13

该固件控制测量按键、激光 MOS、WS2812 状态灯，通过 UART 启动 STM32 采集并接收 640,000 字节 ADS8688 负载，再补齐双通道帧头/帧尾并上传服务器。

为适合公开 GitHub，归档版本已将原文件中的 Wi-Fi 名称和密码替换为 `REPLACE_WITH_...` 占位符；服务器地址和数据协议保持不变。烧录前只在本机填写网络配置，不要提交真实密码。

帧结构、双通道含义与服务器拆帧方式见 [`../../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](../../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)。
