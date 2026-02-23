// 生产环境配置 - 使用你自己的服务器
export default {
  // 你的服务器地址（重要！这里要改成你的服务器）
  baseURL: 'https://your-domain.com/api', // 或 http://你的IP:端口/api
  
  // 生产环境配置
  debug: false,
  logLevel: 'error',
  
  // 不同平台的特殊配置
  platform: {
    h5: {
      baseURL: 'https://your-domain.com/api'
    },
    mpWeixin: {
      // 微信小程序必须使用HTTPS且已备案的域名
      baseURL: 'https://your-domain.com/api'
    },
    app: {
      baseURL: 'https://your-domain.com/api'
    }
  },
  
  // 生产环境可能不需要云函数
  cloud: {
    enable: false
  }
}