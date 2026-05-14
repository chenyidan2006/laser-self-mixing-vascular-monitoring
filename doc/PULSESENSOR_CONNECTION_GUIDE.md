# PulseSensor 硬件连接与测试指南

---

## 一、项目概述

本指南介绍如何将 **PulseSensor 心率传感器** 连接到四层主控板（STM32F4）和 ESP32 模块，实现脉搏波数据采集和上传。

### 系统架构

```
┌──────────────────┐      ┌──────────────────┐      ┌──────────────────┐
│   PulseSensor    │      │   四层主控板     │      │      ESP32       │
│   心率传感器     │─────►│   (STM32F4)     │─────►│   WiFi上传       │
│                  │      │                  │      │                  │
│  VCC/GND/SIGNAL  │      │  ADC采集/UART发送│      │  HTTP POST上传   │
└──────────────────┘      └──────────────────┘      └──────────────────┘
                              │                              │
                              └──────────────────────────────┘
                                           │
                                           ▼
                              ┌──────────────────┐
                              │   后端服务器     │
                              │  (localhost:3002)│
                              └──────────────────┘
```

---

## 二、硬件清单

| 组件 | 数量 | 说明 |
|------|------|------|
| 四层主控板（STM32F4） | 1块 | 核心控制板 |
| ESP32开发板 | 1块 | WiFi通信模块 |
| PulseSensor 心率传感器 | 1个 | 脉搏波采集 |
| SMA转杜邦线 | 2根 | 信号连接线 |
| 杜邦线 | 若干 | 电源线 |
| 3.3V电源 | 1个 | 系统供电 |

---

## 三、连接方案

### 3.1 PulseSensor ↔ 四层主控板

**必需连接（三根线）**：

| PulseSensor引脚 | 四层主控板引脚 | 功能 | 线缆 |
|-----------------|---------------|------|------|
| `VCC` | `3.3V` | 电源输入 | 新增杜邦线 |
| `GND` | `GND` | 地线 | SMA线1 |
| `SIGNAL` | `PA0 (ADC1_IN0)` | 模拟信号输入 | SMA线2 |

**连接示意图**：

```
PulseSensor模块
     │
     ├── VCC ───────► 主控板 3.3V 引脚（新增杜邦线）
     ├── GND ───────► 主控板 GND（SMA线1）
     └── SIGNAL ────► 主控板 PA0（SMA线2）
```

### 3.2 ESP32 ↔ 四层主控板

**串口连接（UART）**：

| ESP32引脚 | STM32引脚 | 功能 |
|-----------|-----------|------|
| `GPIO5 (RX)` | `PA9 (USART1_TX)` | 数据接收 |
| `GPIO4 (TX)` | `PA10 (USART1_RX)` | 数据发送 |
| `3.3V` | `3.3V` | 电源 |
| `GND` | `GND` | 地线 |

### 3.3 完整连接图

```
                    ┌─────────────────────────────────┐
                    │         四层主控板 (STM32F4)    │
                    │                                 │
                    │  ┌──────────────────────────┐   │
                    │  │ PA0 (ADC1_IN0) ◄─────────┼───┼─── SIGNAL (SMA线2)
                    │  │ PA9 (USART1_TX) ◄────────┼───┼─── ESP32 GPIO5
                    │  │ PA10 (USART1_RX) ────────┼───┼─── ESP32 GPIO4
                    │  │ 3.3V ────────────────────┼───┼─── ESP32 3.3V
                    │  │ GND ─────────────────────┼───┼─── ESP32 GND
                    │  │ 3.3V ────────────────────┼───┘
                    │  │ GND ─────────────────────┼───┘
                    │  └──────────────────────────┘   │
                    └─────────────────────────────────┘
                                   │
                                   │ SMA线1 (GND)
                                   │ SMA线2 (SIGNAL)
                                   ▼
                    ┌─────────────────────────────────┐
                    │         PulseSensor             │
                    │                                 │
                    │  VCC ◄── 新增杜邦线 ── 3.3V    │
                    │  GND ◄── SMA线1 ──── GND       │
                    │  SIGNAL ◄── SMA线2 ── PA0      │
                    └─────────────────────────────────┘
```

---

## 四、软件配置

### 4.1 ESP32端配置

**文件**: `esp32_wifi_upload.cpp`

**关键配置项**：

```cpp
// WiFi配置 - 修改为你的WiFi信息
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL = "http://192.168.1.100:3002";  // 后端服务器地址

// 串口配置
#define UART_BAUD_RATE 115200  // 与STM32一致
#define UART_RX_PIN GPIO_NUM_5
#define UART_TX_PIN GPIO_NUM_4
```

**上传数据格式**：

```json
{
  "deviceId": "ESP32_001",
  "rawVoltage": [128, 130, 125, ...],
  "sampleRate": 100,
  "userId": 1
}
```

### 4.2 STM32端配置

**ADC配置**（读取PulseSensor）：

```c
void MX_ADC1_Init(void) {
  ADC_HandleTypeDef hadc1;
  
  hadc1.Instance = ADC1;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;  // 16位精度
  hadc1.Init.ContinuousConvMode = ENABLE;      // 连续转换
  
  // 配置PA0为ADC输入
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel = ADC_CHANNEL_0;  // PA0 = ADC1_IN0
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}
```

**串口配置**（发送数据到ESP32）：

```c
void MX_USART1_UART_Init(void) {
  UART_HandleTypeDef huart1;
  
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;  // 必须与ESP32一致
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
}
```

**数据发送示例**：

```c
#define BUFFER_SIZE 1024
uint16_t adc_buffer[BUFFER_SIZE];

void send_data_to_esp32() {
  // 发送数据长度
  HAL_UART_Transmit(&huart1, (uint8_t*)&BUFFER_SIZE, sizeof(uint32_t), HAL_MAX_DELAY);
  // 发送ADC数据
  HAL_UART_Transmit(&huart1, (uint8_t*)adc_buffer, BUFFER_SIZE * sizeof(uint16_t), HAL_MAX_DELAY);
}
```

---

## 五、测试步骤

### 5.1 硬件检查清单

```
硬件连接检查：
├── ✅ PulseSensor VCC → 主控板 3.3V
├── ✅ PulseSensor GND → 主控板 GND（SMA线1）
├── ✅ PulseSensor SIGNAL → 主控板 PA0（SMA线2）
├── ✅ ESP32 VCC → 3.3V
├── ✅ ESP32 GND → 主控板 GND
├── ✅ ESP32 RX (GPIO5) → 主控板 PA9 (USART1_TX)
├── ✅ ESP32 TX (GPIO4) → 主控板 PA10 (USART1_RX)
└── ✅ 所有模块共地
```

### 5.2 软件测试流程

**步骤1：启动后端服务**

```bash
cd health-backend
node data-server.js
# 预期输出：✅ MySQL数据库连接成功，🚀 健康数据服务已启动
```

**步骤2：上传ESP32代码**

1. 使用Arduino IDE打开 `esp32_wifi_upload.cpp`
2. 配置WiFi名称和密码
3. 上传到ESP32开发板

**步骤3：观察串口输出**

```
================= ESP32启动 =================

缓冲区初始化成功：524288字节
串口初始化完成：UART1，波特率115200，RX=5，TX=4

连接WiFi: MyHomeWiFi...
..........
WiFi连接成功
IP地址: 192.168.1.105
信号强度: -55 dBm

系统就绪！

----- 数据采集完成 -----
已接收：1024字节（1.00 KB）
开始上传数据：1024字节
上传成功！响应: {"errcode":0,"msg":"success"}
```

**步骤4：验证数据上传**

```bash
curl http://localhost:3002/api/data/mock/realtime?deviceId=ESP32_001
```

---

## 六、常见问题排查

### 6.1 PulseSensor无信号

| 现象 | 可能原因 | 解决方案 |
|------|----------|----------|
| SIGNAL引脚始终为0 | 传感器未供电 | 检查VCC连接 |
| 信号波动过大 | 接触不良 | 检查连接线 |
| 信号值恒定不变 | 传感器损坏 | 更换传感器 |

### 6.2 WiFi连接失败

| 现象 | 可能原因 | 解决方案 |
|------|----------|----------|
| 无法连接WiFi | SSID/密码错误 | 检查配置 |
| 连接后很快断开 | 信号弱 | 靠近路由器 |
| IP地址为0.0.0.0 | DHCP失败 | 使用静态IP |

### 6.3 数据上传失败

| 现象 | 可能原因 | 解决方案 |
|------|----------|----------|
| HTTP码404 | 服务器未启动 | 启动data-server |
| HTTP码500 | 服务器错误 | 检查服务器日志 |
| 网络超时 | 服务器不可达 | 检查IP和端口 |

---

## 七、注意事项

### 7.1 电源稳定性

- 使用**干净的3.3V电源**，避免噪声干扰
- 在电源引脚添加**去耦电容**（100nF + 10μF）
- 模拟地和数字地**单点连接**

### 7.2 信号质量

- 使用**屏蔽线**减少干扰（可选）
- 信号线长度**不超过1米**
- 必要时添加**RC低通滤波器**（R=1kΩ, C=100nF）

### 7.3 波特率匹配

- STM32和ESP32的波特率**必须一致**
- 常用波特率：9600, 19200, 115200
- 高速波特率（如1Mbps）需要更短的连接线

---

## 八、代码文件清单

| 文件 | 位置 | 说明 |
|------|------|------|
| `esp32_wifi_upload.cpp` | 项目根目录 | ESP32 WiFi上传代码 |
| `esp32代码.cpp` | 项目根目录 | ESP32 BLE透传代码（1MB缓冲区） |
| `esp32源码.cpp` | 项目根目录 | ESP32 BLE透传代码（512KB缓冲区） |
| `data-server.js` | health-backend/ | 后端数据服务 |

---

## 九、版本历史

| 版本 | 日期 | 更新内容 |
|------|------|----------|
| v1.0 | 2026-05-09 | 初始版本 |

---

**文档位置**：`PULSESENSOR_CONNECTION_GUIDE.md`  
**相关文档**：[WIFI_CONNECTION_GUIDE.md](WIFI_CONNECTION_GUIDE.md) | [DEBUG_GUIDE.md](DEBUG_GUIDE.md)