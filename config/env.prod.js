//使用你自己的服务器
//D:/code/app/appprogram/config/env.prod.js
export default {
  // 你的服务器地址（重要！这里要改成你的服务器）
  baseURL: 'http://192.144.185.64:3001/api', // 认证服务端口
  
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