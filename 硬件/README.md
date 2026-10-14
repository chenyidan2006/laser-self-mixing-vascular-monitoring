# 硬件总览

本目录保存激光自混合干涉（SMI）血管参数测量原型的可公开硬件资料。系统不是 PPG：传感端利用激光自混合干涉获取桡动脉附近皮肤微振信号，再由 ADS8688、STM32 和 ESP32 完成采集与上传。

## 先看哪份文档

- [`STM32_ADS8688.md`](STM32_ADS8688.md)：STM32F407、ADS8688、40 kHz 定时采样、DMA 分块发送和代码版本疑点。
- [`ESP32/README.md`](ESP32/README.md)：ESP32-S3 接线、按键/激光控制、PSRAM 缓存、Wi-Fi 上传和烧录步骤。
- [`PCB_ARCHIVE.md`](PCB_ARCHIVE.md)：两套 Altium 文件的版本关系、板卡划分和查看方法。
- [`BRINGUP_CHECKLIST.md`](BRINGUP_CHECKLIST.md)：从断电检查到服务器收帧的逐项联调清单。
- [`../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md`](../prodict_algorithm_infuwuqi/FRAME_PROTOCOL.md)：640,000 字节负载及服务器拆帧方式。

## 系统分工

```text
激光器/外腔与被测皮肤微振
          │ 模拟 SMI 信号
          ▼
模拟前端与 ADS8688
          │ SPI1，16 bit
          ▼
STM32F407
  ├─ TIM2 触发：40,000 次/s
  ├─ 每 2,000 点形成 4,000-byte 块
  └─ USART1 DMA：1,152,000 baud
          │ 3.3 V TTL UART
          ▼
ESP32-S3
  ├─ 控制按键、激光 MOS、WS2812
  ├─ PSRAM 缓存 640,000-byte 负载
  ├─ 补 ADS8688 协议头尾
  └─ Wi-Fi HTTP POST 到服务器 :3000
```

## 目录与版本结论

| 目录 | 内容 | 当前判断 |
| --- | --- | --- |
| `ESP32/` | 2026-10 归档的 ESP32-S3 固件 | 网络上传端现役快照，但与归档 STM32 的启动方式存在版本不一致 |
| `CHEIVE/ADS8688_PROJ/` | STM32CubeMX/Keil STM32F407 工程 | `.uvprojx` 与 `.map` 证明归档构建使用 `ads8_test.c`；能证明 40 kHz、SPI/USART 引脚和分块发送，不能证明物理双通道 |
| `AIA/stm32f4/` | 2026-01-15 Altium 原理图与 PCB | 较早版板卡设计归档 |
| `Altium_stm32f41_2026-02-06/` | 2026-02-06 Altium 原理图、PCB、TP5400 电源板 | 时间更晚且资料更完整，但未获得生产文件/版本声明，不能仅凭日期称为最终投板版 |

## 当前代码能确认的参数

| 项目 | 值 | 直接依据 |
| --- | ---: | --- |
| MCU | STM32F407VET6，LQFP100 | `ADS8688_PROJ.ioc` |
| 核心/总线时钟 | 168 MHz；APB1 42 MHz、TIM2 84 MHz；APB2 84 MHz | `.ioc` 和生成代码 |
| ADC 触发频率 | 40,000 Hz | `tim.c`: `84 MHz / 84 / 25` |
| SPI1 | 主机、16 bit、Mode 1、约 5.25 Mbit/s | `spi .c` |
| STM32 ↔ ESP32 UART | 1,152,000 baud、8N1、无流控 | 项目负责人确认的正式值；STM32 `usart.c` 与 ESP32 固件一致 |
| 单次采集 | 8 s、320,000 个 `uint16`、640,000 bytes | `ads8688.h` 与 ESP32 帧常量 |
| 发送分块 | 2,000 点/块、4,000 bytes/块，共 160 块 | `ads8_test.c` |

## 三个必须保留的证据边界

1. 帧头的 `ch=2` 是 ESP32 写入的协议声明；历史 MATLAB 工具也确实按奇偶 2,000 点块分成 A/B 两组，并记录过块间直流偏差。但 Keil 实际编译的 `ads8_test.c` 只明确选择 `MAN_2` 后连续读取，当前源码仍不能证明两个物理 ADC 通道交替采样。
2. ESP32 固件发送 `A5 5A 01` 作为开始命令；归档 STM32 工程却在上电后直接调用 `ads8688_start_sample()`，USART1 接收中断为空。两份代码不应直接宣称是同一个联调版本。
3. `.ioc` 仍写着 TIM2 `Period=999` 和 USART `1,000,000`，而生成的 `tim.c`/`usart.c` 已改为 `Period=24` 和 `1,152,000`。现役参数以实际参与编译的 C 文件为准；重新生成 CubeMX 代码前必须先同步 `.ioc`。

## ADS8688 三份源码的版本关系

| 文件 | 是否进入归档 Keil 构建 | 当前有效行为 | 判断 |
| --- | --- | --- | --- |
| `User/src/ads8_test.c` | 是；`.uvprojx` 收录，`.map` 链接到 `ads8_test.o` | 40 kHz、8 秒、2,000 点分块；先 `MAN_2` 后连续 `CONT` | 归档工程实际构建版本 |
| `User/src/ads8688.c` | 否；`IncludeInBuild=0` | 手动读 CH0，约 10,000 点测试 | 早期测试版 |
| `User/src/ads8688 - 副本.c` | 否；工程文件未收录 | 手动读 CH0，采集开关初始关闭，1,024 点停止 | 未完成/诊断副本，不能直接认作现役版 |

如果口头信息称“实际用的是副本”，需要学长再提供当时真正烧录的 `.uvprojx`、`.hex` 或构建 `.map`。现有文件证据与这句话不一致，所以仓库保留副本供追溯，但不替换当前工程入口。

## 面试时可以怎样介绍硬件

可以表述为：系统把实验室中的 SMI 采集链路做成了可联网原型。STM32 负责确定性采样和 DMA 分块发送，ESP32 负责测量交互、缓存和网络上传，服务器负责保存原始帧并调用算法。主要工程难点是 40 kS/s 连续数据下的内存复用、串口吞吐、帧边界和网络重试，而不是简单“把传感器接到 App”。

不要表述为“已经验证双通道各 20 kHz”或“ESP32 命令一定能启动当前 STM32”，这两点还需要实机逻辑分析仪/波形验证和固件版本对齐。
