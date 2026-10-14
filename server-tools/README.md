# 服务器调试工具

本目录保存两个早期联调脚本。它们不是现役生产入口，也不能替代 [`../raw-receiver/server.js`](../raw-receiver/server.js)。保留它们的价值是做最小网络/串口实验，而不是部署整套系统。

## 文件结论

| 文件 | 用途 | 与现役协议是否兼容 | 什么时候使用 |
| --- | --- | --- | --- |
| `server.py` | 在 PC/服务器上临时监听 HTTP `3001`，打印收到的原始字节 | 能接收任意二进制，但不校验 `#ADS8688`、不保存、不运行算法 | 只验证 ESP32 是否能向某台电脑发 HTTP |
| `receiver.py` | PC 从串口读取 `AA 55 AA 55 + uint32 长度 + 数据`，再转发到 HTTP | **不兼容现役 ESP32 固件**；旧协议、115200 baud，且没有补现役 ADS8688 帧头/尾 | 仅用于复现历史串口方案，不用于当前设备 |

生产链路应使用：

```text
STM32 --UART 1,152,000--> ESP32 --HTTP :3000--> raw-receiver
```

## 为什么 `receiver.py` 不能直接用于现役链路

现役 ESP32 固件通过 GPIO17/GPIO18 的 UART1 以 `1,152,000 baud` 接收 STM32 数据，并直接通过 Wi-Fi 上传。它不会把完整原始帧输出到 USB 调试串口。

而 `receiver.py` 假定的是另一套历史协议：

```text
AA 55 AA 55 | uint32 little-endian payload_length | payload
```

脚本还把电脑串口设成 `115200 baud`。现役帧使用的是：

```text
#ADS8688 ...\r\n | 640000-byte uint16le payload | #END ...\r\n
```

所以仅仅把 `115200` 改成 `1152000` 仍不够；除非另写一版 ESP32 USB 串口转储固件，否则电脑串口根本收不到完整现役帧。

## 使用 `server.py` 做最小 HTTP 测试

### 1. 建议创建隔离环境

在本目录逐条执行：

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install flask numpy
```

如果 PowerShell 阻止当前终端激活脚本，可只对当前进程临时放开：

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\.venv\Scripts\Activate.ps1
```

### 2. 启动临时服务

```powershell
python .\server.py
```

它监听：

```text
http://0.0.0.0:3001/api/upload/raw
```

这是调试端口，不是现役 `raw-receiver` 的 `3000`。

### 3. 从同一台电脑发送测试字节

另开一个 PowerShell：

```powershell
$body = [byte[]](1, 2, 3, 4, 5, 6)
Invoke-WebRequest `
  -Method Post `
  -Uri http://127.0.0.1:3001/api/upload/raw `
  -ContentType 'application/octet-stream' `
  -Body $body
```

服务端应打印 6 字节及其前几个数值。这个结果只能证明 HTTP 路由通了，不能证明 ADS8688 帧正确。

### 4. 让 ESP32 临时向它发送

如果确实要用硬件测试，需要在 ESP32 固件本机副本中临时设置：

```cpp
SERVER_HOST = "运行 server.py 的电脑局域网 IP";
SERVER_PORT = 3001;
SERVER_PATH = "/api/upload/raw";
```

电脑防火墙需允许 Python 接收入站连接。测试完应改回生产接收器的 `3000`，不要把临时配置提交到 Git。

`server.py` 对任何非空字节都返回 200，因此仍然不能确认：

- `#ADS8688` 帧头是否正确；
- 是否恰好有 640,000 字节负载；
- `#END` 帧尾是否完整；
- 双通道声明是否符合物理采样；
- 算法和数据库是否成功。

完整验证必须使用 [`../raw-receiver/README.md`](../raw-receiver/README.md) 中的现役接收器。

## 复现历史 `receiver.py`

只有手头固件确实输出 `AA 55 AA 55 + 长度 + 数据` 时才执行：

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install pyserial requests
python .\receiver.py
```

脚本会寻找描述中包含 JTAG/USB Serial 的串口，找不到时要求输入 `COM` 号。它随后把数据转发到文件内写死的服务器地址。运行前必须先打开 `receiver.py`，核对：

```python
SERVER_URL
serial.Serial(..., 115200, ...)
FRAME_MAGIC = b'\xAA\x55\xAA\x55'
```

不要把它误当成当前 `#ADS8688` 链路。

## 端口与速率对照

| 数字 | 本目录中的含义 | 现役系统中的含义 |
| ---: | --- | --- |
| `115200` | `receiver.py` 的历史 PC 串口速率 | 不使用 |
| `1,152,000` | 本目录脚本未使用 | 项目负责人确认的 STM32 与 ESP32 正式 UART 速率 |
| `3001` | `server.py` 的临时 Flask 端口 | 不使用 |
| `3000` | `receiver.py` 转发目标 | 现役 Node `raw-receiver` |
| `3002` | 本目录脚本不访问 | 现役业务后端与 App API |

## 清理调试环境

退出虚拟环境：

```powershell
deactivate
```

`.venv/`、Python 缓存和测试收到的数据都不应提交 Git。若这些脚本以后不再承担历史复现作用，可以整体删除 `server-tools/`；当前 README 已明确它不是部署入口。
