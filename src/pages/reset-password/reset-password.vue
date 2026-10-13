<template>
  <view class="reset-container">
    <view class="header">
      <view class="back-btn" @click="goBack">
        <text class="back-icon">‹</text>
      </view>
      <text class="header-title">{{ tr("Reset Password") }}</text>
      <view class="placeholder"></view>
    </view>

    <view class="form-card">
      <view class="form-title">{{ tr("Verify Your Phone") }}</view>
      <view class="form-subtitle">{{ tr("We'll send a verification code to your phone") }}</view>

      <view class="input-group">
        <input
          type="text"
          v-model="phone"
          :placeholder="tr('Phone Number')"
          class="input-field"
        />
      </view>

      <view class="input-group sms-row">
        <input
          type="text"
          v-model="code"
          :placeholder="tr('Verification Code')"
          class="input-field code-input"
        />
        <button
          class="sms-btn"
          @click="sendCode"
          :disabled="countdown > 0"
        >
          {{ countdown > 0 ? `${countdown} ${tr('seconds')}` : tr("Get Code") }}
        </button>
      </view>

      <view class="input-group">
        <input
          type="password"
          v-model="newPassword"
          :placeholder="tr('New Password')"
          class="input-field"
        />
      </view>

      <view class="input-group">
        <input
          type="password"
          v-model="confirmPassword"
          :placeholder="tr('Confirm Password')"
          class="input-field"
        />
      </view>

      <button class="submit-btn" @click="resetPassword" :disabled="resetting">
        {{ resetting ? tr("Resetting...") : tr("Reset Password") }}
      </button>

      <view class="back-login" @click="goBack">
        <text>{{ tr("Remember password?") }}</text>
        <text class="link">{{ tr("Back to Login") }}</text>
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
      code: '',
      newPassword: '',
      confirmPassword: '',
      countdown: 0,
      resetting: false
    };
  },
  methods: {
    goBack() {
      uni.navigateBack();
    },
    sendCode() {
      if (!this.phone) {
        uni.showToast({ title: this.tr("Please enter phone number"), icon: 'none' });
        return;
      }
      uni.request({
        url: getAuthUrl('sendSms'),
        method: 'POST',
        data: { phone: this.phone },
        success: (res) => {
          if (res.data.errcode === 0) {
            uni.showToast({ title: this.tr("Code sent"), icon: 'success' });
            this.countdown = 60;
            const timer = setInterval(() => {
              this.countdown--;
              if (this.countdown <= 0) {
                clearInterval(timer);
              }
            }, 1000);
          } else {
            uni.showToast({ title: res.data.msg || this.tr("Failed to send"), icon: 'none' });
          }
        },
        fail: () => {
          uni.showToast({ title: this.tr("Network error"), icon: 'none' });
        }
      });
    },
    resetPassword() {
      if (!this.phone || !this.code || !this.newPassword) {
        uni.showToast({ title: this.tr("Please fill all fields"), icon: 'none' });
        return;
      }
      if (this.newPassword !== this.confirmPassword) {
        uni.showToast({ title: this.tr("Passwords do not match"), icon: 'none' });
        return;
      }
      if (this.newPassword.length < 6) {
        uni.showToast({ title: this.tr("Password at least 6 characters"), icon: 'none' });
        return;
      }
      this.resetting = true;
      uni.request({
        url: getAuthUrl('resetPassword'),
        method: 'POST',
        data: {
          phone: this.phone,
          code: this.code,
          newPassword: this.newPassword
        },
        success: (res) => {
          if (res.data.errcode === 0) {
            uni.showToast({ title: this.tr("Password reset success"), icon: 'success' });
            setTimeout(() => {
              uni.navigateBack();
            }, 1500);
          } else {
            uni.showToast({ title: res.data.msg || this.tr("Reset failed"), icon: 'none' });
          }
        },
        fail: () => {
          uni.showToast({ title: this.tr("Network error"), icon: 'none' });
        },
        complete: () => {
          this.resetting = false;
        }
      });
    }
  }
};
</script>

<style scoped>
.reset-container {
  min-height: 100vh;
  background: #f5f7fa;
  padding: 20rpx;
}

.header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 20rpx 20rpx 40rpx;
}

.back-btn {
  width: 60rpx;
  height: 60rpx;
  display: flex;
  align-items: center;
  justify-content: center;
}

.back-icon {
  font-size: 48rpx;
  color: #333;
}

.header-title {
  font-size: 34rpx;
  font-weight: 600;
  color: #333;
}

.placeholder {
  width: 60rpx;
}

.form-card {
  background: white;
  border-radius: 24rpx;
  padding: 40rpx 30rpx;
  margin: 0 10rpx;
}

.form-title {
  font-size: 40rpx;
  font-weight: 700;
  color: #1e293b;
  margin-bottom: 12rpx;
}

.form-subtitle {
  font-size: 28rpx;
  color: #64748b;
  margin-bottom: 40rpx;
}

.input-group {
  margin-bottom: 24rpx;
}

.input-field {
  width: 100%;
  height: 96rpx;
  background: #f8fafc;
  border-radius: 16rpx;
  padding: 0 30rpx;
  font-size: 30rpx;
  box-sizing: border-box;
}

.sms-row {
  display: flex;
  align-items: center;
  gap: 20rpx;
}

.code-input {
  flex: 1;
}

.sms-btn {
  width: 200rpx;
  height: 96rpx;
  background: #3b82f6;
  color: white;
  border-radius: 16rpx;
  font-size: 26rpx;
  display: flex;
  align-items: center;
  justify-content: center;
  margin: 0;
  padding: 0;
}

.sms-btn[disabled] {
  background: #94a3b8;
}

.submit-btn {
  width: 100%;
  height: 96rpx;
  background: #3b82f6;
  color: white;
  border-radius: 16rpx;
  font-size: 32rpx;
  font-weight: 600;
  margin-top: 20rpx;
  display: flex;
  align-items: center;
  justify-content: center;
}

.submit-btn[disabled] {
  background: #94a3b8;
}

.back-login {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 8rpx;
  margin-top: 40rpx;
  font-size: 28rpx;
  color: #64748b;
}

.link {
  color: #3b82f6;
}
</style>
