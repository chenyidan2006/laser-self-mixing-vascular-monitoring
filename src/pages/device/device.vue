<template>
  <view class="device-container">
    <view class="nav-bar">
      <view class="nav-back" @click="goBack">
        <text class="back-icon">‹</text>
        <text class="back-text">Back</text>
      </view>
      <text class="nav-title">Device</text>
      <view class="nav-placeholder"></view>
    </view>

    <view class="search-card">
      <text class="search-title">Search Device</text>
      <view class="search-form">
        <input
          class="search-input"
          v-model="searchKeyword"
          placeholder="Enter device ID (e.g., ESP32)"
          placeholder-class="input-placeholder"
        />
        <button class="search-btn" @click="searchDevices" :disabled="searching">
          {{ searching ? 'Searching...' : 'Search' }}
        </button>
      </view>

      <view v-if="searchResults.length > 0" class="search-results">
        <view
          v-for="device in searchResults"
          :key="device.deviceId"
          class="search-item"
        >
          <view class="search-info">
            <text class="search-id">{{ device.deviceId }}</text>
            <view class="search-status">
              <view class="status-dot" :class="device.status"></view>
              <text>{{ device.status || 'offline' }}</text>
            </view>
          </view>
          <button
            class="bind-quick-btn"
            @click="quickBindDevice(device.deviceId)"
            v-if="!isDeviceBound(device.deviceId)"
          >
            Bind
          </button>
          <text class="already-bound" v-else>Bound</text>
        </view>
      </view>
    </view>

    <view class="add-card">
      <text class="add-title">Manual Bind</text>
      <view class="add-form">
        <input
          class="device-input"
          v-model="newDeviceId"
          placeholder="Enter device ID"
          placeholder-class="input-placeholder"
        />
        <button class="bind-btn" @click="bindDevice" :disabled="binding">
          {{ binding ? 'Binding...' : 'Bind Device' }}
        </button>
      </view>
    </view>

    <view class="device-list-card">
      <view class="list-header">
        <text class="list-title">My Devices</text>
        <text class="list-count">{{ deviceList.length }} device(s)</text>
      </view>
      <view v-if="loading" class="loading-state">
        <text>Loading...</text>
      </view>
      <view v-else-if="deviceList.length === 0" class="empty-state">
        <text class="empty-icon">📱</text>
        <text class="empty-text">No devices bound yet</text>
      </view>
      <view v-else class="device-items">
        <view
          v-for="device in deviceList"
          :key="device.deviceId"
          class="device-item"
        >
          <view class="device-info">
            <text class="device-id">{{ device.deviceId }}</text>
            <view class="device-status">
              <text class="bind-time">Bound: {{ formatTime(device.bindTime) }}</text>
            </view>
          </view>
          <view class="device-actions">
            <button class="unbind-btn" @click="unbindDevice(device.deviceId)">Unbind</button>
          </view>
        </view>
      </view>
    </view>
  </view>
</template>

<script>
import { getAuthUrl } from '@/config/api.js';

export default {
  data() {
    return {
      deviceList: [],
      newDeviceId: '',
      binding: false,
      loading: false,
      searchKeyword: '',
      searching: false,
      searchResults: []
    };
  },
  onShow() {
    this.loadDevices();
  },
  methods: {
    goBack() {
      uni.navigateBack();
    },
    loadDevices() {
      this.loading = true;
      uni.request({
        url: getAuthUrl('devices'),
        header: { Authorization: 'Bearer ' + uni.getStorageSync('token') },
        success: (res) => {
          if (res.data.errcode === 0) {
            this.deviceList = res.data.data || [];
          }
        },
        fail: () => {
          uni.showToast({ title: 'Network error', icon: 'none' });
        },
        complete: () => {
          this.loading = false;
        }
      });
    },
    searchDevices() {
      this.searching = true;
      uni.request({
        url: getAuthUrl('devicesSearch') + '?keyword=' + encodeURIComponent(this.searchKeyword.trim()),
        header: { Authorization: 'Bearer ' + uni.getStorageSync('token') },
        success: (res) => {
          if (res.data.errcode === 0) {
            this.searchResults = res.data.data || [];
          }
        },
        fail: () => {
          uni.showToast({ title: 'Search failed', icon: 'none' });
        },
        complete: () => {
          this.searching = false;
        }
      });
    },
    isDeviceBound(deviceId) {
      return this.deviceList.some(d => d.deviceId === deviceId);
    },
    quickBindDevice(deviceId) {
      this.binding = true;
      uni.request({
        url: getAuthUrl('devicesBind'),
        method: 'POST',
        header: {
          Authorization: 'Bearer ' + uni.getStorageSync('token'),
          'Content-Type': 'application/json'
        },
        data: { deviceId },
        success: (res) => {
          if (res.data.errcode === 0) {
            uni.showToast({ title: 'Bound', icon: 'success' });
            this.loadDevices();
          } else {
            uni.showToast({ title: res.data.msg, icon: 'none' });
          }
        },
        fail: () => {
          uni.showToast({ title: 'Bind failed', icon: 'none' });
        },
        complete: () => {
          this.binding = false;
        }
      });
    },
    bindDevice() {
      if (!this.newDeviceId.trim()) {
        uni.showToast({ title: 'Enter device ID', icon: 'none' });
        return;
      }
      this.binding = true;
      uni.request({
        url: getAuthUrl('devicesBind'),
        method: 'POST',
        header: {
          Authorization: 'Bearer ' + uni.getStorageSync('token'),
          'Content-Type': 'application/json'
        },
        data: { deviceId: this.newDeviceId.trim() },
        success: (res) => {
          if (res.data.errcode === 0) {
            uni.showToast({ title: 'Bound', icon: 'success' });
            this.newDeviceId = '';
            this.loadDevices();
          } else {
            uni.showToast({ title: res.data.msg, icon: 'none' });
          }
        },
        fail: () => {
          uni.showToast({ title: 'Bind failed', icon: 'none' });
        },
        complete: () => {
          this.binding = false;
        }
      });
    },
    unbindDevice(deviceId) {
      uni.showModal({
        title: 'Confirm',
        content: `Unbind device ${deviceId}?`,
        success: (res) => {
          if (res.confirm) {
            uni.request({
              url: getAuthUrl(`devices/${deviceId}`),
              method: 'DELETE',
              header: { Authorization: 'Bearer ' + uni.getStorageSync('token') },
              success: (res) => {
                if (res.data.errcode === 0) {
                  uni.showToast({ title: 'Unbound', icon: 'success' });
                  this.loadDevices();
                } else {
                  uni.showToast({ title: res.data.msg, icon: 'none' });
                }
              },
              fail: () => {
                uni.showToast({ title: 'Unbind failed', icon: 'none' });
              }
            });
          }
        }
      });
    },
    formatTime(time) {
      if (!time) return '';
      const date = new Date(time);
      return `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-${String(date.getDate()).padStart(2, '0')}`;
    }
  }
};
</script>

<style lang="scss" scoped>
.device-container {
  min-height: 100vh;
  background: #f5f5f5;
  padding: 0 15px;
}

.nav-bar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 20px 0;
  background: #fff;
}

.nav-back {
  display: flex;
  align-items: center;
}

.back-icon {
  font-size: 28px;
  color: #333;
}

.back-text {
  font-size: 14px;
  color: #333;
  margin-left: 2px;
}

.nav-title {
  font-size: 16px;
  font-weight: 600;
  color: #333;
}

.nav-placeholder {
  width: 60px;
}

.search-card,
.add-card,
.device-list-card {
  background: #fff;
  border-radius: 12px;
  padding: 15px;
  margin-bottom: 10px;
}

.search-title,
.add-title {
  font-size: 16px;
  font-weight: 600;
  color: #333;
  margin-bottom: 12px;
}

.search-form,
.add-form {
  display: flex;
  gap: 10px;
}

.search-input,
.device-input {
  flex: 1;
  height: 40px;
  border: 1px solid #ddd;
  border-radius: 8px;
  padding: 0 12px;
  font-size: 14px;
}

.search-btn,
.bind-btn {
  height: 40px;
  padding: 0 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: #fff;
  border: none;
  border-radius: 8px;
  font-size: 14px;
}

.search-results {
  margin-top: 15px;
}

.search-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 0;
  border-bottom: 1px solid #f0f0f0;

  &:last-child {
    border-bottom: none;
  }
}

.search-info {
  display: flex;
  flex-direction: column;
}

.search-id {
  font-size: 14px;
  color: #333;
  font-weight: 500;
}

.search-status {
  display: flex;
  align-items: center;
  margin-top: 4px;

  .status-dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    margin-right: 6px;

    &.online {
      background: #52c41a;
    }

    &.offline {
      background: #999;
    }
  }

  text {
    font-size: 12px;
    color: #999;
  }
}

.bind-quick-btn {
  padding: 6px 16px;
  background: #667eea;
  color: #fff;
  border: none;
  border-radius: 16px;
  font-size: 12px;
}

.already-bound {
  font-size: 12px;
  color: #999;
}

.list-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 12px;
}

.list-title {
  font-size: 16px;
  font-weight: 600;
  color: #333;
}

.list-count {
  font-size: 12px;
  color: #999;
}

.loading-state,
.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 30px 0;
  color: #999;
  font-size: 14px;
}

.empty-icon {
  font-size: 48px;
  margin-bottom: 10px;
}

.device-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 0;
  border-bottom: 1px solid #f0f0f0;

  &:last-child {
    border-bottom: none;
  }
}

.device-info {
  display: flex;
  flex-direction: column;
}

.device-id {
  font-size: 14px;
  color: #333;
  font-weight: 500;
}

.device-status {
  margin-top: 4px;
}

.bind-time {
  font-size: 12px;
  color: #999;
}

.device-actions {
  display: flex;
  gap: 8px;
}

.unbind-btn {
  padding: 6px 16px;
  background: #fff;
  color: #e54d42;
  border: 1px solid #e54d42;
  border-radius: 16px;
  font-size: 12px;
}
</style>