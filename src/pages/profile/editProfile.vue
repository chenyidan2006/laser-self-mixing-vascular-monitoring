<template>
  <view class="edit-profile-page">
    <view class="header">
      <view class="back-btn" @click="goBack">
        <text class="back-icon">‹</text>
      </view>
      <text class="header-title">Edit Profile</text>
      <view class="placeholder"></view>
    </view>

    <view class="avatar-section">
      <view class="avatar-wrapper" @click="changeAvatar">
        <text class="avatar">{{ userInfo?.nickname?.charAt(0) || 'U' }}</text>
        <view class="avatar-edit">
          <text class="edit-icon">✎</text>
        </view>
      </view>
      <text class="avatar-hint">Tap to change avatar</text>
    </view>

    <view class="form-container">
      <view class="form-item">
        <text class="form-label">Nickname</text>
        <input type="text" v-model="formData.nickname" placeholder="Enter nickname" class="form-input" />
      </view>

      <view class="form-item">
        <text class="form-label">Gender</text>
        <picker :value="formData.gender" :range="genderOptions" range-key="text" @change="onGenderChange">
          <view class="form-picker">
            <text>{{ genderOptions[formData.gender]?.text || 'Select gender' }}</text>
            <text class="picker-arrow">›</text>
          </view>
        </picker>
      </view>

      <view class="form-item">
        <text class="form-label">Height (cm)</text>
        <input type="number" v-model="formData.height" placeholder="Enter height" class="form-input" />
      </view>

      <view class="form-item">
        <text class="form-label">Weight (kg)</text>
        <input type="number" v-model="formData.weight" placeholder="Enter weight" class="form-input" />
      </view>

      <view class="form-item">
        <text class="form-label">Birthday</text>
        <picker mode="date" :value="formData.birthday" @change="onBirthdayChange">
          <view class="form-picker">
            <text>{{ formData.birthday || 'Select birthday' }}</text>
            <text class="picker-arrow">›</text>
          </view>
        </picker>
      </view>

      <button class="save-btn" @click="saveProfile">Save</button>
    </view>
  </view>
</template>

<script>
import { getAuthUrl } from '@/config/api.js';

export default {
  data() {
    return {
      userInfo: {},
      formData: {
        nickname: '',
        gender: 0,
        height: '',
        weight: '',
        birthday: ''
      },
      genderOptions: [
        { text: 'Secret', value: 0 },
        { text: 'Male', value: 1 },
        { text: 'Female', value: 2 }
      ]
    };
  },
  onLoad() {
    this.loadUserInfo();
  },
  methods: {
    loadUserInfo() {
      const user = uni.getStorageSync('user') || {};
      this.userInfo = user;
      this.formData = {
        nickname: user.nickname || '',
        gender: user.gender !== undefined ? Number(user.gender) : 0,
        height: user.height?.toString() || '',
        weight: user.weight?.toString() || '',
        birthday: user.birthday || ''
      };
    },

    changeAvatar() {
      uni.chooseImage({
        count: 1,
        sizeType: ['compressed'],
        sourceType: ['album', 'camera'],
        success: (res) => {
          const tempFile = res.tempFilePaths[0];
          this.uploadAvatar(tempFile);
        }
      });
    },

    uploadAvatar(filePath) {
      uni.showLoading({ title: 'Uploading' });
      uni.uploadFile({
        url: getAuthUrl('avatar'),
        filePath: filePath,
        name: 'avatar',
        header: { Authorization: 'Bearer ' + uni.getStorageSync('token') },
        success: (res) => {
          const data = JSON.parse(res.data);
          if (data.errcode === 0) {
            this.userInfo.avatar = data.data.avatar;
            uni.setStorageSync('user', this.userInfo);
            uni.showToast({ title: 'Avatar updated', icon: 'success' });
          } else {
            uni.showToast({ title: data.msg || 'Upload failed', icon: 'none' });
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

    onGenderChange(e) {
      this.formData.gender = Number(e.detail.value);
    },

    onBirthdayChange(e) {
      this.formData.birthday = e.detail.value;
    },

    saveProfile() {
      uni.showLoading({ title: 'Saving...' });
      uni.request({
        url: getAuthUrl('profile'),
        method: 'PUT',
        header: {
          Authorization: 'Bearer ' + uni.getStorageSync('token'),
          'Content-Type': 'application/json'
        },
        data: {
          nickname: this.formData.nickname || null,
          gender: this.formData.gender ?? null,
          birthday: this.formData.birthday || null,
          height: this.formData.height ? parseFloat(this.formData.height) : null,
          weight: this.formData.weight ? parseFloat(this.formData.weight) : null
        },
        success: (res) => {
          if (res.data.errcode === 0) {
            const user = uni.getStorageSync('user') || {};
            Object.assign(user, {
              nickname: this.formData.nickname,
              gender: this.formData.gender,
              birthday: this.formData.birthday,
              height: this.formData.height ? parseFloat(this.formData.height) : null,
              weight: this.formData.weight ? parseFloat(this.formData.weight) : null
            });
            uni.setStorageSync('user', user);
            uni.showToast({ title: 'Saved', icon: 'success' });
            setTimeout(() => uni.navigateBack(), 1500);
          } else {
            uni.showToast({ title: res.data.msg || 'Save failed', icon: 'none' });
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

    goBack() {
      uni.navigateBack();
    }
  }
};
</script>

<style scoped>
.edit-profile-page {
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

.avatar-section {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 30px;
  background: #fff;
  margin-bottom: 10px;
}

.avatar-wrapper {
  position: relative;
  width: 80px;
  height: 80px;
}

.avatar {
  width: 80px;
  height: 80px;
  border-radius: 50%;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: #fff;
  font-size: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.avatar-edit {
  position: absolute;
  bottom: 0;
  right: 0;
  width: 28px;
  height: 28px;
  border-radius: 50%;
  background: #333;
  color: #fff;
  font-size: 14px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.avatar-hint {
  margin-top: 10px;
  font-size: 12px;
  color: #999;
}

.form-container {
  background: #fff;
  padding: 20px;
}

.form-item {
  margin-bottom: 20px;
}

.form-label {
  display: block;
  font-size: 14px;
  color: #666;
  margin-bottom: 8px;
}

.form-input {
  width: 100%;
  height: 44px;
  border: 1px solid #ddd;
  border-radius: 8px;
  padding: 0 15px;
  font-size: 16px;
}

.form-picker {
  display: flex;
  align-items: center;
  justify-content: space-between;
  height: 44px;
  border: 1px solid #ddd;
  border-radius: 8px;
  padding: 0 15px;
  font-size: 16px;
}

.picker-arrow {
  font-size: 20px;
  color: #999;
}

.save-btn {
  width: 100%;
  height: 48px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: #fff;
  border: none;
  border-radius: 24px;
  font-size: 16px;
  margin-top: 20px;
}
</style>