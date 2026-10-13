# raw-receiver

这是最终服务器包中原始 PPG 波形接收服务的版本化快照。现役部署目录为 `D:\project\PPG\算法\服务器部署\raw-receiver`。

它监听 `3000` 端口，校验并保存 ESP32 上传的 ADS8688 二进制帧，然后串行调用相邻 `prodict_algorithm_infuwuqi/project/frame_pipeline.py`。启动方法：

```powershell
npm install
npm start
```

`received/` 保存真实波形、元数据和训练清单，`node_modules/` 保存本机依赖；二者都不会上传 GitHub。
