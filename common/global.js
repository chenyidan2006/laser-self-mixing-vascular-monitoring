// 全局状态管理
export default {
  // 蓝牙相关全局数据
  ble: {
    deviceId: '',
    characteristicId: '',
    bleBuffer: new Uint8Array(2 * 1024 * 1024), // 2MB批量缓冲区
    bleBufLen: 0,
    isBatchReceiving: false, // 标记是否批量接收
    expectedBatchLen: 750 * 1024 // 预期批量数据长度（750KB）
  },
  
  // 应用状态
  app: {
    version: '1.0.0',
    platform: '',
    theme: 'light'
  },
  
  // 用户数据
  user: {
    token: '',
    userId: '',
    settings: {}
  },
  
  // 日志数据
  logs: [],
  
  // 初始化方法
  init() {
    console.log('全局状态初始化')
    
    // 获取平台信息
    const systemInfo = uni.getSystemInfoSync()
    this.app.platform = systemInfo.platform
    
    // 从本地存储加载数据
    this.loadFromStorage()
    
    return this
  },
  
  // 从本地存储加载数据
  loadFromStorage() {
    try {
      // 加载日志
      const savedLogs = uni.getStorageSync('logs')
      if (savedLogs) {
        this.logs = savedLogs
      }
      
      // 加载用户设置
      const settings = uni.getStorageSync('user_settings')
      if (settings) {
        this.user.settings = settings
      }
      
      // 加载主题
      const theme = uni.getStorageSync('app_theme')
      if (theme) {
        this.app.theme = theme
      }
    } catch (error) {
      console.error('从本地存储加载数据失败:', error)
    }
  },
  
  // 保存到本地存储
  saveToStorage() {
    try {
      // 保存日志（最多100条）
      if (this.logs.length > 100) {
        this.logs = this.logs.slice(-100)
      }
      uni.setStorageSync('logs', this.logs)
      
      // 保存用户设置
      uni.setStorageSync('user_settings', this.user.settings)
      
      // 保存主题
      uni.setStorageSync('app_theme', this.app.theme)
    } catch (error) {
      console.error('保存到本地存储失败:', error)
    }
  },
  
  // 添加日志
  addLog(action, data = null) {
    const log = {
      timestamp: Date.now(),
      action: action,
      data: data,
      platform: this.app.platform
    }
    
    this.logs.unshift(log)
    
    // 保持最多100条日志
    if (this.logs.length > 100) {
      this.logs.pop()
    }
    
    // 保存到本地存储
    this.saveToStorage()
    
    console.log('日志记录:', log)
    return log
  },
  
  // 获取蓝牙缓冲区
  getBleBuffer() {
    return this.ble.bleBuffer.slice(0, this.ble.bleBufLen)
  },
  
  // 重置蓝牙缓冲区
  resetBleBuffer() {
    this.ble.bleBufLen = 0
    this.ble.isBatchReceiving = false
    console.log('蓝牙缓冲区已重置')
  },
  
  // 设置蓝牙设备
  setBleDevice(deviceId, characteristicId = '') {
    this.ble.deviceId = deviceId
    this.ble.characteristicId = characteristicId
    console.log('蓝牙设备已设置:', deviceId)
  },
  
  // 清空蓝牙设备
  clearBleDevice() {
    this.ble.deviceId = ''
    this.ble.characteristicId = ''
    this.resetBleBuffer()
    console.log('蓝牙设备已清除')
  },
  
  // 切换主题
  toggleTheme() {
    this.app.theme = this.app.theme === 'light' ? 'dark' : 'light'
    this.saveToStorage()
    console.log('主题已切换为:', this.app.theme)
    return this.app.theme
  },
  
  // 获取格式化日志
  getFormattedLogs() {
    return this.logs.map(log => {
      const date = new Date(log.timestamp)
      return {
        date: `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-${String(date.getDate()).padStart(2, '0')} ${String(date.getHours()).padStart(2, '0')}:${String(date.getMinutes()).padStart(2, '0')}:${String(date.getSeconds()).padStart(2, '0')}`,
        action: log.action,
        data: log.data,
        timeStamp: log.timestamp
      }
    })
  },
  
  // 清空日志
  clearLogs() {
    this.logs = []
    uni.removeStorageSync('logs')
    console.log('日志已清空')
  }
}

// 创建单例实例
const globalData = Object.create(global.default).init()

// 导出单例
export { globalData }

// 也导出默认实例以便于兼容性
export default globalData