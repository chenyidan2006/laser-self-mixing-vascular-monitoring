// utils/request.js

/*
* 页面使用：
* import request from '@/utils/request';
* const data = await request('/user/login', { method: 'POST', data: { username, password } });
*/

import { API_BASE_URL } from '../../src/config/api.js';
const BASE_URL = API_BASE_URL + '/api';

function request(url, options = {}) {
  return new Promise((resolve, reject) => {
    uni.request({
      url: BASE_URL + url,
      method: options.method || 'GET',
      data: options.data || {},
      header: {
        'Authorization': `Bearer ${uni.getStorageSync('token') || ''}`,
        ...options.header
      },
      success: (res) => {
        if (res.statusCode >= 200 && res.statusCode < 300) {
          resolve(res.data);
        } else {
          reject(res);
        }
      },
      fail: reject
    });
  });
}

export default request;
