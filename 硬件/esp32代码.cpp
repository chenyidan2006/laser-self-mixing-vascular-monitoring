#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>
#include <string.h>

// 串口配置
#define UART_BAUD_RATE 1000000  // 1Mbps（与外部设备波特率必须一致）
#define UART_RX_PIN GPIO_NUM_5   // 接收引脚
#define BUF_SIZE 1048576        // 1MB 缓冲区（启用PSRAM后可分配）

// BLE配置（与小程序一致，无需修改）
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// 全局变量
uint8_t* uart_buf;
volatile size_t buf_wr = 0;
bool is_receiving = false;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
uint16_t mtu_size = 23;

// BLE连接回调
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) { 
    deviceConnected = true; 
    Serial.println("BLE设备已连接");
  }
  void onDisconnect(BLEServer* pServer) { 
    deviceConnected = false; 
    Serial.println("BLE设备已断开，重新广播...");
    BLEDevice::startAdvertising();
  }
};

// BLE MTU协商回调
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onMtuChanged(BLEDevice* pDevice, uint16_t oldMtu, uint16_t newMtu) {
    mtu_size = newMtu;
    Serial.printf("BLE MTU更新为：%d字节\n", mtu_size);
  }
};

// 串口数据接收回调函数（中断方式，批量读取提升效率）
void serial_receive_callback() {
  if (!is_receiving) return;

  // 批量读取所有可用数据，减少中断循环次数
  size_t available = Serial1.available();
  if (available > 0) {
    size_t read_len = min(available, (size_t)(BUF_SIZE - buf_wr));
    Serial1.readBytes(&uart_buf[buf_wr], read_len);
    buf_wr += read_len;

    // 缓冲区溢出提示（便于排查问题）
    if (buf_wr >= BUF_SIZE) {
      Serial.printf("警告：接收缓冲区溢出！已接收%d字节（缓冲区满）\n", buf_wr);
    }
  }
}

// 初始化串口（使用 Arduino Serial1 + onReceive 中断）
void uart_init() {
  // 强制从 PSRAM 分配 1MB 缓冲区
  uart_buf = (uint8_t*)ps_malloc(BUF_SIZE);
  if (!uart_buf) {
    Serial.printf("错误：PSRAM 缓冲区分配失败！当前设置为%d字节，请检查PSRAM是否启用\n", BUF_SIZE);
    while (1);  // 卡死，等待复位
  }
  memset(uart_buf, 0, BUF_SIZE);
  Serial.printf("PSRAM 缓冲区初始化成功：%d字节（%.2f MB）\n", BUF_SIZE, (float)BUF_SIZE / 1024 / 1024);

  // 初始化 Serial1（对应 UART1），指定 TX=4、RX=5
  Serial1.begin(UART_BAUD_RATE);
  Serial1.setPins(4, UART_RX_PIN);  // TX=4（ESP32发送），RX=5（ESP32接收）

  // 注册串口接收中断回调
  Serial1.onReceive(serial_receive_callback);

  Serial.printf("串口初始化完成：UART1，波特率%d bps，RX=%d，TX=%d\n", 
                UART_BAUD_RATE, UART_RX_PIN, 4);
}

// 初始化BLE（确保执行到这里）
void ble_init() {
  BLEDevice::init("ESP32S3_Data");  // 设备名称（小程序搜索关键词）
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 创建 BLE 服务和特征值（与小程序 UUID 一致）
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY  // 支持读+通知
  );
  pCharacteristic->addDescriptor(new BLE2902());  // 必须添加，否则部分设备无法接收通知
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // 启动服务和广播
  pService->start();
  BLEAdvertising* pAdv = BLEDevice::getAdvertising();
  pAdv->addServiceUUID(SERVICE_UUID);  // 广播中携带服务 UUID，便于小程序识别
  pAdv->setMinPreferred(0x06);          // 优化 BLE 连接性能
  BLEDevice::startAdvertising();

  Serial.println("BLE初始化完成，等待设备连接...");
}

void setup() {
  // 初始化调试串口（波特率115200，用于打印日志）
  Serial.begin(115200);
  while (!Serial) { /* 等待串口监视器连接（部分开发板需要） */ }
  Serial.println("\n================= 系统启动 =================\n");

  // 先初始化串口，再初始化BLE（顺序不影响，但日志更清晰）
  uart_init();
  ble_init();

  Serial.println("\n系统就绪，等待第一轮数据采集...");
}

void loop() {
  // 1. 数据接收阶段（6秒）
  is_receiving = true;
  buf_wr = 0;  // 重置缓冲区指针
  Serial.println("\n===== 开始6秒数据接收 =====");
  unsigned long receive_start = millis();
  
  // 等待6秒（期间中断自动接收数据）
  while (millis() - receive_start < 6000) {
    delay(10);  // 轻微延时，不影响中断接收
  }
  
  is_receiving = false;
  Serial.printf("接收完成：共接收%d字节（%.2f KB）\n", buf_wr, (float)buf_wr / 1024);

  // 2. 数据发送阶段（10秒）
  if (deviceConnected && buf_wr > 0) {
    Serial.println("===== 开始10秒数据转发 =====");
    const size_t max_payload = mtu_size - 3;  // BLE 通知的最大有效载荷（MTU-3字节头）
    uint8_t send_buf[max_payload];
    size_t sent_len = 0;
    unsigned long send_start = millis();

    // 循环发送所有数据（超时10秒）
    while (sent_len < buf_wr && (millis() - send_start) < 10000) {
      size_t send_len = min(max_payload, buf_wr - sent_len);
      memcpy(send_buf, &uart_buf[sent_len], send_len);
      
      pCharacteristic->setValue(send_buf, send_len);
      pCharacteristic->notify();
      
      sent_len += send_len;
      delayMicroseconds(1500);  // 调整延时，避免 BLE 拥塞
    }

    // 打印发送统计
    float send_rate = (float)sent_len / 1024.0 / ((millis() - send_start) / 1000.0);
    Serial.printf("转发完成：发送%d字节（%.2f KB），速率%.2f KB/s\n",
                 sent_len, (float)sent_len / 1024.0, send_rate);
    
    if (sent_len < buf_wr) {
      Serial.println("警告：时间窗口结束，数据未发送完毕！");
    }
  } else if (!deviceConnected) {
    Serial.println("未连接BLE设备，跳过转发");
  } else {
    Serial.println("无数据可转发");
  }

  // 3. 补足16秒周期（确保总周期稳定）
  unsigned long cycle_time = millis() - receive_start;
  if (cycle_time < 16000) {
    delay(16000 - cycle_time);
  }
  Serial.println("===== 周期结束 =====");
}