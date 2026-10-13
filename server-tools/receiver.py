import serial
import serial.tools.list_ports
import requests
import struct

SERVER_URL = 'http://192.144.185.64:3000/api/upload/raw'
FRAME_MAGIC = b'\xAA\x55\xAA\x55'

def find_esp_port():
    for p in serial.tools.list_ports.comports():
        desc = (p.description or '').lower()
        if 'jtag' in desc or 'usb serial' in desc or 'usb-serial' in desc:
            return p.device
    return None

port = find_esp_port()
if port is None:
    print("可用串口:")
    for p in serial.tools.list_ports.comports():
        print(f"  {p.device} - {p.description}")
    port = input("手动输入 ESP32 串口 (如 COM5): ").strip()

print(f"打开 {port}")
ser = serial.Serial(port, 115200, timeout=1)

state = 'IDLE'
magic_pos = 0
len_buf = bytearray()
data_buf = bytearray()
expected_len = 0

print("等待数据...")
while True:
    chunk = ser.read(4096)
    for b in chunk:
        if state == 'IDLE':
            if b == FRAME_MAGIC[magic_pos]:
                magic_pos += 1
                if magic_pos == 4:
                    state = 'READ_LEN'
                    magic_pos = 0
                    len_buf = bytearray()
            else:
                magic_pos = 0
        elif state == 'READ_LEN':
            len_buf.append(b)
            if len(len_buf) == 4:
                expected_len = struct.unpack('<I', len_buf)[0]
                print(f"检测到帧, 长度 = {expected_len}")
                data_buf = bytearray()
                state = 'READ_DATA'
        elif state == 'READ_DATA':
            data_buf.append(b)
            if len(data_buf) >= expected_len:
                print(f"接收完成, POST 到服务器...")
                try:
                    r = requests.post(SERVER_URL, data=bytes(data_buf),
                                      headers={'Content-Type': 'application/octet-stream'},
                                      timeout=30)
                    print(f"服务器返回: {r.status_code} {r.text[:200]}")
                except Exception as e:
                    print(f"上传失败: {e}")
                state = 'IDLE'
                magic_pos = 0