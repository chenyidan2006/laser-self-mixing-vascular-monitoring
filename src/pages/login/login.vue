<template>
  <view class="login-container">
    <view class="language-toggle" @click="switchLanguage">{{ uiLang === 'zh' ? '英文' : 'Chinese' }}</view>
    <view class="logo">{{ tr("Health Monitor") }}</view>

    <!-- Tab switch -->
    <view class="tab-bar">
      <view :class="['tab-item', { active: currentTab === 'password' }]" @click="currentTab = 'password'">{{ tr("Password") }}</view>
      <view :class="['tab-item', { active: currentTab === 'sms' }]" @click="currentTab = 'sms'">{{ tr("SMS Code") }}</view>
    </view>

    <!-- Password login -->
    <view v-if="currentTab === 'password'" class="form">
      <input type="text" v-model="phone" :placeholder="tr('Phone Number')" />
      <input type="password" v-model="password" :placeholder="tr('Password')" />
      <button @click="handlePasswordLogin" :loading="logining">{{ tr("Login") }}</button>
    </view>

    <!-- SMS login -->
    <view v-if="currentTab === 'sms'" class="form">
      <input type="text" v-model="phone" :placeholder="tr('Phone Number')"/>
      <view class="sms-row">
        <input type="text" v-model="smsCode" :placeholder="tr('Verification Code')"/>
        <button @click="sendSmsCode" :disabled="smsCountdown > 0" class="sms-btn">
          {{ smsCountdown > 0 ? `${smsCountdown} ${tr('seconds')} ${tr('Get Code')}` : tr("Get Code") }}
        </button>
      </view>
      <button @click="handleSmsLogin" :loading="logining">{{ tr("Login") }}</button>
    </view>

    <!-- Third party login -->
    <view class="third-party">
      <view class="third-title">{{ tr("Other Login Methods") }}</view>
      <view class="third-icons">
        <view class="third-icon" @click="handleWechatLogin" v-if="hasWechat">
          <image src="/static/wechat.png" mode="aspectFit"></image>
          <text>{{ tr("WeChat") }}</text>
        </view>
        <view class="third-icon" @click="handleQQLogin" v-if="hasQQ">
          <image src="/static/qq.png" mode="aspectFit"></image>
          <text>{{ tr("QQ") }}</text>
        </view>
      </view>
    </view>

    <!-- Footer -->
    <view class="footer">
      <text @click="goGuest">{{ tr("Guest Access") }}</text>
      <view class="footer-links">
        <text @click="goResetPassword">{{ tr("Forgot Password?") }}</text>
        <text @click="goRegister">{{ tr("Register") }}</text>
      </view>
    </view>
  </view>
</template>

<script>
import { getAuthUrl } from '@/config/api.js';

export default {
  data() {
    return {
      currentTab: 'password', // 'password' or 'sms'
      password: '',
      phone: '',
      smsCode: '',
      smsCountdown: 0,
      logining: false,
      hasWechat: false,
      hasQQ: false
    };
  },
  onLoad() {
    // 判断平台是否支持微信/QQ登录
    // #ifdef MP-WEIXIN
    this.hasWechat = true;
    // #endif
    // #ifdef APP-PLUS
    plus.oauth.getServices(services => {
      services.forEach(service => {
        if (service.id === 'weixin' && service.isAuthValid) this.hasWechat = true;
        if (service.id === 'qq' && service.isAuthValid) this.hasQQ = true;
      });
    });
    // #endif
  },
  methods: {
    // 密码登录
    handlePasswordLogin() {
      if (!this.phone || !this.password) {
        uni.showToast({ title: this.tr("Please fill in all fields"), icon: 'none' });
        return;
      }
      this.logining = true;
      uni.request({
        url: getAuthUrl('login'),
        method: 'POST',
        data: { phone: this.phone, password: this.password },
        success: (res) => {
          if (res.data.errcode === 0) {
            this.loginSuccess(res.data.data);
          } else {
            uni.showToast({ title: res.data.msg, icon: 'none' });
          }
        },
        fail: () => uni.showToast({ title: this.tr("Network error"), icon: 'none' }),
        complete: () => this.logining = false
      });
    },
    // Send SMS code
    sendSmsCode() {
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
            uni.showToast({ title: this.tr("Verification code sent") });
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
        fail: () => uni.showToast({ title: this.tr("Network error"), icon: 'none' })
      });
    },
    // SMS login
    handleSmsLogin() {
      if (!this.phone || !this.smsCode) {
        uni.showToast({ title: this.tr("Please fill in all fields"), icon: 'none' });
        return;
      }
      this.logining = true;
      uni.request({
        url: getAuthUrl('loginBySms'),
        method: 'POST',
        data: { phone: this.phone, smsCode: this.smsCode },
        success: (res) => {
          if (res.data.errcode === 0) {
            this.loginSuccess(res.data.data);
          } else {
            uni.showToast({ title: res.data.msg, icon: 'none' });
          }
        },
        fail: () => uni.showToast({ title: this.tr("Network error"), icon: 'none' }),
        complete: () => this.logining = false
      });
    },
    // WeChat login
    handleWechatLogin() {
      // #ifdef MP-WEIXIN
      uni.login({
        provider: 'weixin',
        success: (loginRes) => {
          uni.request({
            url: getAuthUrl('wechatLogin'),
            method: 'POST',
            data: { code: loginRes.code },
            success: (res) => {
              if (res.data.errcode === 0) {
                this.loginSuccess(res.data.data);
              } else {
                uni.showToast({ title: res.data.msg, icon: 'none' });
              }
            },
            fail: () => uni.showToast({ title: this.tr("微信登录失败"), icon: 'none' })
          });
        },
        fail: (err) => {
          uni.showToast({ title: this.tr("微信登录失败"), icon: 'none' });
        }
      });
      // #endif
      // #ifdef APP-PLUS
      plus.oauth.getServices(services => {
        const wechat = services.find(s => s.id === 'weixin');
        if (wechat) {
          wechat.authorize((res) => {
            wechat.getUserInfo((userInfo) => {
              uni.request({
                url: getAuthUrl('wechatLogin'),
                method: 'POST',
                data: { openid: userInfo.openid, unionid: userInfo.unionid, userInfo: userInfo },
                success: (res) => {
                  if (res.data.errcode === 0) {
                    this.loginSuccess(res.data.data);
                  } else {
                    uni.showToast({ title: res.data.msg, icon: 'none' });
                  }
                },
                fail: () => uni.showToast({ title: this.tr("微信登录失败"), icon: 'none' })
              });
            }, (err) => {});
          }, (err) => {});
        }
      });
      // #endif
    },
    handleQQLogin() {
      uni.showToast({ title: this.tr("QQ login not implemented"), icon: 'none' });
    },
    // Guest login
    goGuest() {
      uni.request({
        url: 'http://192.144.185.64:3002/api/user/guestLogin',
        method: 'POST',
        success: (res) => {
          if (res.data.errcode === 0) {
            this.loginSuccess(res.data.data);
          } else {
            uni.showToast({ title: res.data.msg, icon: 'none' });
          }
        },
        fail: () => uni.showToast({ title: this.tr("网络错误"), icon: 'none' })
      });
    },
    // 登录成功处理
    loginSuccess(data) {
      const token = typeof data?.token === 'string'
        ? data.token.trim().replace(/^Bearer\s+/i, '').replace(/^['"]|['"]$/g, '')
        : '';
      if (!/^[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+$/.test(token)) {
        uni.removeStorageSync('token');
        uni.showToast({ title: this.tr("登录响应中的 token 无效，请重新登录"), icon: 'none' });
        console.error('Invalid login token shape', { type: typeof data?.token });
        return;
      }
      uni.setStorageSync('token', token);
      uni.setStorageSync('user', data.user);
      uni.switchTab({ url: '/pages/index/index' }); // 根据实际首页调整
    },
    goRegister() {
      uni.navigateTo({ url: '/pages/register/register' });
    },
    goResetPassword() {
      uni.showModal({
        title: this.tr('Forgot Password?'),
        content: this.uiLang === 'zh'
          ? '当前回退服务器不提供密码重置接口，请联系管理员处理。'
          : 'This server version does not provide password reset. Please contact the administrator.',
        showCancel: false
      });
    }
  }
};
</script>

<style lang="scss">
.login-container {
  padding: 40rpx;
  background-color: #fff;
  min-height: 100vh;
}
.logo {
  font-size: 56rpx;
  font-weight: bold;
  color: #2b85e4;
  text-align: center;
  margin: 80rpx 0 60rpx;
}
.tab-bar {
  display: flex;
  border-bottom: 2rpx solid #eee;
  margin-bottom: 40rpx;
  .tab-item {
    flex: 1;
    text-align: center;
    padding: 20rpx 0;
    font-size: 32rpx;
    color: #666;
    &.active {
      color: #2b85e4;
      border-bottom: 4rpx solid #2b85e4;
    }
  }
}
input {
  height: 88rpx;
  background: #f5f5f5;
  border-radius: 44rpx;
  padding: 0 30rpx;
  margin-bottom: 30rpx;
  font-size: 28rpx;
}
.sms-row {
  display: flex;
  input {
    flex: 1;
    margin-right: 20rpx;
  }
  .sms-btn {
    width: 200rpx;
    height: 88rpx;
    line-height: 88rpx;
    background: #fff;
    border: 2rpx solid #2b85e4;
    color: #2b85e4;
    border-radius: 44rpx;
    font-size: 26rpx;
    padding: 0;
  }
}
button {
  height: 88rpx;
  line-height: 88rpx;
  background: linear-gradient(135deg, #2b85e4, #5cadff);
  color: #fff;
  border-radius: 44rpx;
  font-size: 32rpx;
  margin-top: 20rpx;
}
.third-party {
  margin-top: 60rpx;
  text-align: center;
  .third-title {
    color: #999;
    font-size: 26rpx;
    margin-bottom: 30rpx;
    position: relative;
    &::before, &::after {
      content: '';
      position: absolute;
      top: 50%;
      width: 100rpx;
      height: 2rpx;
      background: #eee;
    }
    &::before { left: 40rpx; }
    &::after { right: 40rpx; }
  }
  .third-icons {
    display: flex;
    justify-content: center;
    .third-icon {
      margin: 0 30rpx;
      image {
        width: 80rpx;
        height: 80rpx;
      }
      text {
        display: block;
        font-size: 24rpx;
        color: #666;
      }
    }
  }
}
.footer {
  margin-top: 60rpx;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 20rpx;
  color: #2b85e4;
  font-size: 28rpx;
}

.footer-links {
  display: flex;
  gap: 40rpx;
}
</style>
