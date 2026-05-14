<template>
  <view class="help-container">
    <view class="nav-bar">
      <view class="nav-back" @click="goBack">
        <text class="back-icon">‹</text>
        <text class="back-text">Back</text>
      </view>
      <text class="nav-title">Help & Support</text>
      <view class="nav-placeholder"></view>
    </view>

    <view class="hero-section">
      <view class="hero-icon">💬</view>
      <text class="hero-title">How can we help you?</text>
      <text class="hero-desc">Find answers to common questions or contact our support team</text>
    </view>

    <view class="quick-actions">
      <view class="action-card" @click="showFeedbackModal = true">
        <view class="action-icon-wrapper feedback">
          <text class="action-icon">✍️</text>
        </view>
        <text class="action-title">Feedback</text>
        <text class="action-desc">Share your thoughts</text>
      </view>
      <view class="action-card" @click="goToFAQ">
        <view class="action-icon-wrapper faq">
          <text class="action-icon">❓</text>
        </view>
        <text class="action-title">FAQ</text>
        <text class="action-desc">Common questions</text>
      </view>
      <view class="action-card" @click="goToContact">
        <view class="action-icon-wrapper contact">
          <text class="action-icon">📞</text>
        </view>
        <text class="action-title">Contact</text>
        <text class="action-desc">Get in touch</text>
      </view>
    </view>

    <view class="faq-section" id="faq">
      <view class="section-header">
        <text class="section-title">Frequently Asked Questions</text>
      </view>
      
      <view class="faq-list">
        <view 
          v-for="(item, index) in faqs" 
          :key="index" 
          class="faq-item"
          :class="{ expanded: expandedIndex === index }"
          @click="toggleFaq(index)"
        >
          <view class="faq-header">
            <text class="faq-icon">{{ item.icon }}</text>
            <text class="faq-question">{{ item.question }}</text>
            <text class="faq-arrow">{{ expandedIndex === index ? '−' : '+' }}</text>
          </view>
          <view class="faq-answer" v-show="expandedIndex === index">
            <text>{{ item.answer }}</text>
          </view>
        </view>
      </view>
    </view>

    <view class="contact-section" id="contact">
      <view class="section-header">
        <text class="section-title">Contact Support</text>
      </view>
      
      <view class="contact-options">
        <view class="contact-option" @click="sendEmail">
          <view class="contact-icon">📧</view>
          <view class="contact-info">
            <text class="contact-title">Email</text>
            <text class="contact-value">support@healthmonitor.com</text>
          </view>
        </view>
        <view class="contact-option" @click="makeCall">
          <view class="contact-icon">📱</view>
          <view class="contact-info">
            <text class="contact-title">Phone</text>
            <text class="contact-value">+1 (800) 123-4567</text>
          </view>
        </view>
        <view class="contact-option" @click="openChat">
          <view class="contact-icon">💬</view>
          <view class="contact-info">
            <text class="contact-title">Live Chat</text>
            <text class="contact-value">Available 9am-5pm</text>
          </view>
        </view>
      </view>
    </view>

    <view class="support-info">
      <view class="info-card">
        <text class="info-title">🕐 Support Hours</text>
        <text class="info-content">Monday - Friday: 9:00 AM - 5:00 PM (EST)</text>
        <text class="info-content">Saturday - Sunday: 10:00 AM - 4:00 PM (EST)</text>
      </view>
      <view class="info-card">
        <text class="info-title">📍 Office</text>
        <text class="info-content">123 Health Street</text>
        <text class="info-content">New York, NY 10001</text>
      </view>
    </view>

    <view class="feedback-modal" v-if="showFeedbackModal" @click="showFeedbackModal = false">
      <view class="modal-content" @click.stop>
        <view class="modal-header">
          <text class="modal-title">Send Feedback</text>
          <text class="modal-close" @click="showFeedbackModal = false">×</text>
        </view>
        
        <view class="modal-body">
          <view class="feedback-type">
            <text class="type-label">Feedback Type</text>
            <view class="type-options">
              <view 
                v-for="type in feedbackTypes" 
                :key="type.value"
                class="type-option"
                :class="{ active: feedbackType === type.value }"
                @click="feedbackType = type.value"
              >
                <text class="type-icon">{{ type.icon }}</text>
                <text class="type-name">{{ type.label }}</text>
              </view>
            </view>
          </view>
          
          <view class="feedback-form">
            <text class="form-label">Subject</text>
            <input 
              v-model="feedbackSubject" 
              class="form-input" 
              placeholder="Enter subject"
            />
            
            <text class="form-label">Message</text>
            <textarea 
              v-model="feedbackMessage" 
              class="form-textarea" 
              placeholder="Describe your issue or suggestion..."
            />
            
            <text class="form-label">Email (optional)</text>
            <input 
              v-model="feedbackEmail" 
              class="form-input" 
              placeholder="your@email.com"
            />
          </view>
        </view>
        
        <view class="modal-footer">
          <view class="btn btn-secondary" @click="showFeedbackModal = false">Cancel</view>
          <view class="btn btn-primary" @click="submitFeedback">Submit</view>
        </view>
      </view>
    </view>
  </view>
</template>

<script>
export default {
  data() {
    return {
      expandedIndex: -1,
      showFeedbackModal: false,
      feedbackType: 'suggestion',
      feedbackSubject: '',
      feedbackMessage: '',
      feedbackEmail: '',
      feedbackTypes: [
        { value: 'bug', label: 'Bug Report', icon: '🐛' },
        { value: 'suggestion', label: 'Suggestion', icon: '💡' },
        { value: 'question', label: 'Question', icon: '❓' },
        { value: 'complaint', label: 'Complaint', icon: '😕' }
      ],
      faqs: [
        {
          icon: '🔗',
          question: 'How do I connect my device?',
          answer: 'To connect your health monitor device, go to the Devices page, tap "+ Add Device", and follow the on-screen instructions. Make sure your device is in pairing mode and Bluetooth is enabled on your phone.'
        },
        {
          icon: '📊',
          question: 'Why am I not seeing real-time data?',
          answer: 'Please ensure your device is properly connected and within Bluetooth range. If the issue persists, try restarting both your device and the app. You may also need to check if your device has sufficient battery.'
        },
        {
          icon: '💾',
          question: 'Where is my data stored?',
          answer: 'Your health data is securely stored in our encrypted cloud servers. You can also export your data at any time from the Profile page. We never share your data with third parties without your consent.'
        },
        {
          icon: '🔄',
          question: 'How do I sync data across devices?',
          answer: 'Data syncs automatically when you sign in with the same account on multiple devices. Make sure you have an active internet connection for sync to work properly.'
        },
        {
          icon: '⚡',
          question: 'How long does the battery last?',
          answer: 'The device battery typically lasts 7-10 days with regular use. Battery life may vary depending on usage frequency and measurement settings.'
        },
        {
          icon: '🌡️',
          question: 'Are the measurements accurate?',
          answer: 'Our devices are calibrated to provide accurate readings, but they are for reference only. For medical concerns, please consult a healthcare professional.'
        }
      ]
    };
  },
  methods: {
    goBack() {
      uni.navigateBack();
    },
    toggleFaq(index) {
      this.expandedIndex = this.expandedIndex === index ? -1 : index;
    },
    goToFAQ() {
      const faqSection = document.getElementById('faq');
      if (faqSection) {
        faqSection.scrollIntoView({ behavior: 'smooth' });
      }
    },
    goToContact() {
      const contactSection = document.getElementById('contact');
      if (contactSection) {
        contactSection.scrollIntoView({ behavior: 'smooth' });
      }
    },
    sendEmail() {
      uni.showToast({ title: 'Opening email...', icon: 'none' });
    },
    makeCall() {
      uni.showToast({ title: 'Dialing...', icon: 'none' });
    },
    openChat() {
      uni.showToast({ title: 'Opening chat...', icon: 'none' });
    },
    submitFeedback() {
      if (!this.feedbackMessage.trim()) {
        uni.showToast({ title: 'Please enter a message', icon: 'none' });
        return;
      }
      
      uni.showLoading({ title: 'Sending...' });
      
      setTimeout(() => {
        uni.hideLoading();
        uni.showToast({ title: 'Feedback submitted', icon: 'success' });
        this.showFeedbackModal = false;
        this.feedbackSubject = '';
        this.feedbackMessage = '';
        this.feedbackEmail = '';
        this.feedbackType = 'suggestion';
      }, 1000);
    }
  }
};
</script>

<style lang="scss" scoped>
.help-container {
  min-height: 100vh;
  background: linear-gradient(180deg, #f0fdf4 0%, #f8fafc 100%);
  padding-bottom: 40rpx;
}

.nav-bar {
  background: linear-gradient(135deg, #22c55e 0%, #16a34a 50%, #15803d 100%);
  padding: 60rpx 20rpx 20rpx;
  display: flex;
  align-items: center;
  justify-content: space-between;
  box-shadow: 0 8rpx 32rpx rgba(34, 197, 94, 0.3);
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

.hero-section {
  text-align: center;
  padding: 60rpx 40rpx;
}

.hero-icon {
  font-size: 80rpx;
  margin-bottom: 20rpx;
}

.hero-title {
  display: block;
  font-size: 40rpx;
  font-weight: 700;
  color: #0f172a;
  margin-bottom: 16rpx;
}

.hero-desc {
  display: block;
  font-size: 28rpx;
  color: #64748b;
}

.quick-actions {
  display: flex;
  gap: 20rpx;
  padding: 0 20rpx;
  margin-bottom: 40rpx;
}

.action-card {
  flex: 1;
  background: white;
  border-radius: 24rpx;
  padding: 30rpx 20rpx;
  text-align: center;
  box-shadow: 0 8rpx 24rpx rgba(0, 0, 0, 0.06);
}

.action-icon-wrapper {
  width: 80rpx;
  height: 80rpx;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  margin: 0 auto 16rpx;
}

.action-icon-wrapper.feedback {
  background: linear-gradient(135deg, #fbbf24, #f59e0b);
}

.action-icon-wrapper.faq {
  background: linear-gradient(135deg, #3b82f6, #2563eb);
}

.action-icon-wrapper.contact {
  background: linear-gradient(135deg, #ec4899, #db2777);
}

.action-icon {
  font-size: 36rpx;
}

.action-title {
  display: block;
  font-size: 28rpx;
  font-weight: 600;
  color: #0f172a;
  margin-bottom: 8rpx;
}

.action-desc {
  display: block;
  font-size: 22rpx;
  color: #64748b;
}

.faq-section, .contact-section {
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

.faq-list {
  display: flex;
  flex-direction: column;
  gap: 16rpx;
}

.faq-item {
  border-radius: 20rpx;
  overflow: hidden;
  background: #f8fafc;
}

.faq-header {
  display: flex;
  align-items: center;
  gap: 16rpx;
  padding: 24rpx;
  transition: all 0.2s;
}

.faq-item.expanded .faq-header {
  background: #f0fdf4;
}

.faq-icon {
  font-size: 28rpx;
}

.faq-question {
  flex: 1;
  font-size: 28rpx;
  font-weight: 600;
  color: #0f172a;
}

.faq-arrow {
  font-size: 36rpx;
  font-weight: 300;
  color: #64748b;
}

.faq-answer {
  padding: 0 24rpx 24rpx;
  font-size: 26rpx;
  color: #475569;
  line-height: 1.6;
}

.contact-options {
  display: flex;
  flex-direction: column;
  gap: 16rpx;
}

.contact-option {
  display: flex;
  align-items: center;
  gap: 20rpx;
  padding: 24rpx;
  border-radius: 20rpx;
  background: #f8fafc;
  transition: all 0.2s;
}

.contact-option:active {
  background: #f0fdf4;
}

.contact-icon {
  font-size: 40rpx;
}

.contact-info {
  flex: 1;
}

.contact-title {
  display: block;
  font-size: 24rpx;
  color: #64748b;
  margin-bottom: 4rpx;
}

.contact-value {
  font-size: 28rpx;
  font-weight: 600;
  color: #0f172a;
}

.support-info {
  display: flex;
  gap: 20rpx;
  padding: 0 20rpx;
}

.info-card {
  flex: 1;
  background: white;
  border-radius: 24rpx;
  padding: 24rpx;
  box-shadow: 0 8rpx 24rpx rgba(0, 0, 0, 0.06);
}

.info-title {
  display: block;
  font-size: 26rpx;
  font-weight: 600;
  color: #0f172a;
  margin-bottom: 12rpx;
}

.info-content {
  display: block;
  font-size: 22rpx;
  color: #64748b;
  line-height: 1.6;
}

.feedback-modal {
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
  padding: 32rpx;
  padding-bottom: calc(32rpx + env(safe-area-inset-bottom));
  max-height: 80vh;
  overflow-y: auto;
}

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 32rpx;
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

.feedback-type {
  margin-bottom: 32rpx;
}

.type-label {
  display: block;
  font-size: 26rpx;
  font-weight: 600;
  color: #0f172a;
  margin-bottom: 16rpx;
}

.type-options {
  display: flex;
  gap: 12rpx;
}

.type-option {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8rpx;
  padding: 20rpx 12rpx;
  border-radius: 16rpx;
  background: #f8fafc;
  border: 2rpx solid transparent;
  transition: all 0.2s;
}

.type-option.active {
  background: #f0fdf4;
  border-color: #22c55e;
}

.type-icon {
  font-size: 28rpx;
}

.type-name {
  font-size: 22rpx;
  font-weight: 500;
  color: #0f172a;
}

.feedback-form {
  display: flex;
  flex-direction: column;
  gap: 20rpx;
}

.form-label {
  display: block;
  font-size: 26rpx;
  font-weight: 600;
  color: #0f172a;
  margin-bottom: 12rpx;
}

.form-input {
  width: 100%;
  height: 80rpx;
  border-radius: 16rpx;
  background: #f8fafc;
  padding: 0 20rpx;
  font-size: 28rpx;
  box-sizing: border-box;
}

.form-textarea {
  width: 100%;
  height: 200rpx;
  border-radius: 16rpx;
  background: #f8fafc;
  padding: 20rpx;
  font-size: 28rpx;
  box-sizing: border-box;
}

.modal-footer {
  display: flex;
  gap: 20rpx;
  margin-top: 32rpx;
}

.btn {
  flex: 1;
  height: 88rpx;
  border-radius: 44rpx;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 30rpx;
  font-weight: 600;
}

.btn-secondary {
  background: #f1f5f9;
  color: #64748b;
}

.btn-primary {
  background: linear-gradient(135deg, #22c55e, #16a34a);
  color: white;
}
</style>