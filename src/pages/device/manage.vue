<template>
  <view class="device-manage">
    <view class="header">
      <view class="back-btn" @click="goBack">
        <text class="back-icon">‹</text>
      </view>
      <text class="header-title">Device Management</text>
      <view class="placeholder"></view>
    </view>

    <view class="section">
      <view class="section-title">My Devices</view>
      <view v-if="deviceList.length === 0" class="empty-state">
        <text class="empty-icon">📱</text>
        <text class="empty-text">No devices bound yet</text>
      </view>
      <view v-else>
        <view v-for="(device, index) in deviceList" :key="index" class="device-item">
          <view class="device-icon">📱</view>
          <view class="device-info">
            <text class="device-name">{{ device.deviceName || device.deviceId }}</text>
            <text class="device-id">{{ device.deviceId }}</text>
            <text class="device-time">Bound: {{ formatTime(device.bindTime) }}</text>
          </view>
          <button class="unbind-btn" @click="unbindDevice(device)">Unbind</button>
        </view>
      </view>
    </view>

    <view class="section">
      <view class="section-title">Add New Device</view>
      <button class="add-btn" @click="scanDevice">Scan for Devices</button>
      <text class="add-tip">Make sure the device is powered on and in pairing mode</text>
    </view>

    <uni-popup ref="scanPopup" type="bottom">
      <view class="scan-popup">
        <view class="popup-header">
          <text>Found Devices</text>
          <text class="close" @click="closeScan">×</text>
        </view>
        <scroll-view scroll-y class="device-scroll">
          <view v-if="scanning" class="loading-state">
            <text>Scanning...</text>
          </view>
          <view v-else>
            <view v-for="(item, idx) in scanDevices" :key="idx" class="scan-item" @click="bindDevice(item)">
              <view class="scan-info">
                <text class="scan-name">{{ item.deviceName || 'Unknown' }}</text>
                <text class="scan-id">{{ item.deviceId }}</text>
                <text class="scan-status" :class="item.status">{{ item.status || 'offline' }}</text>
              </view>
              <text class="bind-hint">Tap to bind</text>
            </view>
            <view v-if="scanDevices.length === 0" class="no-devices">No devices found</view>
          </view>
        </scroll-view>
      </view>
    </uni-popup>
  </view>
</template>

<script>
import { getAuthUrl } from '@/config/api.js';

export default {
  data() {
    return {
      deviceList: [],
      scanDevices: [],
      scanning: false
    };
  },
  onShow() {
    this.loadDevices();
  },
  methods: {
    loadDevices() {
      uni.request({
        url: getAuthUrl('devices'),
        header: { Authorization: 'Bearer ' + uni.getStorageSync('token') },
        success: (res) => {
          if (res.data.errcode === 0) {
            this.deviceList = res.data.data || [];
          } else {
            console.error('Load devices failed:', res.data.msg);
          }
        },
        fail: (err) => {
          console.error('Network error:', err);
        }
      });
    },

    scanDevice() {
      this.scanDevices = [];
      this.scanning = true;
      this.$refs.scanPopup.open();

      uni.request({
        url: getAuthUrl('devicesSearch'),
        header: { Authorization: 'Bearer ' + uni.getStorageSync('token') },
        success: (res) => {
          if (res.data.errcode === 0) {
            this.scanDevices = res.data.data || [];
          }
        },
        complete: () => {
          this.scanning = false;
        }
      });
    },

    closeScan() {
      this.$refs.scanPopup.close();
    },

    bindDevice(device) {
      uni.showLoading({ title: 'Binding...' });
      uni.request({
        url: getAuthUrl('devicesBind'),
        method: 'POST',
        header: {
          Authorization: 'Bearer ' + uni.getStorageSync('token'),
          'Content-Type': 'application/json'
        },
        data: { deviceId: device.deviceId, deviceName: device.deviceName },
        success: (res) => {
          if (res.data.errcode === 0) {
            uni.setStorageSync('selectedDeviceId', device.deviceId);
            uni.showToast({ title: 'Bound successfully', icon: 'success' });
            this.closeScan();
            this.loadDevices();
          } else {
            uni.showToast({ title: res.data.msg || 'Bind failed', icon: 'none' });
          }
        },
        fail: () => {
          uni.showToast({ title: 'Network error', icon: 'none' });
        },
        complete: () => {
          uni.hideLoading();
        }
      });
    },

    unbindDevice(device) {
      uni.showModal({
        title: 'Confirm',
        content: `Unbind device ${device.deviceId}?`,
        success: (res) => {
          if (res.confirm) {
            uni.showLoading({ title: 'Unbinding...' });
            uni.request({
              url: getAuthUrl(`devices/${device.deviceId}`),
              method: 'DELETE',
              header: { Authorization: 'Bearer ' + uni.getStorageSync('token') },
              success: (res) => {
                if (res.data.errcode === 0) {
                  uni.showToast({ title: 'Unbound', icon: 'success' });
                  this.loadDevices();
                } else {
                  uni.showToast({ title: res.data.msg || 'Unbind failed', icon: 'none' });
                }
              },
              fail: () => {
                uni.showToast({ title: 'Network error', icon: 'none' });
              },
              complete: () => {
                uni.hideLoading();
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
    },

    goBack() {
      uni.navigateBack();
    }
  }
};
</script>

<style lang="scss" scoped>
.device-manage {
  min-height: 100vh;
  background: #f5f5f5;
}

.header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 20px;
  background: #fff;
}

.back-btn {
  width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.back-icon {
  font-size: 32px;
  color: #333;
}

.header-title {
  font-size: 18px;
  font-weight: 600;
  color: #333;
}

.placeholder {
  width: 40px;
}

.section {
  background: #fff;
  margin-top: 10px;
  padding: 20px;
}

.section-title {
  font-size: 16px;
  font-weight: 600;
  color: #333;
  margin-bottom: 15px;
}

.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 40px 0;

  .empty-icon {
    font-size: 48px;
    margin-bottom: 10px;
  }

  .empty-text {
    font-size: 14px;
    color: #999;
  }
}

.device-item {
  display: flex;
  align-items: center;
  padding: 15px 0;
  border-bottom: 1px solid #f0f0f0;

  &:last-child {
    border-bottom: none;
  }
}

.device-icon {
  font-size: 32px;
  margin-right: 15px;
}

.device-info {
  flex: 1;
  display: flex;
  flex-direction: column;

  .device-name {
    font-size: 16px;
    color: #333;
    font-weight: 500;
  }

  .device-id {
    font-size: 12px;
    color: #999;
    margin-top: 2px;
  }

  .device-time {
    font-size: 12px;
    color: #999;
    margin-top: 2px;
  }
}

.unbind-btn {
  background: #fff;
  color: #e54d42;
  border: 1px solid #e54d42;
  border-radius: 20px;
  font-size: 12px;
  padding: 5px 15px;
}

.add-btn {
  width: 100%;
  height: 44px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: #fff;
  border: none;
  border-radius: 22px;
  font-size: 16px;
}

.add-tip {
  display: block;
  text-align: center;
  color: #999;
  font-size: 12px;
  margin-top: 10px;
}

.scan-popup {
  background: #fff;
  border-radius: 20px 20px 0 0;
  padding: 20px;
  max-height: 60vh;

  .popup-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 16px;
    font-weight: 600;
    padding-bottom: 15px;
    border-bottom: 1px solid #f0f0f0;

    .close {
      font-size: 28px;
      color: #999;
    }
  }

  .device-scroll {
    max-height: 400px;
    margin-top: 15px;
  }

  .loading-state {
    text-align: center;
    padding: 30px;
    color: #999;
  }

  .scan-item {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 15px 0;
    border-bottom: 1px solid #f0f0f0;

    &:last-child {
      border-bottom: none;
    }
  }

  .scan-info {
    display: flex;
    flex-direction: column;

    .scan-name {
      font-size: 16px;
      color: #333;
      font-weight: 500;
    }

    .scan-id {
      font-size: 12px;
      color: #999;
      margin-top: 2px;
    }

    .scan-status {
      font-size: 11px;
      margin-top: 3px;
      padding: 2px 8px;
      border-radius: 10px;
      width: fit-content;

      &.online {
        background: #e6f7e6;
        color: #52c41a;
      }

      &.offline {
        background: #f5f5f5;
        color: #999;
      }
    }
  }

  .bind-hint {
    font-size: 12px;
    color: #667eea;
  }

  .no-devices {
    text-align: center;
    padding: 30px;
    color: #999;
  }
}
</style>