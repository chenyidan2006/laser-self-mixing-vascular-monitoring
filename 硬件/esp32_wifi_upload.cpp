#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <string.h>

// ==================== 配网配置 ====================
#define CONFIG_AP_SSID "HealthMonitor-Config"   // 配网热点名称
#define CONFIG_AP_PASSWORD "12345678"          // 配网热点密码

// ==================== 服务器配置 ====================
const char* SERVER_URL = "http://192.168.1.100:3002";  // 后端服务器地址

// ==================== 串口配置（STM32 ↔ ESP32）====================
#define UART_BAUD_RATE 115200  // 与STM32波特率一致
#define UART_RX_PIN GPIO_NUM_5
#define UART_TX_PIN GPIO_NUM_4

// ==================== 缓冲区配置 ====================
#define BUF_SIZE 524288        // 512KB缓冲区
#define SEND_INTERVAL_MS 6000  // 每6秒上传一次

// ==================== 全局变量 ====================
uint8_t* uart_buf;
volatile size_t buf_wr = 0;
bool is_receiving = false;
unsigned long last_send_time = 0;
bool wifi_connected = false;
bool config_mode = false;

// 配网相关
WebServer server(80);
Preferences preferences;

// ==================== WiFi连接函数 ====================
bool connect_wifi(const char* ssid, const char* password) {
  Serial.printf("连接WiFi: %s...\n", ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    Serial.printf("\n✅ WiFi连接成功\n");
    Serial.printf("📶 IP地址: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("📡 信号强度: %d dBm\n", WiFi.RSSI());
    return true;
  } else {
    wifi_connected = false;
    Serial.println("\n❌ WiFi连接失败");
    return false;
  }
}

// ==================== 保存WiFi配置到Flash ====================
void save_wifi_config(const char* ssid, const char* password) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
  Serial.println("📝 WiFi配置已保存");
}

// ==================== 读取保存的WiFi配置 ====================
bool load_wifi_config(String& ssid, String& password) {
  preferences.begin("wifi", false);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();
  
  return (ssid != "");
}

// ==================== 配网页面处理 ====================
void handle_config_page() {
  String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>健康监测设备配网</title>
      <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; 
               background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); 
               min-height: 100vh; display: flex; align-items: center; justify-content: center; padding: 20px; }
        .container { background: white; border-radius: 16px; box-shadow: 0 20px 60px rgba(0,0,0,0.3); 
                     padding: 40px; max-width: 420px; width: 100%; }
        h1 { text-align: center; color: #333; margin-bottom: 8px; font-size: 24px; }
        .subtitle { text-align: center; color: #666; margin-bottom: 30px; font-size: 14px; }
        .logo { text-align: center; font-size: 48px; margin-bottom: 20px; }
        .form-group { margin-bottom: 20px; }
        label { display: block; margin-bottom: 8px; color: #333; font-weight: 500; }
        input { width: 100%; padding: 14px 16px; border: 2px solid #e0e0e0; border-radius: 10px; 
                font-size: 16px; transition: border-color 0.3s; outline: none; }
        input:focus { border-color: #667eea; }
        button { width: 100%; padding: 16px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); 
                 color: white; border: none; border-radius: 10px; font-size: 16px; font-weight: 600; 
                 cursor: pointer; transition: transform 0.2s; }
        button:hover { transform: translateY(-2px); }
        button:active { transform: translateY(0); }
        .status { margin-top: 20px; padding: 12px; border-radius: 8px; text-align: center; display: none; }
        .status.success { background: #d4edda; color: #155724; display: block; }
        .status.error { background: #f8d7da; color: #721c24; display: block; }
      </style>
    </head>
    <body>
      <div class="container">
        <div class="logo">📱</div>
        <h1>设备配网</h1>
        <p class="subtitle">请输入您的WiFi信息</p>
        
        <form id="configForm">
          <div class="form-group">
            <label for="ssid">WiFi名称 (SSID)</label>
            <input type="text" id="ssid" name="ssid" required placeholder="请输入WiFi名称">
          </div>
          <div class="form-group">
            <label for="password">WiFi密码</label>
            <input type="password" id="password" name="password" placeholder="请输入WiFi密码（无密码可不填）">
          </div>
          <button type="submit">确认配置</button>
        </form>
        
        <div id="status" class="status"></div>
      </div>
      
      <script>
        document.getElementById('configForm').addEventListener('submit', function(e) {
          e.preventDefault();
          const ssid = document.getElementById('ssid').value;
          const password = document.getElementById('password').value;
          const status = document.getElementById('status');
          
          status.textContent = '正在连接WiFi...';
          status.className = 'status';
          
          fetch('/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(password)}`
          })
          .then(response => response.text())
          .then(data => {
            if (data.includes('成功')) {
              status.textContent = '配置成功！设备正在重启...';
              status.className = 'status success';
            } else {
              status.textContent = '连接失败，请检查WiFi信息后重试';
              status.className = 'status error';
            }
          })
          .catch(err => {
            status.textContent = '配置失败，请重试';
            status.className = 'status error';
          });
        });
      </script>
    </body>
    </html>
  )";
  server.send(200, "text/html", html);
}

// ==================== 配网提交处理 ====================
void handle_config_submit() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  Serial.printf("\n📤 收到配网请求: SSID=%s, Password=%s\n", ssid.c_str(), password.c_str());
  
  // 尝试连接用户提供的WiFi
  if (connect_wifi(ssid.c_str(), password.c_str())) {
    // 连接成功，保存配置
    save_wifi_config(ssid.c_str(), password.c_str());
    server.send(200, "text/plain", "配置成功！设备将重启并连接WiFi");
    
    // 延迟后重启
    delay(2000);
    ESP.restart();
  } else {
    server.send(200, "text/plain", "连接失败！请检查WiFi名称和密码");
  }
}

// ==================== 启动配网模式 ====================
void start_config_mode() {
  config_mode = true;
  
  // 启动AP模式
  WiFi.mode(WIFI_AP);
  WiFi.softAP(CONFIG_AP_SSID, CONFIG_AP_PASSWORD);
  
  Serial.println("\n===== 🔧 进入配网模式 =====");
  Serial.printf("📶 配网热点: %s\n", CONFIG_AP_SSID);
  Serial.printf("🔑 热点密码: %s\n", CONFIG_AP_PASSWORD);
  Serial.printf("🌐 配网地址: http://%s\n", WiFi.softAPIP().toString().c_str());
  Serial.println("💡 请在手机WiFi设置中连接此热点，然后打开浏览器访问配网地址");
  
  // 配置HTTP服务器路由
  server.on("/", handle_config_page);
  server.on("/config", handle_config_submit);
  server.begin();
  
  Serial.println("🚀 配网服务器已启动");
}

// ==================== 数据上传函数 ====================
bool upload_data(uint8_t* data, size_t length) {
  if (!wifi_connected) {
    Serial.println("❌ WiFi未连接，跳过上传");
    return false;
  }
  
  if (length == 0) {
    Serial.println("ℹ️ 无数据可上传");
    return false;
  }
  
  HTTPClient http;
  String url = String(SERVER_URL) + "/api/upload/raw";
  
  Serial.printf("📤 开始上传数据：%d字节\n", length);
  
  if (!http.begin(url)) {
    Serial.println("❌ HTTP连接失败");
    return false;
  }
  
  http.addHeader("Content-Type", "application/json");
  
  // 构建JSON数据
  String json = "{\"deviceId\":\"ESP32_001\",\"rawVoltage\":[";
  for (size_t i = 0; i < length; i++) {
    if (i > 0) json += ",";
    json += String(data[i]);
  }
  json += "],\"sampleRate\":100,\"userId\":1}";
  
  int httpCode = http.POST(json);
  
  bool success = false;
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.printf("✅ 上传成功！响应: %s\n", response.c_str());
    success = true;
  } else {
    Serial.printf("❌ 上传失败，HTTP码: %d\n", httpCode);
  }
  
  http.end();
  return success;
}

// ==================== 串口接收中断回调 ====================
void serial_receive_callback() {
  if (!is_receiving) return;
  
  size_t available = Serial1.available();
  if (available > 0) {
    size_t read_len = min(available, (size_t)(BUF_SIZE - buf_wr));
    Serial1.readBytes(&uart_buf[buf_wr], read_len);
    buf_wr += read_len;
    
    if (buf_wr >= BUF_SIZE) {
      Serial.printf("⚠️ 警告：缓冲区溢出！已接收%d字节\n", buf_wr);
      is_receiving = false;
    }
  }
}

// ==================== 初始化串口 ====================
void uart_init() {
  uart_buf = (uint8_t*)malloc(BUF_SIZE);
  if (!uart_buf) {
    Serial.printf("❌ 错误：缓冲区分配失败！\n");
    while (1);
  }
  memset(uart_buf, 0, BUF_SIZE);
  Serial.printf("✅ 缓冲区初始化成功：%d字节\n", BUF_SIZE);
  
  // 初始化Serial1
  Serial1.begin(UART_BAUD_RATE);
  Serial1.setPins(UART_TX_PIN, UART_RX_PIN);
  Serial1.onReceive(serial_receive_callback);
  
  Serial.printf("✅ 串口初始化完成：UART1，波特率%d，RX=%d，TX=%d\n", 
                UART_BAUD_RATE, UART_RX_PIN, UART_TX_PIN);
}

// ==================== 主程序 ====================
void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("\n================= ESP32启动 =================\n");
  
  // 初始化串口
  uart_init();
  
  // 尝试读取保存的WiFi配置
  String saved_ssid, saved_password;
  if (load_wifi_config(saved_ssid, saved_password)) {
    Serial.printf("🔄 尝试连接保存的WiFi: %s\n", saved_ssid.c_str());
    
    if (connect_wifi(saved_ssid.c_str(), saved_password.c_str())) {
      Serial.println("\n✅ 系统就绪！");
      return;
    }
    
    Serial.println("⚠️ 保存的WiFi连接失败，进入配网模式");
  } else {
    Serial.println("ℹ️ 未找到保存的WiFi配置，进入配网模式");
  }
  
  // 进入配网模式
  start_config_mode();
}

void loop() {
  if (config_mode) {
    // 配网模式：处理HTTP请求
    server.handleClient();
    delay(10);
  } else {
    // 正常工作模式
    
    // 定时重新连接WiFi（如果断开）
    if (!wifi_connected && millis() - last_send_time > 10000) {
      String saved_ssid, saved_password;
      if (load_wifi_config(saved_ssid, saved_password)) {
        connect_wifi(saved_ssid.c_str(), saved_password.c_str());
      }
    }
    
    // 数据采集阶段
    is_receiving = true;
    
    // 每6秒上传一次
    if (millis() - last_send_time >= SEND_INTERVAL_MS) {
      last_send_time = millis();
      
      is_receiving = false;
      
      if (buf_wr > 0) {
        Serial.printf("\n----- 📊 数据采集完成 -----\n");
        Serial.printf("📥 已接收：%d字节（%.2f KB）\n", buf_wr, (float)buf_wr / 1024);
        
        // 上传数据
        upload_data(uart_buf, buf_wr);
        
        // 重置缓冲区
        buf_wr = 0;
        memset(uart_buf, 0, BUF_SIZE);
      }
      
      is_receiving = true;
    }
    
    delay(10);
  }
}