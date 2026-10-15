import { API_BASE_URL } from '../src/config/api.js';
export default {
  // 改成你自己的服务器地址
  baseURL: API_BASE_URL + '/api',
  
  debug: true,
  logLevel: 'debug',
  
  platform: {
    h5: { baseURL: API_BASE_URL + '/api' },
    mpWeixin: { baseURL: API_BASE_URL + '/api' },
    app: { baseURL: API_BASE_URL + '/api' }
  },
  
  // 开发环境也不需要使用云函数了
  cloud: { enable: false }
}
