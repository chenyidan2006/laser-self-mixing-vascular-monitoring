// src/config/api.js - API配置文件
export const API_CONFIG = {
  auth: {
    baseUrl: 'http://192.144.185.64:3002',
    register: '/api/user/register',
    login: '/api/user/login',
    sendSms: '/api/user/sendSms',
    loginBySms: '/api/user/loginBySms',
    changePassword: '/api/user/change-password',
    guestLogin: '/api/user/guestLogin',
    wechatLogin: '/api/user/wechatLogin',
    profile: '/api/user/profile',
    devices: '/api/user/devices',
    devicesBind: '/api/user/devices/bind',
    devicesSearch: '/api/user/devices/search'
  },
  data: {
    baseUrl: 'http://192.144.185.64:3002',
    realtime: '/api/data/realtime',
    historyTrend: '/api/data/history-trend',
    rangeRecords: '/api/data/range-records',
    deviceStatus: '/api/data/device/status',
    dailySummary: '/api/data/daily-summary',
    hourlyRange: '/api/data/hourly-range',
    aiHealthAdvice: '/api/ai/health-advice',
    aiRealtimeAdvice: '/api/ai/realtime-advice'
  }
};

export function getAuthUrl(path) {
  return API_CONFIG.auth.baseUrl + (API_CONFIG.auth[path] || '/api/user/' + path);
}

export function getDataUrl(path) {
  return API_CONFIG.data.baseUrl + (API_CONFIG.data[path] || '/api/data/' + path);
}

