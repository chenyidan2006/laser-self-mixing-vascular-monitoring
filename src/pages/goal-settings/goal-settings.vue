<template>
  <view class="container">
    <view class="header">
      <view class="back-btn" @click="goBack">
        <text class="back-icon">←</text>
      </view>
      <text class="header-title">Daily Goals</text>
      <view class="placeholder"></view>
    </view>

    <view class="content">
      <view class="goal-card">
        <view class="goal-header">
          <text class="goal-icon">💓</text>
          <text class="goal-name">Heart Rate Check</text>
        </view>
        <view class="goal-description">
          <text>How many times do you want to check your heart rate daily?</text>
        </view>
        <view class="goal-input-section">
          <view class="stepper">
            <view class="stepper-btn minus" @click="decreaseHeartRateTarget">
              <text>−</text>
            </view>
            <text class="stepper-value">{{ goalSettings.heartRateTarget }}</text>
            <view class="stepper-btn plus" @click="increaseHeartRateTarget">
              <text>+</text>
            </view>
          </view>
          <text class="goal-unit">times/day</text>
        </view>
      </view>

      <view class="goal-card">
        <view class="goal-header">
          <text class="goal-icon">🚶</text>
          <text class="goal-name">Daily Steps</text>
        </view>
        <view class="goal-description">
          <text>What is your daily step goal?</text>
        </view>
        <view class="goal-input-section">
          <view class="stepper">
            <view class="stepper-btn minus" @click="decreaseStepsTarget">
              <text>−</text>
            </view>
            <text class="stepper-value">{{ goalSettings.stepsTarget }}</text>
            <view class="stepper-btn plus" @click="increaseStepsTarget">
              <text>+</text>
            </view>
          </view>
          <text class="goal-unit">steps</text>
        </view>
        <view class="preset-buttons">
          <view 
            v-for="preset in stepPresets" 
            :key="preset"
            class="preset-btn"
            :class="{ active: goalSettings.stepsTarget === preset }"
            @click="setStepsPreset(preset)"
          >
            {{ preset }}
          </view>
        </view>
      </view>

      <view class="goal-card">
        <view class="goal-header">
          <text class="goal-icon">💧</text>
          <text class="goal-name">Water Intake</text>
        </view>
        <view class="goal-description">
          <text>How many glasses of water do you want to drink daily?</text>
        </view>
        <view class="goal-input-section">
          <view class="stepper">
            <view class="stepper-btn minus" @click="decreaseWaterTarget">
              <text>−</text>
            </view>
            <text class="stepper-value">{{ goalSettings.waterTarget }}</text>
            <view class="stepper-btn plus" @click="increaseWaterTarget">
              <text>+</text>
            </view>
          </view>
          <text class="goal-unit">glasses/day</text>
        </view>
      </view>

      <view class="goal-card">
        <view class="goal-header">
          <text class="goal-icon">😴</text>
          <text class="goal-name">Sleep Hours</text>
        </view>
        <view class="goal-description">
          <text>How many hours of sleep do you aim for each night?</text>
        </view>
        <view class="goal-input-section">
          <view class="stepper">
            <view class="stepper-btn minus" @click="decreaseSleepTarget">
              <text>−</text>
            </view>
            <text class="stepper-value">{{ goalSettings.sleepTarget }}</text>
            <view class="stepper-btn plus" @click="increaseSleepTarget">
              <text>+</text>
            </view>
          </view>
          <text class="goal-unit">hours</text>
        </view>
      </view>
    </view>

    <view class="footer">
      <button class="save-btn" @click="saveSettings">Save Settings</button>
      <button class="reset-btn" @click="resetToDefault">Reset to Default</button>
    </view>
  </view>
</template>

<script>
export default {
  data() {
    return {
      goalSettings: {
        heartRateTarget: 5,
        stepsTarget: 10000,
        waterTarget: 8,
        sleepTarget: 8
      },
      stepPresets: [5000, 8000, 10000, 15000]
    };
  },
  onLoad() {
    this.loadSettings();
  },
  methods: {
    loadSettings() {
      const saved = uni.getStorageSync('goalSettings');
      if (saved) {
        this.goalSettings = JSON.parse(saved);
      }
    },
    saveSettings() {
      uni.setStorageSync('goalSettings', JSON.stringify(this.goalSettings));
      uni.showToast({ title: 'Settings saved!', icon: 'success' });
      setTimeout(() => {
        uni.navigateBack();
      }, 1500);
    },
    resetToDefault() {
      uni.showModal({
        title: 'Reset Settings',
        content: 'Are you sure you want to reset all goals to default?',
        success: (res) => {
          if (res.confirm) {
            this.goalSettings = {
              heartRateTarget: 5,
              stepsTarget: 10000,
              waterTarget: 8,
              sleepTarget: 8
            };
            uni.setStorageSync('goalSettings', JSON.stringify(this.goalSettings));
            uni.showToast({ title: 'Reset to default', icon: 'success' });
          }
        }
      });
    },
    goBack() {
      uni.navigateBack();
    },
    increaseHeartRateTarget() {
      if (this.goalSettings.heartRateTarget < 20) {
        this.goalSettings.heartRateTarget++;
      }
    },
    decreaseHeartRateTarget() {
      if (this.goalSettings.heartRateTarget > 1) {
        this.goalSettings.heartRateTarget--;
      }
    },
    increaseStepsTarget() {
      this.goalSettings.stepsTarget += 500;
    },
    decreaseStepsTarget() {
      if (this.goalSettings.stepsTarget > 1000) {
        this.goalSettings.stepsTarget -= 500;
      }
    },
    setStepsPreset(preset) {
      this.goalSettings.stepsTarget = preset;
    },
    increaseWaterTarget() {
      if (this.goalSettings.waterTarget < 16) {
        this.goalSettings.waterTarget++;
      }
    },
    decreaseWaterTarget() {
      if (this.goalSettings.waterTarget > 1) {
        this.goalSettings.waterTarget--;
      }
    },
    increaseSleepTarget() {
      if (this.goalSettings.sleepTarget < 12) {
        this.goalSettings.sleepTarget++;
      }
    },
    decreaseSleepTarget() {
      if (this.goalSettings.sleepTarget > 4) {
        this.goalSettings.sleepTarget--;
      }
    }
  }
};
</script>

<style lang="scss" scoped>
.container {
  min-height: 100vh;
  background: linear-gradient(180deg, #f0f9ff 0%, #ffffff 100%);
}

.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 60rpx 32rpx 32rpx;
  background: white;
}

.back-btn {
  width: 72rpx;
  height: 72rpx;
  border-radius: 50%;
  background: #f1f5f9;
  display: flex;
  align-items: center;
  justify-content: center;
}

.back-icon {
  font-size: 36rpx;
  color: #334155;
}

.header-title {
  font-size: 36rpx;
  font-weight: 600;
  color: #1e293b;
}

.placeholder {
  width: 72rpx;
}

.content {
  padding: 32rpx;
  display: flex;
  flex-direction: column;
  gap: 24rpx;
}

.goal-card {
  background: white;
  border-radius: 24rpx;
  padding: 32rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.goal-header {
  display: flex;
  align-items: center;
  gap: 16rpx;
  margin-bottom: 16rpx;
}

.goal-icon {
  font-size: 40rpx;
}

.goal-name {
  font-size: 32rpx;
  font-weight: 600;
  color: #1e293b;
}

.goal-description {
  font-size: 26rpx;
  color: #64748b;
  margin-bottom: 24rpx;
  line-height: 1.5;
}

.goal-input-section {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 24rpx;
}

.stepper {
  display: flex;
  align-items: center;
  background: #f8fafc;
  border-radius: 16rpx;
  padding: 8rpx;
}

.stepper-btn {
  width: 80rpx;
  height: 80rpx;
  border-radius: 12rpx;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 40rpx;
  font-weight: 300;
  transition: all 0.2s;
}

.stepper-btn.minus {
  color: #64748b;
}

.stepper-btn.minus:active {
  background: #e2e8f0;
}

.stepper-btn.plus {
  color: #6366f1;
}

.stepper-btn.plus:active {
  background: #e0e7ff;
}

.stepper-value {
  min-width: 120rpx;
  text-align: center;
  font-size: 40rpx;
  font-weight: 700;
  color: #1e293b;
}

.goal-unit {
  font-size: 28rpx;
  color: #94a3b8;
}

.preset-buttons {
  display: flex;
  gap: 16rpx;
  margin-top: 20rpx;
}

.preset-btn {
  flex: 1;
  padding: 16rpx;
  text-align: center;
  background: #f8fafc;
  border-radius: 12rpx;
  font-size: 26rpx;
  color: #64748b;
  transition: all 0.2s;
}

.preset-btn.active {
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
  color: white;
}

.preset-btn:active {
  transform: scale(0.95);
}

.footer {
  padding: 32rpx;
  padding-bottom: 60rpx;
  display: flex;
  flex-direction: column;
  gap: 16rpx;
}

.save-btn {
  width: 100%;
  height: 96rpx;
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
  border-radius: 20rpx;
  border: none;
  color: white;
  font-size: 32rpx;
  font-weight: 600;
  box-shadow: 0 8rpx 24rpx rgba(99, 102, 241, 0.3);
}

.save-btn:active {
  transform: scale(0.98);
}

.reset-btn {
  width: 100%;
  height: 88rpx;
  background: transparent;
  border: 2rpx solid #e2e8f0;
  border-radius: 16rpx;
  color: #64748b;
  font-size: 28rpx;
}

.reset-btn:active {
  background: #f8fafc;
}
</style>
