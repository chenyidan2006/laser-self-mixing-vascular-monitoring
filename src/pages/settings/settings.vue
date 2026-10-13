<template>
  <view class="settings-page">
    <view class="header">
      <view class="back-button" @click="back">‹ {{ tr('Back') }}</view>
      <view class="header-title">
        <text class="title">{{ t('settings') }}</text>
      </view>
    </view>

    <view class="settings-section">
      <view class="section-title">{{ t('account') }}</view>
      <view class="settings-card">
        <view class="settings-item" @click="goChangePassword">
          <view class="item-left">
            <text class="item-icon">🔐</text>
            <text class="item-text">{{ t('changePassword') }}</text>
          </view>
          <text class="arrow">›</text>
        </view>


      </view>
    </view>

    <view class="settings-section">
      <view class="section-title">{{ t('preferences') }}</view>
      <view class="settings-card">


        <view class="settings-item" @click="clearCache">
          <view class="item-left">
            <text class="item-icon">🗑️</text>
            <text class="item-text">{{ t('clearCache') }}</text>
          </view>
          <view class="item-right">
            <text class="cache-size">{{ cacheSize }}</text>
            <text class="arrow">›</text>
          </view>
        </view>

        <view class="settings-item" @click="toggleLanguage">
          <view class="item-left">
            <text class="item-icon">🌐</text>
            <text class="item-text">{{ t('language') }}</text>
          </view>
          <view class="item-right">
            <text class="lang-text">{{ currentLang === 'en' ? tr("English") : '中文' }}</text>
            <text class="arrow">›</text>
          </view>
        </view>
      </view>
    </view>

    <view class="settings-section">
      <view class="section-title">{{ t('about') }}</view>
      <view class="settings-card">
        <view class="settings-item" @click="goAbout">
          <view class="item-left">
            <text class="item-icon">ℹ️</text>
            <text class="item-text">{{ t('aboutUs') }}</text>
          </view>
          <text class="arrow">›</text>
        </view>

        <view class="settings-item">
          <view class="item-left">
            <text class="item-icon">📌</text>
            <text class="item-text">{{ t('version') }}</text>
          </view>
          <text class="version-text">1.0.0</text>
        </view>
      </view>
    </view>

    <view class="logout-section">
      <button class="logout-btn" @click="logout">{{ t('logout') }}</button>
    </view>
  </view>
</template>

<script>
import { t as translate, getLang } from '@/utils/i18n.js';
import {backToTab} from '@/utils/navigation.js';

export default {
  data() {
    return {
      notificationEnabled: true,
      cacheSize: '--',
      currentLang: 'zh'
    };
  },
  onLoad() {
    this.currentLang = uni.getStorageSync('language') || 'zh';
  },
  onShow() {
    this.currentLang = uni.getStorageSync('language') || 'zh';
    this.loadSettings();
    this.calcCacheSize();
  },
  methods: {
    back(){backToTab('profile');},
    t(key) {
      return translate(key);
    },
    loadSettings() {
      const settings = uni.getStorageSync('settings') || {};
      this.notificationEnabled = settings.notification !== false;
    },
    calcCacheSize() {
      setTimeout(() => {
        const info = uni.getStorageInfoSync(); this.cacheSize = info.currentSize + (this.currentLang === 'zh' ? ' 千字节' : ' KB');
      }, 500);
    },
    toggleNotification(e) {
      this.notificationEnabled = e.detail.value;
      uni.setStorageSync('settings', { notification: this.notificationEnabled });
      uni.showToast({ title: this.t('saved'), icon: 'success' });
    },
    toggleLanguage() {
      const newLang = this.currentLang === 'en' ? 'zh' : 'en';
      uni.setStorageSync('language', newLang);
      this.currentLang = newLang;
      
      uni.showModal({
        title: this.t('languageChanged'),
        content: newLang === 'en' ? this.t('languageEnglish') : this.t('languageChinese'),
        showCancel: false,
        success: () => {
          uni.reLaunch({ url: '/pages/index/index' });
        }
      });
    },
    goChangePassword() {
      uni.navigateTo({ url: '/pages/change-password/change-password' });
    },
    goPrivacy() {
      uni.navigateTo({ url: '/pages/settings/privacy' });
    },
    goAbout() {
      uni.navigateTo({ url: '/pages/about/about' });
    },
    clearCache() {
      uni.showModal({
        title: this.t('clearCache'),
        content: this.t('clearAllCache'),
        success: (res) => {
          if (res.confirm) {
            const token = uni.getStorageSync('token');
            const user = uni.getStorageSync('user');
            const language = uni.getStorageSync('language');
            uni.removeStorageSync('mockTrendData');
            uni.removeStorageSync('isSimulationMode');
            uni.setStorageSync('token', token);
            uni.setStorageSync('user', user);
            uni.setStorageSync('language', language);
            this.cacheSize = '0 KB';
            uni.showToast({ title: this.t('cacheCleared'), icon: 'success' });
          }
        }
      });
    },
    logout() {
      uni.showModal({
        title: this.t('logout'),
        content: this.t('areYouSure'),
        success: (res) => {
          if (res.confirm) {
            uni.removeStorageSync('token');
            uni.removeStorageSync('user');
            uni.reLaunch({ url: '/pages/login/login' });
          }
        }
      });
    }
  }
};
</script>

<style lang="scss" scoped>
.back-button{font-size:28rpx;color:#64748b;padding:16rpx 20rpx 16rpx 0;min-width:100rpx;}
.settings-page {
  min-height: 100vh;
  background: linear-gradient(180deg, #f0f9ff 0%, #f8fafc 50%, #ffffff 100%);
  padding: 0 24rpx calc(60rpx + env(safe-area-inset-bottom));
  box-sizing: border-box;
}

.header {
  padding: 60rpx 0 30rpx;
  display:flex;
  align-items:center;
  gap:20rpx;
}

.header-title {
  display: flex;
  align-items: center;
}

.title {
  font-size: 36rpx;
  font-weight: bold;
  color: #0f172a;
}

.settings-section {
  margin-top: 32rpx;
}

.section-title {
  font-size: 28rpx;
  font-weight: 600;
  color: #64748b;
  text-transform: uppercase;
  letter-spacing: 1px;
  margin-bottom: 16rpx;
  padding: 0 8rpx;
}

.settings-card {
  background: white;
  border-radius: 28rpx;
  box-shadow: 0 8rpx 32rpx rgba(0, 0, 0, 0.06);
}

.settings-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 28rpx 32rpx;
  border-bottom: 2rpx solid #f1f5f9;

  &:last-child {
    border-bottom: none;
  }
}

.item-left {
  display: flex;
  align-items: center;
}

.item-icon {
  font-size: 32rpx;
  margin-right: 20rpx;
}

.item-text {
  font-size: 30rpx;
  color: #0f172a;
}

.item-right {
  display: flex;
  align-items: center;
}

.arrow {
  font-size: 36rpx;
  color: #cbd5e1;
  font-weight: 300;
}

.cache-size,
.lang-text,
.version-text {
  font-size: 26rpx;
  color: #94a3b8;
  margin-right: 8rpx;
}

.logout-section {
  margin-top: 48rpx;
  padding: 0 16rpx;
}

.logout-btn {
  height: 88rpx;
  line-height: 88rpx;
  background: white;
  color: #ef4444;
  border: 2rpx solid #ef4444;
  border-radius: 44rpx;
  font-size: 32rpx;
  font-weight: 500;
  box-shadow: 0 4rpx 16rpx rgba(239, 68, 68, 0.1);

  &::after {
    border: none;
  }
}
</style>
