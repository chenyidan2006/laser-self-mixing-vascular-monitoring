// 开发环境配置 - 使用DCloud服务器
export default {
  // API 基础地址
  baseURL: 'https://你的DCloud云函数域名/api',
  
  // 其他开发环境配置
  debug: true,
  logLevel: 'debug',
  
  // 不同平台的特殊配置
  platform: {
    h5: {
      // H5特殊配置
    },
    mpWeixin: {
      // 微信小程序特殊配置
    }
  },
  
  // 云函数配置（如果使用uniCloud）
  cloud: {
    enable: true,
    provider: 'aliyun',
    spaceId: 'mp-4c720286-d2a9-47bd-ac02-bb9f04ae1704'//需要换成自己的spaceId
  }
}