-- 创建缺失的表
-- 在服务器的 MySQL 中执行这些 SQL 语句

USE health_monitor;

-- 创建 processed_health_data 表
CREATE TABLE IF NOT EXISTS processed_health_data (
  id INT AUTO_INCREMENT PRIMARY KEY,
  device_id VARCHAR(64) NOT NULL,
  user_id INT NOT NULL,
  timestamp DATETIME NOT NULL,
  heart_rate INT,
  systolic_bp INT,
  diastolic_bp INT,
  spo2 DECIMAL(5,2),
  temperature DECIMAL(4,2),
  raw_voltage JSON,
  processed_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  INDEX idx_device_timestamp (device_id, timestamp),
  INDEX idx_user_timestamp (user_id, timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 创建 daily_summary 表
CREATE TABLE IF NOT EXISTS daily_summary (
  id INT AUTO_INCREMENT PRIMARY KEY,
  device_id VARCHAR(64) NOT NULL,
  user_id INT NOT NULL,
  date DATE NOT NULL,
  avg_heart_rate DECIMAL(5,2),
  avg_systolic_bp DECIMAL(5,2),
  avg_diastolic_bp DECIMAL(5,2),
  avg_spo2 DECIMAL(5,2),
  avg_temperature DECIMAL(4,2),
  min_heart_rate INT,
  max_heart_rate INT,
  min_bp INT,
  max_bp INT,
  data_points INT DEFAULT 0,
  created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  UNIQUE KEY uk_device_date (device_id, date),
  INDEX idx_user_date (user_id, date)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 插入一些测试数据到 processed_health_data
INSERT IGNORE INTO processed_health_data (device_id, user_id, timestamp, heart_rate, systolic_bp, diastolic_bp, spo2, temperature) VALUES
('ESP32_001', 1, '2026-05-09 08:00:00', 72, 120, 80, 98.5, 36.8),
('ESP32_001', 1, '2026-05-09 09:00:00', 75, 118, 78, 97.8, 36.9),
('ESP32_001', 1, '2026-05-09 10:00:00', 70, 122, 82, 98.2, 37.0);