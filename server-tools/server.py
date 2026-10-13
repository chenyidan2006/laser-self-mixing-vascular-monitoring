from flask import Flask, request
import numpy as np

app = Flask(__name__)

@app.route('/api/upload/raw', methods=['POST'])
def upload_raw():
    # 1. 直接获取 HTTP POST 传来的原始二进制数据
    raw_data = request.data
    
    if not raw_data:
        return {"error": "未接收到数据"}, 400
    
    # 2. 校验数据长度（可选，打印日志方便调试）
    data_length = len(raw_data)
    print(f"📥 收到二进制数据流: {data_length} 字节")

    # 3. 将二进制数据解包为 uint8 数组
    # 由于前端发来的是纯粹的 uint8 内存块，这里直接用 numpy 解析
    data_array = np.frombuffer(raw_data, dtype=np.uint8)

    # 如果你需要将其还原为 16 位的数据 (假设 STM32 发送时是低字节在前)：
    # data_array_16bit = np.frombuffer(raw_data, dtype=np.uint16)

    # 打印前 20 个采样点用于验证
    print(f"📊 数据前20个字节: {data_array[:20]}")
    
    # ==========================================
    # 在这里添加你的业务逻辑：存入数据库、画图或保存为.npy文件
    # np.save('latest_data.npy', data_array)
    # ==========================================

    return {"status": "success", "received_bytes": data_length}, 200

if __name__ == '__main__':
    # 监听所有IP的 3001 端口，与你的 ESP32 配置一致
    app.run(host='0.0.0.0', port=3001, debug=True)