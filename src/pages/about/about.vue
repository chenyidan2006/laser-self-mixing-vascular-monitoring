<template>
  <view class="about-container">
    <view class="nav-bar">
      <view class="nav-back" @click="goBack">
        <text class="back-icon">‹</text>
        <text class="back-text">Back</text>
      </view>
      <text class="nav-title">About Us</text>
      <view class="nav-placeholder"></view>
    </view>

    <view class="brand-section">
      <view class="brand-icon">❤️</view>
      <text class="brand-name">Health Monitor</text>
      <text class="brand-tagline">Your Personal Health Companion</text>
      <view class="version-badge">
        <text class="version-text">Version {{ appVersion }}</text>
      </view>
    </view>

    <view class="features-section">
      <view class="section-header">
        <text class="section-title">Features</text>
      </view>
      
      <view class="features-grid">
        <view class="feature-card">
          <view class="feature-icon heart">❤️</view>
          <text class="feature-title">Heart Rate</text>
          <text class="feature-desc">Real-time heart rate monitoring</text>
        </view>
        <view class="feature-card">
          <view class="feature-icon bp">🩸</view>
          <text class="feature-title">Blood Pressure</text>
          <text class="feature-desc">Accurate BP measurement</text>
        </view>
        <view class="feature-card">
          <view class="feature-icon spo2">🫁</view>
          <text class="feature-title">SpO2</text>
          <text class="feature-desc">Blood oxygen level tracking</text>
        </view>
        <view class="feature-card">
          <view class="feature-icon trend">📈</view>
          <text class="feature-title">Trends</text>
          <text class="feature-desc">Health data analysis</text>
        </view>
        <view class="feature-card">
          <view class="feature-icon cloud">☁️</view>
          <text class="feature-title">Cloud Sync</text>
          <text class="feature-desc">Secure data backup</text>
        </view>
        <view class="feature-card">
          <view class="feature-icon share">🔗</view>
          <text class="feature-title">Share</text>
          <text class="feature-desc">Share with healthcare providers</text>
        </view>
      </view>
    </view>

    <view class="about-section">
      <view class="section-header">
        <text class="section-title">About Our App</text>
      </view>
      
      <view class="about-content">
        <text class="about-text">
          Health Monitor is dedicated to providing you with accurate, reliable health monitoring tools. Our mission is to empower individuals to take control of their health through technology.
        </text>
        <text class="about-text">
          With our advanced sensor technology and intelligent algorithms, we aim to make health monitoring accessible and convenient for everyone. Whether you're tracking daily vitals or managing chronic conditions, Health Monitor is here to help.
        </text>
      </view>
    </view>

    <view class="team-section">
      <view class="section-header">
        <text class="section-title">Our Team</text>
      </view>
      
      <view class="team-grid">
        <view class="team-card">
          <view class="team-avatar">👨‍💻</view>
          <text class="team-name">John Smith</text>
          <text class="team-role">CEO & Founder</text>
        </view>
        <view class="team-card">
          <view class="team-avatar">👩‍⚕️</view>
          <text class="team-name">Emily Davis</text>
          <text class="team-role">Medical Director</text>
        </view>
        <view class="team-card">
          <view class="team-avatar">👨‍🔬</view>
          <text class="team-name">Michael Chen</text>
          <text class="team-role">Lead Engineer</text>
        </view>
        <view class="team-card">
          <view class="team-avatar">👩‍🎨</view>
          <text class="team-name">Sarah Wilson</text>
          <text class="team-role">Design Lead</text>
        </view>
      </view>
    </view>

    <view class="quick-links">
      <view class="link-item" @click="showPrivacyModal = true">
        <text class="link-icon">🔒</text>
        <text class="link-text">Privacy Policy</text>
        <text class="link-arrow">›</text>
      </view>
      <view class="link-item" @click="showTermsModal = true">
        <text class="link-icon">📜</text>
        <text class="link-text">Terms of Service</text>
        <text class="link-arrow">›</text>
      </view>
      <view class="link-item" @click="checkUpdates">
        <text class="link-icon">🔄</text>
        <text class="link-text">Check for Updates</text>
        <text class="link-arrow">›</text>
      </view>
      <view class="link-item" @click="goToHelp">
        <text class="link-icon">❓</text>
        <text class="link-text">Help Center</text>
        <text class="link-arrow">›</text>
      </view>
    </view>

    <view class="footer">
      <text class="footer-text">© 2024 Health Monitor. All rights reserved.</text>
      <text class="footer-tag">Designed with ❤️ for better health</text>
    </view>

    <view class="privacy-modal" v-if="showPrivacyModal" @click="showPrivacyModal = false">
      <view class="modal-content" @click.stop>
        <view class="modal-header">
          <text class="modal-title">Privacy Policy</text>
          <text class="modal-close" @click="showPrivacyModal = false">×</text>
        </view>
        <scroll-view scroll-y class="modal-body">
          <text class="policy-section">1. Data Collection</text>
          <text class="policy-text">We collect health data including heart rate, blood pressure, and SpO2 levels to provide our monitoring services.</text>
          <text class="policy-section">2. Data Storage</text>
          <text class="policy-text">Your data is securely stored in encrypted cloud servers. We never share your personal health information with third parties.</text>
          <text class="policy-section">3. User Rights</text>
          <text class="policy-text">You have the right to access, modify, or delete your personal data at any time through the app settings.</text>
          <text class="policy-section">4. Security</text>
          <text class="policy-text">We implement industry-standard security measures to protect your data from unauthorized access.</text>
        </scroll-view>
        <view class="modal-footer">
          <view class="btn btn-primary" @click="showPrivacyModal = false">OK</view>
        </view>
      </view>
    </view>

    <view class="terms-modal" v-if="showTermsModal" @click="showTermsModal = false">
      <view class="modal-content" @click.stop>
        <view class="modal-header">
          <text class="modal-title">Terms of Service</text>
          <text class="modal-close" @click="showTermsModal = false">×</text>
        </view>
        <scroll-view scroll-y class="modal-body">
          <text class="policy-section">1. Acceptance</text>
          <text class="policy-text">By using this app, you agree to be bound by these terms of service.</text>
          <text class="policy-section">2. Use</text>
          <text class="policy-text">This app is for personal health monitoring only. It is not a substitute for professional medical advice.</text>
          <text class="policy-section">3. Accuracy</text>
          <text class="policy-text">While we strive for accuracy, measurements are for reference only and should not be used for medical diagnosis.</text>
          <text class="policy-section">4. Liability</text>
          <text class="policy-text">We are not liable for any damages arising from the use of this app or its data.</text>
        </scroll-view>
        <view class="modal-footer">
          <view class="btn btn-primary" @click="showTermsModal = false">OK</view>
        </view>
      </view>
    </view>
  </view>
</template>

<script>
export default {
  data() {
    return {
      appVersion: '1.0.0',
      showPrivacyModal: false,
      showTermsModal: false
    };
  },
  methods: {
    goBack() {
      uni.navigateBack();
    },
    checkUpdates() {
      uni.showLoading({ title: 'Checking...' });
      setTimeout(() => {
        uni.hideLoading();
        uni.showToast({ title: 'You have the latest version', icon: 'success' });
      }, 1000);
    },
    goToHelp() {
      uni.navigateTo({ url: '/pages/help/help' });
    }
  }
};
</script>

<style lang="scss" scoped>
.about-container {
  min-height: 100vh;
  background: linear-gradient(180deg, #fef3c7 0%, #f8fafc 50%, #ffffff 100%);
  padding-bottom: 40rpx;
}

.nav-bar {
  background: linear-gradient(135deg, #f59e0b 0%, #d97706 50%, #b45309 100%);
  padding: 60rpx 20rpx 20rpx;
  display: flex;
  align-items: center;
  justify-content: space-between;
  box-shadow: 0 8rpx 32rpx rgba(245, 158, 11, 0.3);
}

.nav-back {
  display: flex;
  align-items: center;
  gap: 10rpx;
  color: white;
}

.back-icon {
  font-size: 44rpx;
  font-weight: bold;
}

.back-text {
  font-size: 28rpx;
}

.nav-title {
  font-size: 36rpx;
  font-weight: 700;
  color: white;
}

.nav-placeholder {
  width: 80rpx;
}

.brand-section {
  text-align: center;
  padding: 80rpx 40rpx 60rpx;
}

.brand-icon {
  font-size: 120rpx;
  margin-bottom: 24rpx;
}

.brand-name {
  display: block;
  font-size: 52rpx;
  font-weight: 800;
  color: #0f172a;
  margin-bottom: 12rpx;
}

.brand-tagline {
  display: block;
  font-size: 28rpx;
  color: #64748b;
  margin-bottom: 24rpx;
}

.version-badge {
  display: inline-block;
  padding: 12rpx 28rpx;
  border-radius: 40rpx;
  background: rgba(0, 0, 0, 0.05);
}

.version-text {
  font-size: 24rpx;
  color: #64748b;
}

.features-section, .about-section, .team-section {
  margin: 0 20rpx 30rpx;
  background: white;
  border-radius: 28rpx;
  padding: 30rpx;
  box-shadow: 0 8rpx 24rpx rgba(0, 0, 0, 0.06);
}

.section-header {
  margin-bottom: 24rpx;
}

.section-title {
  font-size: 32rpx;
  font-weight: 700;
  color: #0f172a;
}

.features-grid {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr;
  gap: 20rpx;
}

.feature-card {
  text-align: center;
  padding: 24rpx 12rpx;
  border-radius: 20rpx;
  background: #f8fafc;
}

.feature-icon {
  font-size: 48rpx;
  margin-bottom: 12rpx;
}

.feature-title {
  display: block;
  font-size: 26rpx;
  font-weight: 600;
  color: #0f172a;
  margin-bottom: 8rpx;
}

.feature-desc {
  display: block;
  font-size: 20rpx;
  color: #64748b;
}

.about-content {
  display: flex;
  flex-direction: column;
  gap: 20rpx;
}

.about-text {
  font-size: 28rpx;
  color: #475569;
  line-height: 1.7;
}

.team-grid {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr 1fr;
  gap: 16rpx;
}

.team-card {
  text-align: center;
}

.team-avatar {
  width: 80rpx;
  height: 80rpx;
  border-radius: 50%;
  background: linear-gradient(135deg, #6366f1, #8b5cf6);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 36rpx;
  margin: 0 auto 12rpx;
}

.team-name {
  display: block;
  font-size: 24rpx;
  font-weight: 600;
  color: #0f172a;
  margin-bottom: 4rpx;
}

.team-role {
  display: block;
  font-size: 20rpx;
  color: #64748b;
}

.quick-links {
  margin: 0 20rpx 30rpx;
  background: white;
  border-radius: 28rpx;
  overflow: hidden;
  box-shadow: 0 8rpx 24rpx rgba(0, 0, 0, 0.06);
}

.link-item {
  display: flex;
  align-items: center;
  gap: 16rpx;
  padding: 28rpx 30rpx;
  border-bottom: 1rpx solid #f1f5f9;
}

.link-item:last-child {
  border-bottom: none;
}

.link-icon {
  font-size: 32rpx;
}

.link-text {
  flex: 1;
  font-size: 28rpx;
  color: #0f172a;
}

.link-arrow {
  font-size: 36rpx;
  color: #94a3b8;
}

.footer {
  text-align: center;
  padding: 40rpx;
}

.footer-text {
  display: block;
  font-size: 24rpx;
  color: #94a3b8;
  margin-bottom: 8rpx;
}

.footer-tag {
  display: block;
  font-size: 22rpx;
  color: #cbd5e1;
}

.privacy-modal, .terms-modal {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: flex-end;
  z-index: 100;
}

.modal-content {
  width: 100%;
  background: white;
  border-radius: 32rpx 32rpx 0 0;
  max-height: 70vh;
  overflow: hidden;
}

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 32rpx;
  border-bottom: 1rpx solid #f1f5f9;
}

.modal-title {
  font-size: 36rpx;
  font-weight: 700;
  color: #0f172a;
}

.modal-close {
  font-size: 48rpx;
  color: #94a3b8;
  padding: 0 16rpx;
}

.modal-body {
  padding: 32rpx;
  max-height: 50vh;
}

.policy-section {
  display: block;
  font-size: 28rpx;
  font-weight: 600;
  color: #0f172a;
  margin-top: 20rpx;
  margin-bottom: 12rpx;
}

.policy-section:first-child {
  margin-top: 0;
}

.policy-text {
  display: block;
  font-size: 26rpx;
  color: #475569;
  line-height: 1.6;
}

.modal-footer {
  padding: 24rpx 32rpx;
  padding-bottom: calc(24rpx + env(safe-area-inset-bottom));
  border-top: 1rpx solid #f1f5f9;
}

.btn {
  width: 100%;
  height: 88rpx;
  border-radius: 44rpx;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 30rpx;
  font-weight: 600;
}

.btn-primary {
  background: linear-gradient(135deg, #f59e0b, #d97706);
  color: white;
}
</style>