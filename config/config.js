// D:/code/app/appprogram/config/config.js
// 统一配置文件 - 根据环境自动切换

// 导入环境配置
import devConfig from './env.dev.js'
import prodConfig from './env.prod.js'

let config = {}

// 判断当前环境
if (import.meta.env.MODE === 'development') {
  // 开发环境与发布环境读取同一个API地址，只有调试日志配置不同。
  console.log('开发模式：使用 src/config/api.js 中的服务器地址')
  config = devConfig
} else {
  // 生产环境 - 使用自己的服务器
  console.log('当前是生产环境，使用自己的服务器')
  config = prodConfig
}

// 添加一个辅助函数，方便获取配置
export function getConfig(platform = '') {
  if (platform && config.platform && config.platform[platform]) {
    return { ...config, ...config.platform[platform] }
  }
  return config
}

export default config
