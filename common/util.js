// D:/code/app/appprogram/common/util.js
export const saveLog = (message) => {
  console.log('[LOG]', message);
  
  // 存储到本地
  try {
    const logs = uni.getStorageSync('appLogs') || [];
    logs.unshift({
      time: new Date().toLocaleString('zh-CN'),
      message: message,
      page: 'index'
    });
    
    // 只保留最近的100条日志
    if (logs.length > 100) {
      logs.length = 100;
    }
    
    uni.setStorageSync('appLogs', logs);
  } catch (error) {
    console.error('保存日志失败:', error);
  }
};

// 其他工具函数可以在这里添加
export const formatTime = (date) => {
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, '0');
  const day = String(date.getDate()).padStart(2, '0');
  const hour = String(date.getHours()).padStart(2, '0');
  const minute = String(date.getMinutes()).padStart(2, '0');
  const second = String(date.getSeconds()).padStart(2, '0');
  
  return `${year}-${month}-${day} ${hour}:${minute}:${second}`;
};