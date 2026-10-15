<!-- App.vue -->
<script setup>
import { onLaunch, onShow, onHide } from '@dcloudio/uni-app'
import {tr, syncNavigation} from '@/utils/i18n.js'
import {appendAppLog} from '@/utils/app-logs.js'

// 初始化全局数据
const initGlobalData = () => {
  console.log('初始化全局数据')
  
  uni.$app = uni.$app || {}
  uni.$app.globalData = {
    deviceId: '',
    characteristicId: '',
    bleBuffer: new Uint8Array(2 * 1024 * 1024),
    bleBufLen: 0,
    isBatchReceiving: false,
    expectedBatchLen: 750 * 1024
  }
  
  console.log('全局数据初始化完成')
}

// 全局请求拦截器
const setupRequestInterceptor = () => {
  // Keep the original `uni` receiver.  In H5, extracting the method and
  // calling it as a plain function can lose its internal request context.
  const originalRequest = uni.request.bind(uni);

  const normalizeJwt = (value) => {
    if (value && typeof value === 'object') value = value.token;
    if (typeof value !== 'string') return '';
    const token = value.trim()
      .replace(/^Bearer\s+/i, '')
      .replace(/^['"]|['"]$/g, '');
    return /^[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+$/.test(token) ? token : '';
  };
  
  uni.request = (options) => {
    // Only send a real JWT. Older builds may have left "undefined", an
    // object, or an already-prefixed value in storage; any of those becomes
    // "jwt malformed" on protected device/data endpoints.
    const storedToken = uni.getStorageSync('token');
    const token = normalizeJwt(storedToken);
    options.header = options.header || {};
    if (token) {
      options.header.Authorization = `Bearer ${token}`;
    } else {
      delete options.header.Authorization;
      if (storedToken) uni.removeStorageSync('token');
    }
    
    // 保存原始的 success 和 fail 回调
    const expectsPromise = !options.success && !options.fail;
    const originalSuccess = options.success;
    const originalFail = options.fail;
    
    // 重写 success 回调处理 401
    options.success = (res) => {
      if(res.statusCode>=400) appendAppLog(`HTTP ${res.statusCode} ${String(options.url).split('?')[0]}`);
      if (res.statusCode === 401 && /\/api\/(?:data|user\/(?:profile|devices))/.test(options.url)) {
        // Token 过期或未登录，清除本地存储并跳转登录页
        uni.removeStorageSync('token');
        uni.removeStorageSync('user');
        uni.showToast({ title: tr('登录已失效，请重新登录'), icon: 'none' });
        setTimeout(() => {
          uni.reLaunch({ url: '/pages/login/login' });
        }, 1500);
      }
      if (originalSuccess) {
        originalSuccess(res);
      }
    };
    
    // 重写 fail 回调
    options.fail = (err) => {
      appendAppLog(`Request failed: ${String(options.url).split('?')[0]}`);
      console.error('Request failed:', err);
      if (originalFail) {
        originalFail(err);
      }
    };
    
    if(expectsPromise) return new Promise((resolve,reject)=>originalRequest({...options,success:res=>{options.success(res);resolve(res);},fail:err=>{options.fail(err);reject(err);}}));
    return originalRequest(options);
  };
}

// 应用启动时执行
onLaunch((options) => {
  appendAppLog('Application started');
  console.log('========== App Launch ==========', options)
  console.log('当前平台:', uni.getSystemInfoSync().platform)
  
  // 初始化全局数据
  initGlobalData()
  
  // 设置全局请求拦截器
  setupRequestInterceptor()
  syncNavigation()
  for (const method of ['showToast','showLoading','showModal']) {
    uni.addInterceptor(method,{invoke(args){for(const key of ['title','content','confirmText','cancelText'])if(args[key])args[key]=tr(args[key]);if(method==='showModal'){args.confirmText=args.confirmText||tr('Confirm');args.cancelText=args.cancelText||tr('Cancel');}}});
  }
  
  console.log('应用启动完成')
    const token = uni.getStorageSync('token');
    if (!token) {
      // 未登录,跳转到登录页
      setTimeout(() => {
        uni.reLaunch({ url: '/pages/login/login' });
      }, 100);
    }
})

onShow((options) => {
  console.log('App Show', options)
})

onHide(() => {
  console.log('App Hide')
})
</script>

<style lang="scss">
	@import 'uview-plus/theme.scss';
	@import "uview-plus/index.scss";
page {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
  font-size: 28rpx;
  background-color: #f5f7fa;
  height: 100%;
  overflow-y: auto;
  box-sizing: border-box;
  padding-bottom: 0;
}

/* 重置默认样式 */
button {
  margin: 0;
  padding: 0;
  background-color: transparent;
  border-radius: 0;
  line-height: normal;
}

button::after {
  border: none;
}

/* 适配 H5 底部安全区域 */
.fixed-bottom {
  position: fixed;
  bottom: 0;
  left: 0;
  width: 100%;
  padding-bottom: constant(safe-area-inset-bottom);
  padding-bottom: env(safe-area-inset-bottom);
  box-sizing: content-box; /* 确保内边距不影响宽度 */
}

/* 全局工具类 */
.text-center { text-align: center; }
.flex { display: flex; }
.flex-center { display: flex; align-items: center; justify-content: center; }
.flex-between { display: flex; align-items: center; justify-content: space-between; }
.mt-10 { margin-top: 10rpx; }
.mt-20 { margin-top: 20rpx; }
.mb-10 { margin-bottom: 10rpx; }
.mb-20 { margin-bottom: 20rpx; }
</style>
