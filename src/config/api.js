// src/config/api.js - API配置文件
export const API_CONFIG = {
  auth: {
    baseUrl: 'http://192.144.185.64:3001',
    register: '/api/user/register',
    login: '/api/user/login',
    sendSms: '/api/user/sendSms',
    loginBySms: '/api/user/loginBySms',
    guestLogin: '/api/user/guestLogin',
    wechatLogin: '/api/user/wechatLogin',
    profile: '/api/user/profile',
    avatar: '/api/user/avatar',
    devices: '/api/user/devices',
    devicesBind: '/api/user/devices/bind',
    devicesSearch: '/api/user/devices/search'
  },
  data: {
    baseUrl: 'http://192.144.185.64:3002',
    realtime: '/api/data/realtime',
    mockRealtime: '/api/data/mock/realtime',
    historyTrend: '/api/data/historyTrend',
    mockHistoryTrend: '/api/data/mock/historyTrend',
    rangeRecords: '/api/data/rangeRecords',
    mockRangeRecords: '/api/data/mock/rangeRecords',
    deviceStatus: '/api/device/status',
    dailySummary: '/api/data/dailySummary',
    hourlyRange: '/api/data/hourlyRange',
    aiHealthAdvice: '/api/ai/health-advice',
    aiRealtimeAdvice: '/api/ai/realtime-advice'
  }
};

export function getAuthUrl(path) {
  return API_CONFIG.auth.baseUrl + API_CONFIG.auth[path];
}

export function getDataUrl(path) {
  return API_CONFIG.data.baseUrl + API_CONFIG.data[path];
}