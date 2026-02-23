// config/config.js
// 统一配置文件 - 根据环境自动切换

// 导入环境配置
import devConfig from './env.dev.js'
import prodConfig from './env.prod.js'

let config = {}

// 判断当前环境
if (import.meta.env.MODE === 'development') {
  // 开发环境 - 使用DCloud服务器
  console.log('当前是开发环境，使用DCloud服务器')
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