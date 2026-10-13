<template><view class="page"><view class="nav"><view class="back" @click="goBack">‹ {{ tr('Back') }}</view><text>{{ tr('Personal Info') }}</text><view class="nav-space" /></view>
  <view class="profile-banner"><view class="avatar-wrapper"><image v-if="userInfo.avatar" :src="avatarUrl" class="avatar-image" mode="aspectFill" /><text v-else>{{ formData.nickname?.charAt(0)||'·' }}</text></view><view class="profile-copy"><text>{{ formData.nickname || tr('Your profile') }}</text><text>{{ tr('Make your health records personal') }}</text></view></view>
  <text class="section-title">{{ tr('Basic information') }}</text><view class="card form-card"><view class="form-row"><text>{{ tr('Nickname') }}</text><input v-model="formData.nickname" :maxlength="40" :placeholder="tr('Enter nickname')" /></view><view class="form-row"><text>{{ tr('Gender') }}</text><picker :value="formData.gender" :range="genderOptions" range-key="text" @change="onGenderChange"><view class="picker-value"><text>{{ genderOptions[formData.gender]?.text || tr('Select gender') }}</text><text class="chevron">›</text></view></picker></view><view class="form-row"><text>{{ tr('Birthday') }}</text><picker mode="multiSelector" :value="birthdaySelection" :range="birthdayRange" @columnchange="onBirthdayColumnChange" @change="onBirthdayChange"><view class="picker-value"><text :class="{placeholder:!formData.birthday}">{{ formData.birthday || tr('Select birthday') }}</text><text class="chevron">›</text></view></picker></view></view>
  <view class="body-heading"><text class="section-title">{{ tr('Body measurements') }}</text><text class="linked-chip">{{ tr('Linked defaults') }}</text></view><view class="body-grid"><view class="body-card"><view class="body-icon height-icon">↕</view><text>{{ tr('Height') }}</text><view class="body-value"><input type="digit" v-model="formData.height" :placeholder="tr('Not set')" /><text>cm</text></view></view><view class="body-card"><view class="body-icon weight-icon">⚖</view><text>{{ tr('Weight') }}</text><view class="body-value"><input type="digit" v-model="formData.weight" :placeholder="tr('Not set')" /><text>kg</text></view></view></view>
  <view class="link-note"><text>ⓘ</text><text>{{ tr('Saved height and weight become the defaults in Health Input.') }}</text></view>
  <button class="button save" @click="saveProfile" :disabled="saving" :loading="saving">{{ tr('Save profile') }}</button><text class="save-note">{{ tr('Your changes are saved to your account') }}</text>
</view></template>
<script>
import {backToTab} from '@/utils/navigation.js';
import { getAuthUrl } from '@/config/api.js';

export default {
  data() {
    return {
      saving:false,userInfo: {},birthdaySelection:[100,0,0],
      formData: {
        nickname: '',
        gender: 0,
        height: '',
        weight: '',
        birthday: ''
      },
      genderOptions: [
        { text: this.tr("Secret"), value: 0 },
        { text: this.tr("Male"), value: 1 },
        { text: this.tr("Female"), value: 2 }
      ]
    };
  },
  computed:{avatarUrl(){const value=this.userInfo.avatar;return /^https?:/.test(value)?value:getAuthUrl('profile').replace('/api/user/profile','')+value;},birthdayRange(){const year=1900+this.birthdaySelection[0],month=this.birthdaySelection[1]+1;return [Array.from({length:new Date().getFullYear()-1899},(_,i)=>String(1900+i)),Array.from({length:12},(_,i)=>String(i+1).padStart(2,'0')),Array.from({length:new Date(year,month,0).getDate()},(_,i)=>String(i+1).padStart(2,'0'))];}},
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
      const parts=String(user.birthday||'').slice(0,10).split('-').map(Number);
      if(parts.length===3&&parts[0]>=1900&&parts[0]<=new Date().getFullYear())this.birthdaySelection=[parts[0]-1900,parts[1]-1,parts[2]-1];
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
      uni.showLoading({ title: this.tr("Uploading") });
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
            uni.showToast({ title: this.tr("Avatar updated"), icon: 'success' });
          } else {
            uni.showToast({ title: data.msg || this.tr("Upload failed"), icon: 'none' });
          }
        },
        fail: () => {
          uni.showToast({ title: this.tr("Network error"), icon: 'none' });
        },
        complete: () => {
          this.saving=false;uni.hideLoading();
        }
      });
    },

    onGenderChange(e) {
      this.formData.gender = Number(e.detail.value);
    },

    onBirthdayChange(e) {
      const values=e.detail.value;this.birthdaySelection=values;
      this.formData.birthday=`${1900+values[0]}-${String(values[1]+1).padStart(2,'0')}-${String(values[2]+1).padStart(2,'0')}`;
    },
    onBirthdayColumnChange(e){const values=[...this.birthdaySelection];values[e.detail.column]=e.detail.value;const days=new Date(1900+values[0],values[1]+1,0).getDate();values[2]=Math.min(values[2],days-1);this.birthdaySelection=values;},

    saveProfile() {
      if(this.saving)return;this.saving=true;
      uni.showLoading({ title: this.tr("Saving...") });
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
            uni.showToast({ title: this.tr("Saved"), icon: 'success' });
            setTimeout(() => this.goBack(), 700);
          } else {
            uni.showToast({ title: res.data.msg || this.tr("Save failed"), icon: 'none' });
          }
        },
        fail: () => {
          uni.showToast({ title: this.tr("Network error"), icon: 'none' });
        },
        complete: () => {
          this.saving=false;uni.hideLoading();
        }
      });
    },

    goBack() {
      backToTab('profile');
    }
  }
};
</script>
<style scoped>.page{padding:28rpx 28rpx 60rpx;background:#f5f6fb;min-height:100vh;color:#29344d;box-sizing:border-box}.nav{display:flex;align-items:center;justify-content:space-between;padding:14rpx 0 32rpx;font-size:26rpx}.nav>text{font-size:32rpx;font-weight:700}.back{padding:18rpx 18rpx 18rpx 0;min-width:100rpx;color:#6b7790}.nav-space{width:118rpx}.hero{background:linear-gradient(125deg,#7565d9,#987de8);border-radius:30rpx;padding:34rpx;color:white;margin-bottom:34rpx}.hero-title{display:block;font-size:38rpx;font-weight:700;line-height:1.4}.hero-sub{display:block;font-size:24rpx;opacity:.8;line-height:1.7;margin-top:14rpx}.section-title{display:block;font-size:30rpx;font-weight:700;margin:32rpx 6rpx 22rpx}.card{background:white;border-radius:26rpx;padding:28rpx;margin-bottom:24rpx}.muted{font-size:23rpx;color:#99a2b4;line-height:1.7}.button{background:#8370dc;color:white;padding:22rpx;border-radius:19rpx;font-size:27rpx}.button:after{border:0}
.profile-banner{display:flex;align-items:center;gap:28rpx;background:linear-gradient(120deg,#ece7fa,#f2eefb);border-radius:30rpx;padding:36rpx 30rpx;margin-bottom:32rpx}.avatar-wrapper{position:relative;width:115rpx;height:115rpx;border-radius:35rpx;background:linear-gradient(130deg,#8770dc,#a38ae7);color:white;font-size:50rpx;line-height:115rpx;text-align:center;flex-shrink:0}.avatar-image{width:100%;height:100%;border-radius:35rpx}.avatar-edit{position:absolute;right:-8rpx;bottom:-8rpx;width:38rpx;height:38rpx;line-height:38rpx;border-radius:14rpx;color:#9a83d0;background:white;font-size:24rpx;box-shadow:0 3rpx 9rpx #8d7aaa14}.profile-copy{min-width:0}.profile-copy>text{display:block}.profile-copy>text:first-child{font-size:34rpx;font-weight:600;color:#6d5c9f;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.profile-copy>text:nth-child(2){font-size:22rpx;color:#ae9ec7;margin-top:10rpx;line-height:1.5}.profile-copy>view{display:inline-block;font-size:23rpx;color:#9a83cd;margin-top:17rpx;padding:6rpx 0}.form-card{padding:0 28rpx}.form-row{display:flex;align-items:center;justify-content:space-between;gap:24rpx;min-height:108rpx;border-bottom:1px solid #f0f2f8;font-size:26rpx}.form-row:last-child{border:0}.form-row>text{color:#929bae;flex-shrink:0}.form-row input{flex:1;min-width:0;text-align:right;font-size:26rpx;color:#55627c;height:70rpx}.form-row picker{flex:1;min-width:0}.picker-value{display:flex;justify-content:flex-end;align-items:center;gap:18rpx;color:#55627c;min-height:90rpx}.chevron{font-size:34rpx;color:#c2c7d2}.placeholder{color:#c2c7d2}.body-heading{display:flex;align-items:center;justify-content:space-between}.linked-chip{font-size:21rpx;background:#ede8fa;color:#a08acf;padding:8rpx 15rpx;border-radius:13rpx}.body-grid{display:grid;grid-template-columns:1fr 1fr;gap:22rpx}.body-card{background:white;border-radius:25rpx;padding:28rpx;box-sizing:border-box;min-width:0}.body-icon{width:55rpx;height:55rpx;line-height:55rpx;text-align:center;border-radius:18rpx;font-size:33rpx;margin-bottom:18rpx}.height-icon{background:#edf3fa;color:#92aed2}.weight-icon{background:#faf0ec;color:#d6ad94}.body-card>text{font-size:25rpx;color:#99a3b5}.body-value{display:flex;align-items:baseline;margin-top:19rpx;gap:6rpx}.body-value input{min-width:0;width:100%;flex:1;color:#5a6782;font-size:43rpx;font-weight:600;height:64rpx}.body-value>text{font-size:23rpx;color:#b1b8c7}.link-note{display:flex;align-items:flex-start;gap:12rpx;margin:24rpx 8rpx 42rpx;color:#a9b1c0;font-size:22rpx;line-height:1.7}.link-note>text:first-child{color:#b7a6d9;font-size:28rpx}.save-note{display:block;text-align:center;color:#b3bac7;font-size:22rpx;margin-top:20rpx}
</style>
