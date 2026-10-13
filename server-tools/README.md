# 服务器调试工具

这两个文件来自最终 `Desktop.zip` 根目录，均不是现役生产入口：

- `receiver.py`：PC 通过串口接收 ESP32 帧，再转发到 HTTP 接收服务。脚本中的服务器 IP 是历史部署地址，使用前要按实际服务器修改。
- `server.py`：最小 Flask 二进制接收测试，只打印数据并监听 `3001`，不保存波形、不调用算法、不写数据库。

按需安装依赖：

```powershell
pip install pyserial requests flask numpy
```

生产服务器应启动 `raw-receiver/server.js`，不要用这里的 `server.py` 替代。
