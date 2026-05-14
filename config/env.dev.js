// config/env.dev.js
export default {
  // 改成你自己的服务器地址
  baseURL: 'http://192.144.185.64:3001/api',   // 认证服务端口
  
  debug: true,
  logLevel: 'debug',
  
  platform: {
    h5: { baseURL: 'http://192.144.185.64:3000/api' },
    mpWeixin: { baseURL: 'http://192.144.185.64:3000/api' }, // 微信小程序开发时可不校验域名
    app: { baseURL: 'http://192.144.185.64:3000/api' }
  },
  
  // 开发环境也不需要使用云函数了
  cloud: { enable: false }
}