<!-- App.vue -->
<script setup>
import { onLaunch, onShow, onHide } from '@dcloudio/uni-app'

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

// 应用启动时执行
onLaunch((options) => {
  console.log('========== App Launch ==========', options)
  console.log('当前平台:', uni.getSystemInfoSync().platform)
  
  // 初始化全局数据
  initGlobalData()
  
  console.log('应用启动完成')
})

onShow((options) => {
  console.log('App Show', options)
})

onHide(() => {
  console.log('App Hide')
})
</script>

<style>
/* 全局样式保持不变 */
page {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
  font-size: 28rpx;
  background-color: #f5f7fa;
  height: 100%;
  box-sizing: border-box;
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