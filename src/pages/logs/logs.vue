<!-- pages/logs/logs.vue -->
<template>
  <view class="logs-page">
    <view class="header">
      <text class="title">{{ tr("应用日志") }}</text>
      <button @click="clearLogs" class="clear-btn">{{ tr("清空日志") }}</button>
    </view>
    
    <scroll-view class="log-list" scroll-y>
      <view v-for="(log, index) in logs" :key="index" class="log-item">
        <text class="log-time">{{ log.time }}</text>
        <text class="log-message">{{ log.message }}</text>
      </view>
      <view v-if="logs.length === 0" class="empty">{{ tr("暂无日志") }}</view>
    </scroll-view>
  </view>
</template>

<script setup>
import { ref, onMounted } from 'vue'

const logs = ref([])

onMounted(() => {
  loadLogs()
})

const loadLogs = () => {
  try {
    logs.value = uni.getStorageSync('appLogs') || []
    console.log('加载日志:', logs.value.length, '长度')
  } catch (e) {
    console.error('加载日志失败:', e)
  }
}

const clearLogs = () => {
  uni.showModal({
    title: '确认清空',
    content: '确定要清空所有日志吗',
    success: (res) => {
      if (res.confirm) {
        uni.removeStorageSync('appLogs')
        logs.value = []
        uni.showToast({ title: '日志已清空' })
      }
    }
  })
}
</script>

<style>
.logs-page {
  padding: 20rpx;
}

.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20rpx;
}

.title {
  font-size: 36rpx;
  font-weight: bold;
}

.clear-btn {
  background-color: #ff4d4f;
  color: white;
  font-size: 24rpx;
  padding: 8rpx 16rpx;
  border-radius: 8rpx;
}

.log-list {
  height: calc(100vh - 120rpx);
}

.log-item {
  padding: 20rpx;
  border-bottom: 1rpx solid #eee;
}

.log-time {
  font-size: 24rpx;
  color: #666;
  display: block;
}

.log-message {
  font-size: 28rpx;
  color: #333;
  display: block;
  margin-top: 8rpx;
}

.empty {
  text-align: center;
  color: #999;
  padding: 40rpx;
}
</style>