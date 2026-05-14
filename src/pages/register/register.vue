<template>
  <view class="register-page">
    <view class="header">
      <view class="back-btn" @click="goBack">
        <text class="back-icon">‹</text>
      </view>
      <text class="header-title">Register</text>
      <view class="placeholder"></view>
    </view>

    <view class="form-container">
      <view class="form-header">
        <text class="form-title">Create Account</text>
        <text class="form-desc">Join us to monitor your health</text>
      </view>

      <view class="input-group">
        <view class="input-item">
          <text class="input-label">Phone</text>
          <input type="text" v-model="phone" placeholder="Enter phone number" class="input-field" />
        </view>

        <view class="input-item">
          <text class="input-label">Verification Code</text>
          <view class="sms-row">
            <input type="text" v-model="smsCode" placeholder="Enter code" class="input-field" />
            <button class="sms-btn" @click="sendSmsCode" :disabled="smsCountdown > 0">
              {{ smsCountdown > 0 ? `${smsCountdown}s` : 'Get Code' }}
            </button>
          </view>
        </view>

        <view class="input-item">
          <text class="input-label">Password</text>
          <input type="password" v-model="password" placeholder="Enter password" class="input-field" />
        </view>

        <view class="input-item">
          <text class="input-label">Confirm Password</text>
          <input type="password" v-model="confirmPassword" placeholder="Confirm password" class="input-field" />
        </view>
      </view>

      <view class="agreement" @click="agree = !agree">
        <view class="checkbox" :class="{ checked: agree }">
          <text class="check-icon" v-if="agree">✓</text>
        </view>
        <text class="agreement-text">I agree to the <text class="link" @click.stop="showTerms">Terms of Service</text> and <text class="link" @click.stop="showPrivacy">Privacy Policy</text></text>
      </view>

      <button class="register-btn" @click="handleRegister" :loading="registering">Register</button>

      <view class="divider">
        <view class="divider-line"></view>
        <text class="divider-text">Or continue with</text>
        <view class="divider-line"></view>
      </view>

      <view class="third-party">
        <view class="third-btn" @click="handleWechatLogin">
          <text class="third-icon">💬</text>
          <text class="third-text">WeChat</text>
        </view>
        <view class="third-btn" @click="handleQQLogin">
          <text class="third-icon">🐧</text>
          <text class="third-text">QQ</text>
        </view>
      </view>

      <view class="footer">
        <text class="footer-text">Already have an account?</text>
        <text class="footer-link" @click="goLogin">Login</text>
      </view>
    </view>
  </view>
</template>

<script>
import { getAuthUrl } from '@/config/api.js';

export default {
  data() {
    return {
      phone: '',
      smsCode: '',
      password: '',
      confirmPassword: '',
      smsCountdown: 0,
      registering: false,
      agree: false
    };
  },
  methods: {
    goBack() {
      uni.navigateBack();
    },
    sendSmsCode() {
      if (!this.phone) {
        uni.showToast({ title: 'Please enter phone number', icon: 'none' });
        return;
      }
      uni.request({
        url: getAuthUrl('sendSms'),
        method: 'POST',
        data: { phone: this.phone },
        success: (res) => {
          if (res.data.errcode === 0) {
            uni.showToast({ title: 'Code sent', icon: 'success' });
            this.smsCountdown = 60;
            const timer = setInterval(() => {
              if (this.smsCountdown <= 1) {
                clearInterval(timer);
                this.smsCountdown = 0;
              } else {
                this.smsCountdown--;
              }
            }, 1000);
          } else {
            uni.showToast({ title: res.data.msg, icon: 'none' });
          }
        },
        fail: () => uni.showToast({ title: 'Network error', icon: 'none' })
      });
    },
    handleRegister() {
      if (!this.agree) {
        uni.showToast({ title: 'Please agree to the terms', icon: 'none' });
        return;
      }
      if (!this.phone || !this.smsCode || !this.password || !this.confirmPassword) {
        uni.showToast({ title: 'Please fill in all fields', icon: 'none' });
        return;
      }
      if (this.password !== this.confirmPassword) {
        uni.showToast({ title: 'Passwords do not match', icon: 'none' });
        return;
      }
      this.registering = true;
      uni.request({
        url: getAuthUrl('register'),
        method: 'POST',
        data: {
          username: this.phone,
          phone: this.phone,
          password: this.password
        },
        success: (res) => {
          if (res.data.errcode === 0) {
            uni.showToast({ title: 'Registration successful', icon: 'success' });
            setTimeout(() => {
              this.autoLoginAfterRegister();
            }, 1500);
          } else {
            uni.showToast({ title: res.data.msg, icon: 'none' });
          }
        },
        fail: () => {
          this.handleRegisterFail();
        },
        complete: () => this.registering = false
      });
    },
    autoLoginAfterRegister() {
      uni.request({
        url: getAuthUrl('login'),
        method: 'POST',
        data: { username: this.phone, password: this.password },
        success: (res) => {
          if (res.data.errcode === 0) {
            this.loginSuccess(res.data.data);
          } else {
            this.handleRegisterFail();
          }
        },
        fail: () => {
          this.handleRegisterFail();
        }
      });
    },
    handleRegisterFail() {
      const mockUser = {
        token: 'guest_token_' + Date.now(),
        user: {
          id: 'guest_' + Date.now(),
          username: this.phone || 'Guest',
          phone: this.phone || '13800138000',
          avatar: ''
        }
      };
      uni.setStorageSync('token', mockUser.token);
      uni.setStorageSync('user', mockUser.user);
      uni.showToast({ title: 'Using guest mode', icon: 'none' });
      setTimeout(() => {
        uni.switchTab({ url: '/pages/index/index' });
      }, 1000);
    },
    loginSuccess(data) {
      uni.setStorageSync('token', data.token);
      uni.setStorageSync('user', data.user);
      uni.switchTab({ url: '/pages/index/index' });
    },
    handleWechatLogin() {
      uni.showToast({ title: 'WeChat login not implemented', icon: 'none' });
    },
    handleQQLogin() {
      uni.showToast({ title: 'QQ login not implemented', icon: 'none' });
    },
    showTerms() {
      uni.showModal({
        title: 'Terms of Service',
        content: 'By using this app, you agree to be bound by these terms...',
        showCancel: false
      });
    },
    showPrivacy() {
      uni.showModal({
        title: 'Privacy Policy',
        content: 'We collect health data to provide monitoring services...',
        showCancel: false
      });
    },
    goLogin() {
      uni.navigateBack();
    }
  }
};
</script>

<style lang="scss">
.register-page {
  min-height: 100vh;
  background: linear-gradient(180deg, #f0f9ff 0%, #f8fafc 50%, #ffffff 100%);
}

.header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 60rpx 24rpx 20rpx;
}

.back-btn {
  width: 72rpx;
  height: 72rpx;
  border-radius: 50%;
  background: rgba(255, 255, 255, 0.9);
  display: flex;
  align-items: center;
  justify-content: center;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.08);
}

.back-icon {
  font-size: 44rpx;
  font-weight: bold;
  color: #0f172a;
}

.header-title {
  font-size: 36rpx;
  font-weight: 700;
  color: #0f172a;
}

.placeholder {
  width: 72rpx;
}

.form-container {
  padding: 40rpx 24rpx;
}

.form-header {
  margin-bottom: 48rpx;
}

.form-title {
  display: block;
  font-size: 48rpx;
  font-weight: 800;
  color: #0f172a;
  margin-bottom: 12rpx;
}

.form-desc {
  display: block;
  font-size: 28rpx;
  color: #64748b;
}

.input-group {
  display: flex;
  flex-direction: column;
  gap: 24rpx;
  margin-bottom: 32rpx;
}

.input-item {
  display: flex;
  flex-direction: column;
}

.input-label {
  font-size: 26rpx;
  font-weight: 600;
  color: #374151;
  margin-bottom: 12rpx;
}

.input-field {
  height: 88rpx;
  background: white;
  border-radius: 20rpx;
  padding: 0 24rpx;
  font-size: 28rpx;
  border: 2rpx solid #e5e7eb;
  box-shadow: 0 4rpx 12rpx rgba(0, 0, 0, 0.04);
}

.sms-row {
  display: flex;
  gap: 16rpx;

  .input-field {
    flex: 1;
  }

  .sms-btn {
    width: 200rpx;
    height: 88rpx;
    background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
    color: white;
    border: none;
    border-radius: 20rpx;
    font-size: 26rpx;
    font-weight: 600;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 0;
    margin: 0;

    &[disabled] {
      background: #e5e7eb;
      color: #9ca3af;
    }
  }
}

.agreement {
  display: flex;
  align-items: flex-start;
  gap: 16rpx;
  margin-bottom: 32rpx;
}

.checkbox {
  width: 40rpx;
  height: 40rpx;
  border-radius: 10rpx;
  border: 2rpx solid #d1d5db;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
  margin-top: 4rpx;

  &.checked {
    background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
    border-color: #6366f1;
  }
}

.check-icon {
  color: white;
  font-size: 24rpx;
  font-weight: bold;
}

.agreement-text {
  font-size: 24rpx;
  color: #6b7280;
  line-height: 1.5;
}

.link {
  color: #6366f1;
  font-weight: 500;
}

.register-btn {
  width: 100%;
  height: 96rpx;
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
  color: white;
  border: none;
  border-radius: 48rpx;
  font-size: 32rpx;
  font-weight: 600;
  box-shadow: 0 8rpx 32rpx rgba(99, 102, 241, 0.3);
  margin-bottom: 48rpx;

  &::after {
    border: none;
  }
}

.divider {
  display: flex;
  align-items: center;
  gap: 24rpx;
  margin-bottom: 32rpx;
}

.divider-line {
  flex: 1;
  height: 2rpx;
  background: #e5e7eb;
}

.divider-text {
  font-size: 24rpx;
  color: #9ca3af;
}

.third-party {
  display: flex;
  gap: 24rpx;
  margin-bottom: 48rpx;
}

.third-btn {
  flex: 1;
  height: 96rpx;
  background: white;
  border: 2rpx solid #e5e7eb;
  border-radius: 24rpx;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12rpx;
  box-shadow: 0 4rpx 12rpx rgba(0, 0, 0, 0.04);
}

.third-icon {
  font-size: 36rpx;
}

.third-text {
  font-size: 28rpx;
  font-weight: 500;
  color: #374151;
}

.footer {
  display: flex;
  justify-content: center;
  gap: 8rpx;
}

.footer-text {
  font-size: 26rpx;
  color: #6b7280;
}

.footer-link {
  font-size: 26rpx;
  color: #6366f1;
  font-weight: 600;
}
</style>
