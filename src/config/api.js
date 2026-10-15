// src/config/api.js - API配置文件
export const API_BASE_URL = (import.meta.env.VITE_API_BASE_URL || 'http://192.144.185.64:3002').replace(/\/+$/, '');
export const API_CONFIG = {
  auth: {
    baseUrl: API_BASE_URL,
    register: '/api/user/register',
    login: '/api/user/login',
    sendSms: '/api/user/sendSms',
    loginBySms: '/api/user/loginBySms',
    changePassword: '/api/user/change-password',
    resetPassword: '/api/user/reset-password',
    avatar: '/api/user/avatar',
    guestLogin: '/api/user/guestLogin',
    wechatLogin: '/api/user/wechatLogin',
    profile: '/api/user/profile',
    devices: '/api/user/devices',
    devicesBind: '/api/user/devices/bind',
    devicesSearch: '/api/user/devices/search'
  },
  data: {
    baseUrl: API_BASE_URL,
    waveform: '/api/data/waveform',
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

export function getTrainingUrl(key) {
  const paths = { pendingSessions: '/api/training/sessions/pending', referenceBase: '/api/training' };
  if (!(key in paths)) throw new Error(`Unknown training endpoint: ${key}`);
  return API_BASE_URL + paths[key];
}

