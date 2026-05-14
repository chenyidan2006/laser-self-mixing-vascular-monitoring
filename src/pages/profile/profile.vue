<template>
  <view class="profile-page">
    <view class="header">
      <view class="user-info">
        <view class="avatar-wrapper">
          <text class="avatar">{{ user?.nickname?.charAt(0) || 'U' }}</text>
        </view>
        <view class="greeting">
          <text class="greeting-text">{{ user?.nickname || 'Guest' }}</text>
          <text class="phone-text">{{ user?.phone || (user?.is_guest ? 'Guest Mode' : 'No phone') }}</text>
        </view>
      </view>
      <view class="header-actions">
        <view class="action-btn" @click="goSettings">
          <text class="action-icon">⚙️</text>
        </view>
      </view>
    </view>

    <view class="menu-section">
      <view class="section-header">
        <text class="section-title">Menu</text>
      </view>

      <view class="menu-card">
        <view class="menu-item" @click="goEditProfile">
          <view class="menu-icon">👤</view>
          <view class="menu-content">
            <text class="menu-title">Personal Info</text>
            <text class="menu-desc">View and edit profile</text>
          </view>
          <text class="arrow">›</text>
        </view>

        <view class="menu-item" @click="goDeviceManage">
          <view class="menu-icon">📱</view>
          <view class="menu-content">
            <text class="menu-title">Device Management</text>
            <text class="menu-desc">Bind/unbind devices</text>
          </view>
          <text class="arrow">›</text>
        </view>
      </view>
    </view>

    <view class="menu-section">
      <view class="section-header">
        <text class="section-title">Settings</text>
      </view>

      <view class="menu-card">
        <view class="menu-item" @click="goSettings">
          <view class="menu-icon">⚙️</view>
          <view class="menu-content">
            <text class="menu-title">Settings</text>
            <text class="menu-desc">Account, notifications</text>
          </view>
          <text class="arrow">›</text>
        </view>

        <view class="menu-item" @click="goHelp">
          <view class="menu-icon">❓</view>
          <view class="menu-content">
            <text class="menu-title">Help & Feedback</text>
            <text class="menu-desc">FAQ, feedback</text>
          </view>
          <text class="arrow">›</text>
        </view>

        <view class="menu-item" @click="goAbout">
          <view class="menu-icon">ℹ️</view>
          <view class="menu-content">
            <text class="menu-title">About Us</text>
            <text class="menu-desc">Version, privacy policy</text>
          </view>
          <text class="arrow">›</text>
        </view>
      </view>
    </view>

    <view class="action-section">
      <button v-if="!user?.is_guest" class="logout-btn" @click="logout">Logout</button>
      <button v-else class="login-btn" @click="goLogin">Login / Register</button>
    </view>
  </view>
</template>

<script>
export default {
  data() {
    return {
      user: null
    };
  },
  onShow() {
    const storedUser = uni.getStorageSync('user');
    if (storedUser) {
      this.user = storedUser;
    } else {
      this.user = { is_guest: true, nickname: 'Guest', phone: 'Guest Mode' };
    }
  },
  methods: {
    goEditProfile() {
      if (this.user?.is_guest) {
        this.goLogin();
        return;
      }
      uni.navigateTo({ url: '/pages/profile/editProfile' });
    },
    goDeviceManage() {
      if (this.user?.is_guest) {
        this.goLogin();
        return;
      }
      uni.navigateTo({ url: '/pages/device/device' });
    },
    goSettings() {
      uni.navigateTo({ url: '/pages/settings/settings' });
    },
    goHelp() {
      uni.navigateTo({ url: '/pages/help/help' });
    },
    goAbout() {
      uni.navigateTo({ url: '/pages/about/about' });
    },
    goLogin() {
      uni.reLaunch({ url: '/pages/login/login' });
    },
    logout() {
      uni.showModal({
        title: 'Confirm',
        content: 'Are you sure you want to logout?',
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

<style lang="scss">
.profile-page {
  min-height: 100vh;
  background: linear-gradient(180deg, #f0f9ff 0%, #f8fafc 50%, #ffffff 100%);
  padding: 0 24rpx 120rpx;
}

.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 60rpx 0 30rpx;
}

.user-info {
  display: flex;
  align-items: center;
  gap: 20rpx;
}

.avatar-wrapper {
  width: 96rpx;
  height: 96rpx;
  border-radius: 50%;
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
  display: flex;
  align-items: center;
  justify-content: center;
}

.avatar {
  font-size: 40rpx;
  color: #fff;
  font-weight: bold;
}

.greeting {
  display: flex;
  flex-direction: column;
}

.greeting-text {
  font-size: 36rpx;
  font-weight: bold;
  color: #0f172a;
}

.phone-text {
  font-size: 26rpx;
  color: #64748b;
  margin-top: 4rpx;
}

.header-actions {
  display: flex;
  gap: 16rpx;
}

.action-btn {
  width: 72rpx;
  height: 72rpx;
  border-radius: 50%;
  background: rgba(255, 255, 255, 0.9);
  display: flex;
  align-items: center;
  justify-content: center;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.08);
}

.action-icon {
  font-size: 32rpx;
}

.menu-section {
  margin-top: 32rpx;
}

.section-header {
  margin-bottom: 16rpx;
  padding: 0 8rpx;
}

.section-title {
  font-size: 28rpx;
  font-weight: 600;
  color: #64748b;
  text-transform: uppercase;
  letter-spacing: 1px;
}

.menu-card {
  background: white;
  border-radius: 28rpx;
  padding: 8rpx 0;
  box-shadow: 0 8rpx 32rpx rgba(0, 0, 0, 0.06);
}

.menu-item {
  display: flex;
  align-items: center;
  padding: 28rpx 32rpx;
  border-bottom: 2rpx solid #f1f5f9;

  &:last-child {
    border-bottom: none;
  }
}

.menu-icon {
  width: 72rpx;
  height: 72rpx;
  border-radius: 20rpx;
  background: #f8fafc;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 32rpx;
  margin-right: 20rpx;
}

.menu-content {
  flex: 1;
  display: flex;
  flex-direction: column;
}

.menu-title {
  font-size: 30rpx;
  font-weight: 600;
  color: #0f172a;
  margin-bottom: 4rpx;
}

.menu-desc {
  font-size: 24rpx;
  color: #94a3b8;
}

.arrow {
  font-size: 36rpx;
  color: #cbd5e1;
  font-weight: 300;
}

.action-section {
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

.login-btn {
  height: 88rpx;
  line-height: 88rpx;
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
  color: #fff;
  border-radius: 44rpx;
  font-size: 32rpx;
  font-weight: 500;
  box-shadow: 0 8rpx 32rpx rgba(99, 102, 241, 0.3);

  &::after {
    border: none;
  }
}
</style>
