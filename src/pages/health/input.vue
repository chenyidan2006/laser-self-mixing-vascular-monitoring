<template>
  <view class="health-input-container">
    <view class="nav-bar">
      <view class="nav-back" @click="goBack">
        <text class="back-icon">‹</text>
        <text class="back-text">Back</text>
      </view>
      <text class="nav-title">Health Input</text>
      <view class="nav-placeholder"></view>
    </view>

    <scroll-view scroll-y class="content-scroll">
      <view class="section">
        <view class="section-title">
          <text class="title-icon">💧</text>
          <text>Hydration</text>
        </view>
        <view class="input-card">
          <view class="input-row">
            <text class="input-label">Water Intake (ml)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('water', 100)">-</view>
              <text class="stepper-value">{{ userInputs.water }}</text>
              <view class="stepper-btn plus" @click="increase('water', 100)">+</view>
            </view>
          </view>
          <view class="quick-btns">
            <view class="quick-btn" @click="setQuick('water', 250)">250ml</view>
            <view class="quick-btn" @click="setQuick('water', 500)">500ml</view>
            <view class="quick-btn" @click="setQuick('water', 1000)">1L</view>
            <view class="quick-btn" @click="setQuick('water', 1500)">1.5L</view>
          </view>
        </view>
      </view>

      <view class="section">
        <view class="section-title">
          <text class="title-icon">☕</text>
          <text>Caffeine Intake</text>
        </view>
        <view class="input-card">
          <view class="input-row">
            <text class="input-label">Coffee (cups)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('coffee', 1)">-</view>
              <text class="stepper-value">{{ userInputs.coffee }}</text>
              <view class="stepper-btn plus" @click="increase('coffee', 1)">+</view>
            </view>
          </view>
          <view class="caffeine-info">
            <text class="caffeine-note">☕ ~95mg per cup | 🥤 Soda ~40mg | 🧃 Tea ~30mg</text>
          </view>
          <view class="input-row">
            <text class="input-label">Soda (cups)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('soda', 1)">-</view>
              <text class="stepper-value">{{ userInputs.soda }}</text>
              <view class="stepper-btn plus" @click="increase('soda', 1)">+</view>
            </view>
          </view>
          <view class="input-row">
            <text class="input-label">Tea (cups)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('tea', 1)">-</view>
              <text class="stepper-value">{{ userInputs.tea }}</text>
              <view class="stepper-btn plus" @click="increase('tea', 1)">+</view>
            </view>
          </view>
          <view class="total-caffeine">
            <text>Total Caffeine: ~{{ totalCaffeine }}mg</text>
            <text class="caffeine-level" :class="caffeineLevelClass">{{ caffeineLevel }}</text>
          </view>
        </view>
      </view>

      <view class="section">
        <view class="section-title">
          <text class="title-icon">😴</text>
          <text>Sleep</text>
        </view>
        <view class="input-card">
          <view class="input-row">
            <text class="input-label">Sleep Duration (hours)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decreaseSleep">-</view>
              <text class="stepper-value">{{ userInputs.sleepHours }}h {{ userInputs.sleepMinutes }}m</text>
              <view class="stepper-btn plus" @click="increaseSleep">+</view>
            </view>
          </view>
          <view class="sleep-quality">
            <text>Sleep Quality:</text>
            <view class="quality-btns">
              <view
                v-for="q in ['Poor', 'Fair', 'Good', 'Excellent']"
                :key="q"
                class="quality-btn"
                :class="{ active: userInputs.sleepQuality === q }"
                @click="userInputs.sleepQuality = q"
              >{{ q }}</view>
            </view>
          </view>
          <view class="sleep-status" :class="sleepStatusClass">
            <text>{{ sleepStatus }}</text>
          </view>
        </view>
      </view>

      <view class="section">
        <view class="section-title">
          <text class="title-icon">⚖️</text>
          <text>Body Metrics</text>
        </view>
        <view class="input-card">
          <view class="input-row">
            <text class="input-label">Weight (kg)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('weight', 0.5)">-</view>
              <text class="stepper-value">{{ userInputs.weight }}</text>
              <view class="stepper-btn plus" @click="increase('weight', 0.5)">+</view>
            </view>
          </view>
          <view class="input-row">
            <text class="input-label">Height (cm)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('height', 1)">-</view>
              <text class="stepper-value">{{ userInputs.height }}</text>
              <view class="stepper-btn plus" @click="increase('height', 1)">+</view>
            </view>
          </view>
          <view class="bmi-display" v-if="bmi > 0">
            <view class="bmi-value">
              <text class="bmi-number">{{ bmi }}</text>
              <text class="bmi-label">BMI</text>
            </view>
            <view class="bmi-status" :class="bmiStatusClass">
              <text>{{ bmiStatus }}</text>
            </view>
          </view>
        </view>
      </view>

      <view class="section">
        <view class="section-title">
          <text class="title-icon">🍽️</text>
          <text>Diet</text>
        </view>
        <view class="input-card">
          <view class="input-row">
            <text class="input-label">Meals Today</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('meals', 1)">-</view>
              <text class="stepper-value">{{ userInputs.meals }}</text>
              <view class="stepper-btn plus" @click="increase('meals', 1)">+</view>
            </view>
          </view>
          <view class="input-row">
            <text class="input-label">Vegetables (servings)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('vegetables', 1)">-</view>
              <text class="stepper-value">{{ userInputs.vegetables }}</text>
              <view class="stepper-btn plus" @click="increase('vegetables', 1)">+</view>
            </view>
          </view>
          <view class="input-row">
            <text class="input-label">Fruits (servings)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('fruits', 1)">-</view>
              <text class="stepper-value">{{ userInputs.fruits }}</text>
              <view class="stepper-btn plus" @click="increase('fruits', 1)">+</view>
            </view>
          </view>
          <view class="diet-summary">
            <text>Today's Diet Score: {{ dietScore }}/100</text>
            <view class="diet-progress">
              <view class="diet-fill" :style="{ width: dietScore + '%', background: dietColor }"></view>
            </view>
          </view>
        </view>
      </view>

      <view class="section">
        <view class="section-title">
          <text class="title-icon">🏃</text>
          <text>Exercise</text>
        </view>
        <view class="input-card">
          <view class="input-row">
            <text class="input-label">Steps Today</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('steps', 1000)">-</view>
              <text class="stepper-value">{{ userInputs.steps.toLocaleString() }}</text>
              <view class="stepper-btn plus" @click="increase('steps', 1000)">+</view>
            </view>
          </view>
          <view class="quick-btns">
            <view class="quick-btn" @click="setQuick('steps', 5000)">5K</view>
            <view class="quick-btn" @click="setQuick('steps', 8000)">8K</view>
            <view class="quick-btn" @click="setQuick('steps', 10000)">10K</view>
          </view>
          <view class="input-row">
            <text class="input-label">Exercise (minutes)</text>
            <view class="stepper">
              <view class="stepper-btn minus" @click="decrease('exercise', 15)">-</view>
              <text class="stepper-value">{{ userInputs.exercise }}</text>
              <view class="stepper-btn plus" @click="increase('exercise', 15)">+</view>
            </view>
          </view>
        </view>
      </view>

      <view class="section submit-section">
        <view class="submit-btn" @click="submitAndGetAdvice">
          <text v-if="!loading">Get AI Health Advice</text>
          <text v-else>Analyzing...</text>
        </view>
      </view>

      <view class="section" v-if="healthAdvice">
        <view class="section-title">
          <text class="title-icon">🤖</text>
          <text>AI Health Advice</text>
        </view>
        
        <!-- 结构化建议展示 -->
        <view class="advice-card structured" v-if="healthAdvice.type === 'structured'">
          <!-- 总体评价 -->
          <view class="summary-card">
            <text class="summary-text">{{ healthAdvice.summary }}</text>
          </view>
          
          <!-- 指标卡片 -->
          <view class="indicators-section">
            <view 
              v-for="(indicator, index) in healthAdvice.indicators" 
              :key="index"
              class="indicator-card"
              :class="indicator.status"
            >
              <view class="indicator-header">
                <text class="indicator-icon">{{ indicator.icon }}</text>
                <view class="indicator-info">
                  <text class="indicator-name">{{ indicator.name }}</text>
                  <text class="indicator-value">{{ indicator.value }}</text>
                </view>
                <view class="indicator-status-badge" :class="indicator.status">
                  <text>{{ indicator.statusText }}</text>
                </view>
              </view>
              <view class="indicator-advice">
                <text class="advice-label">💡 建议</text>
                <text class="advice-content">{{ indicator.advice }}</text>
              </view>
            </view>
          </view>
          
          <!-- 生活方式建议 -->
          <view class="lifestyle-section">
            <text class="lifestyle-title">🌟 生活方式建议</text>
            <view 
              v-for="(item, index) in healthAdvice.lifestyle" 
              :key="index"
              class="lifestyle-card"
            >
              <view class="lifestyle-header">
                <text class="lifestyle-icon">{{ item.icon }}</text>
                <text class="lifestyle-category">{{ item.category }}</text>
              </view>
              <view class="lifestyle-tips">
                <text 
                  v-for="(tip, tipIndex) in item.tips" 
                  :key="tipIndex"
                  class="tip-item"
                >{{ tipIndex + 1 }}. {{ tip }}</text>
              </view>
            </view>
          </view>
          
          <!-- 综合建议 -->
          <view class="overall-advice">
            <text class="overall-text">{{ healthAdvice.overallAdvice }}</text>
          </view>
        </view>
        
        <!-- 纯文本建议展示（兼容旧格式） -->
        <view class="advice-card text-only" v-else>
          <text class="advice-text">{{ healthAdvice.content || healthAdvice }}</text>
        </view>
      </view>
    </scroll-view>
  </view>
</template>

<script>
export default {
  data() {
    return {
      userInputs: {
        water: 0,
        coffee: 0,
        soda: 0,
        tea: 0,
        sleepHours: 7,
        sleepMinutes: 0,
        sleepQuality: 'Good',
        weight: 65,
        height: 170,
        meals: 3,
        vegetables: 2,
        fruits: 2,
        steps: 0,
        exercise: 0
      },
      healthAdvice: '',
      loading: false
    };
  },
  computed: {
    totalCaffeine() {
      return this.userInputs.coffee * 95 + this.userInputs.soda * 40 + this.userInputs.tea * 30;
    },
    caffeineLevel() {
      if (this.totalCaffeine < 200) return 'Low';
      if (this.totalCaffeine < 400) return 'Moderate';
      if (this.totalCaffeine < 600) return 'High';
      return 'Very High';
    },
    caffeineLevelClass() {
      if (this.totalCaffeine < 200) return 'low';
      if (this.totalCaffeine < 400) return 'moderate';
      if (this.totalCaffeine < 600) return 'high';
      return 'very-high';
    },
    bmi() {
      if (!this.userInputs.weight || !this.userInputs.height) return 0;
      const heightM = this.userInputs.height / 100;
      return Number((this.userInputs.weight / (heightM * heightM)).toFixed(1));
    },
    bmiStatus() {
      if (this.bmi < 18.5) return 'Underweight';
      if (this.bmi < 24) return 'Normal';
      if (this.bmi < 28) return 'Overweight';
      return 'Obese';
    },
    bmiStatusClass() {
      if (this.bmi < 18.5) return 'underweight';
      if (this.bmi < 24) return 'normal';
      if (this.bmi < 28) return 'overweight';
      return 'obese';
    },
    sleepStatus() {
      const hours = this.userInputs.sleepHours + this.userInputs.sleepMinutes / 60;
      if (hours < 6) return 'Insufficient sleep - may affect health';
      if (hours < 7) return 'Below recommended - try to get more rest';
      if (hours <= 9) return 'Good sleep duration';
      return 'Well above recommended - may be excessive';
    },
    sleepStatusClass() {
      const hours = this.userInputs.sleepHours + this.userInputs.sleepMinutes / 60;
      if (hours < 6) return 'poor';
      if (hours < 7) return 'fair';
      if (hours <= 9) return 'good';
      return 'excellent';
    },
    dietScore() {
      let score = 50;
      if (this.userInputs.vegetables >= 3) score += 15;
      else if (this.userInputs.vegetables >= 2) score += 10;
      else if (this.userInputs.vegetables >= 1) score += 5;

      if (this.userInputs.fruits >= 2) score += 15;
      else if (this.userInputs.fruits >= 1) score += 10;

      if (this.userInputs.meals >= 3 && this.userInputs.meals <= 5) score += 10;

      if (this.userInputs.water >= 1500) score += 10;
      else if (this.userInputs.water >= 1000) score += 7;
      else if (this.userInputs.water >= 500) score += 3;

      return Math.min(100, score);
    },
    dietColor() {
      if (this.dietScore < 40) return '#ef4444';
      if (this.dietScore < 70) return '#f59e0b';
      return '#22c55e';
    }
  },
  onLoad() {
    this.loadSavedData();
  },
  methods: {
    goBack() {
      uni.navigateBack();
    },
    increase(key, amount) {
      this.userInputs[key] = Math.min(9999, (this.userInputs[key] || 0) + amount);
    },
    decrease(key, amount) {
      this.userInputs[key] = Math.max(0, (this.userInputs[key] || 0) - amount);
    },
    increaseSleep() {
      this.userInputs.sleepMinutes += 15;
      if (this.userInputs.sleepMinutes >= 60) {
        this.userInputs.sleepHours += 1;
        this.userInputs.sleepMinutes = 0;
      }
    },
    decreaseSleep() {
      this.userInputs.sleepMinutes -= 15;
      if (this.userInputs.sleepMinutes < 0) {
        if (this.userInputs.sleepHours > 0) {
          this.userInputs.sleepHours -= 1;
          this.userInputs.sleepMinutes = 45;
        } else {
          this.userInputs.sleepMinutes = 0;
        }
      }
    },
    setQuick(key, value) {
      this.userInputs[key] = value;
    },
    loadSavedData() {
      const saved = uni.getStorageSync('healthInputs');
      if (saved) {
        this.userInputs = { ...this.userInputs, ...saved };
      }
    },
    saveData() {
      uni.setStorageSync('healthInputs', this.userInputs);
    },
    async submitAndGetAdvice() {
      this.saveData();

      // 检查是否已登录
      const token = uni.getStorageSync('token');
      if (!token) {
        uni.showToast({ title: '请先登录', icon: 'none', duration: 2000 });
        setTimeout(() => {
          uni.reLaunch({ url: '/pages/login/login' });
        }, 1500);
        return;
      }

      this.loading = true;

      try {
        const selectedDeviceId = uni.getStorageSync('selectedDeviceId') || 'ESP32_001';
        
        const resData = await new Promise((resolve, reject) => {
          uni.request({
            url: 'http://192.144.185.64:3002/api/ai/health-advice',
            method: 'POST',
            header: {
              'Authorization': 'Bearer ' + token,
              'Content-Type': 'application/json'
            },
            data: {
              deviceId: selectedDeviceId,
              date: new Date().toISOString().split('T')[0]
            },
            timeout: 60000,
            success: (res) => {
              console.log('AI建议响应:', JSON.stringify(res, null, 2));
              resolve(res.data || res);
            },
            fail: (err) => {
              reject(err);
            }
          });
        });

        if (!resData) {
          throw new Error('响应数据为空');
        }

        // 检查业务错误码（401/403 表示登录失效）
        if (resData.errcode === 401 || resData.errcode === 403) {
          this.handleLoginExpired();
          return;
        }

        if (resData.errcode === 0) {
          let advice = resData.data.advice;
          // 如果 advice 是对象且包含 indicators 字段，标记为结构化数据
          if (advice && typeof advice === 'object' && advice.indicators) {
            advice.type = 'structured';
          }
          this.healthAdvice = advice;
        } else {
          // 其他错误（如无数据）则使用本地建议
          this.healthAdvice = this.generateLocalAdvice();
          uni.showToast({ title: resData.msg || '获取建议失败，使用本地分析', icon: 'none' });
        }
      } catch (e) {
        console.error('AI建议请求失败:', e);
        this.healthAdvice = this.generateLocalAdvice();
        const errorMsg = e.errMsg || e.message || '未知错误';
        uni.showToast({ title: '网络错误: ' + errorMsg, icon: 'none', duration: 3000 });
      } finally {
        this.loading = false;
      }
    },

    // 新增方法：处理登录失效
    handleLoginExpired() {
      uni.removeStorageSync('token');
      uni.removeStorageSync('userInfo');
      uni.showToast({ title: '登录已失效，请重新登录', icon: 'none', duration: 2000 });
      setTimeout(() => {
        uni.reLaunch({ url: '/pages/login/login' }); // 根据实际登录页路径修改
      }, 1500);
    },
    generateLocalAdvice() {
      const advices = [];
      const hours = this.userInputs.sleepHours + this.userInputs.sleepMinutes / 60;

      if (hours < 7) {
        advices.push('Consider getting more sleep - adults typically need 7-9 hours.');
      }

      if (this.userInputs.water < 1500) {
        advices.push('Increase your water intake to at least 1.5L daily.');
      }

      if (this.totalCaffeine > 400) {
        advices.push('Your caffeine intake is high. Consider reducing coffee or soda.');
      }

      if (this.bmi > 25) {
        advices.push('Your BMI is in the overweight range. Consider more exercise and balanced diet.');
      } else if (this.bmi < 18.5) {
        advices.push('Your BMI is in the underweight range. Consider a more nutritious diet.');
      }

      if (this.userInputs.vegetables < 2) {
        advices.push('Try to eat at least 2 servings of vegetables daily.');
      }

      if (this.userInputs.fruits < 2) {
        advices.push('Include 2 servings of fruits in your daily diet.');
      }

      if (this.userInputs.steps < 5000) {
        advices.push('Try to walk at least 5,000 steps daily for better health.');
      }

      if (this.userInputs.exercise < 30) {
        advices.push('Consider adding at least 30 minutes of exercise daily.');
      }

      if (advices.length === 0) {
        return 'Great job! Your health habits look good. Keep maintaining this balanced lifestyle.';
      }

      return advices.join(' ');
    }
  }
};
</script>

<style lang="scss" scoped>
.health-input-container {
  min-height: 100vh;
  background: #f8fafc;
}

.nav-bar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 60rpx 24rpx 20rpx;
  background: white;
  position: sticky;
  top: 0;
  z-index: 10;
}

.nav-back {
  display: flex;
  align-items: center;
  gap: 8rpx;
}

.back-icon {
  font-size: 48rpx;
  color: #1e293b;
}

.back-text {
  font-size: 28rpx;
  color: #1e293b;
}

.nav-title {
  font-size: 32rpx;
  font-weight: 600;
  color: #1e293b;
}

.nav-placeholder {
  width: 100rpx;
}

.content-scroll {
  height: calc(100vh - 120rpx);
}

.section {
  padding: 24rpx;
}

.section-title {
  display: flex;
  align-items: center;
  gap: 12rpx;
  margin-bottom: 16rpx;
}

.title-icon {
  font-size: 32rpx;
}

.section-title text:not(.title-icon) {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
}

.input-card {
  background: white;
  border-radius: 20rpx;
  padding: 24rpx;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.04);
}

.input-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20rpx;
}

.input-row:last-child {
  margin-bottom: 0;
}

.input-label {
  font-size: 28rpx;
  color: #475569;
}

.stepper {
  display: flex;
  align-items: center;
  gap: 16rpx;
}

.stepper-btn {
  width: 64rpx;
  height: 64rpx;
  border-radius: 50%;
  background: #f1f5f9;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 32rpx;
  color: #64748b;
}

.stepper-btn:active {
  background: #e2e8f0;
}

.stepper-value {
  font-size: 32rpx;
  font-weight: 600;
  color: #1e293b;
  min-width: 120rpx;
  text-align: center;
}

.quick-btns {
  display: flex;
  gap: 12rpx;
  margin-bottom: 20rpx;
}

.quick-btn {
  flex: 1;
  padding: 12rpx;
  text-align: center;
  background: #f1f5f9;
  border-radius: 12rpx;
  font-size: 24rpx;
  color: #64748b;
}

.quick-btn:active {
  background: #e2e8f0;
}

.caffeine-info {
  margin-bottom: 20rpx;
}

.caffeine-note {
  font-size: 22rpx;
  color: #94a3b8;
}

.total-caffeine {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16rpx;
  background: #f8fafc;
  border-radius: 12rpx;
  margin-top: 16rpx;
}

.total-caffeine text:first-child {
  font-size: 26rpx;
  color: #475569;
}

.caffeine-level {
  font-size: 24rpx;
  font-weight: 600;
  padding: 6rpx 16rpx;
  border-radius: 20rpx;
}

.caffeine-level.low { background: #dcfce7; color: #166534; }
.caffeine-level.moderate { background: #fef9c3; color: #854d0e; }
.caffeine-level.high { background: #fee2e2; color: #991b1b; }
.caffeine-level.very-high { background: #fecaca; color: #7f1d1d; }

.sleep-quality {
  display: flex;
  align-items: center;
  gap: 16rpx;
  margin-bottom: 16rpx;
}

.sleep-quality > text {
  font-size: 28rpx;
  color: #475569;
}

.quality-btns {
  display: flex;
  gap: 8rpx;
}

.quality-btn {
  padding: 8rpx 16rpx;
  font-size: 22rpx;
  color: #64748b;
  background: #f1f5f9;
  border-radius: 8rpx;
}

.quality-btn.active {
  background: #6366f1;
  color: white;
}

.sleep-status {
  text-align: center;
  padding: 12rpx;
  border-radius: 12rpx;
  font-size: 24rpx;
}

.sleep-status.poor { background: #fee2e2; color: #991b1b; }
.sleep-status.fair { background: #fef9c3; color: #854d0e; }
.sleep-status.good { background: #dcfce7; color: #166534; }
.sleep-status.excellent { background: #dcfce7; color: #166534; }

.bmi-display {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20rpx;
  background: #f8fafc;
  border-radius: 12rpx;
  margin-top: 16rpx;
}

.bmi-value {
  display: flex;
  align-items: baseline;
  gap: 8rpx;
}

.bmi-number {
  font-size: 48rpx;
  font-weight: 700;
  color: #1e293b;
}

.bmi-label {
  font-size: 24rpx;
  color: #64748b;
}

.bmi-status {
  padding: 8rpx 20rpx;
  border-radius: 20rpx;
  font-size: 24rpx;
  font-weight: 500;
}

.bmi-status.underweight { background: #fef9c3; color: #854d0e; }
.bmi-status.normal { background: #dcfce7; color: #166534; }
.bmi-status.overweight { background: #fee2e2; color: #991b1b; }
.bmi-status.obese { background: #fecaca; color: #7f1d1d; }

.diet-summary {
  margin-top: 16rpx;
  padding: 16rpx;
  background: #f8fafc;
  border-radius: 12rpx;
}

.diet-summary > text {
  font-size: 26rpx;
  color: #475569;
  display: block;
  margin-bottom: 12rpx;
}

.diet-progress {
  height: 12rpx;
  background: #e2e8f0;
  border-radius: 6rpx;
  overflow: hidden;
}

.diet-fill {
  height: 100%;
  border-radius: 6rpx;
  transition: width 0.3s ease;
}

.submit-section {
  padding-top: 0;
}

.submit-btn {
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
  color: white;
  text-align: center;
  padding: 24rpx;
  border-radius: 20rpx;
  font-size: 32rpx;
  font-weight: 600;
}

.advice-card {
  background: white;
  border-radius: 20rpx;
  padding: 24rpx;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.04);
}

.advice-card.structured {
  padding: 0;
  overflow: hidden;
}

.advice-text {
  font-size: 28rpx;
  color: #475569;
  line-height: 1.6;
}

/* 总体评价卡片 */
.summary-card {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  padding: 32rpx;
  margin-bottom: 24rpx;
}

.summary-text {
  color: white;
  font-size: 30rpx;
  font-weight: 600;
  line-height: 1.5;
}

/* 指标区域 */
.indicators-section {
  padding: 0 24rpx;
  margin-bottom: 24rpx;
}

.indicator-card {
  background: #f8fafc;
  border-radius: 16rpx;
  padding: 24rpx;
  margin-bottom: 16rpx;
  border-left: 6rpx solid #e2e8f0;
}

.indicator-card.normal {
  background: #f0fdf4;
  border-left-color: #22c55e;
}

.indicator-card.warning {
  background: #fffbeb;
  border-left-color: #f59e0b;
}

.indicator-card.danger {
  background: #fef2f2;
  border-left-color: #ef4444;
}

.indicator-header {
  display: flex;
  align-items: center;
  gap: 16rpx;
  margin-bottom: 16rpx;
}

.indicator-icon {
  font-size: 40rpx;
}

.indicator-info {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 4rpx;
}

.indicator-name {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
}

.indicator-value {
  font-size: 24rpx;
  color: #64748b;
}

.indicator-status-badge {
  padding: 8rpx 16rpx;
  border-radius: 20rpx;
  font-size: 22rpx;
  font-weight: 500;
}

.indicator-status-badge.normal {
  background: #dcfce7;
  color: #166534;
}

.indicator-status-badge.warning {
  background: #fef3c7;
  color: #92400e;
}

.indicator-status-badge.danger {
  background: #fee2e2;
  color: #991b1b;
}

.indicator-advice {
  background: white;
  border-radius: 12rpx;
  padding: 16rpx;
  margin-top: 8rpx;
}

.advice-label {
  font-size: 24rpx;
  color: #64748b;
  display: block;
  margin-bottom: 8rpx;
}

.advice-content {
  font-size: 26rpx;
  color: #334155;
  line-height: 1.5;
}

/* 生活方式建议区域 */
.lifestyle-section {
  padding: 0 24rpx;
  margin-bottom: 24rpx;
}

.lifestyle-title {
  font-size: 30rpx;
  font-weight: 600;
  color: #1e293b;
  display: block;
  margin-bottom: 16rpx;
}

.lifestyle-card {
  background: #f8fafc;
  border-radius: 16rpx;
  padding: 20rpx;
  margin-bottom: 12rpx;
}

.lifestyle-header {
  display: flex;
  align-items: center;
  gap: 12rpx;
  margin-bottom: 12rpx;
}

.lifestyle-icon {
  font-size: 32rpx;
}

.lifestyle-category {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
}

.lifestyle-tips {
  display: flex;
  flex-direction: column;
  gap: 8rpx;
}

.tip-item {
  font-size: 26rpx;
  color: #475569;
  line-height: 1.5;
  padding-left: 8rpx;
}

/* 综合建议 */
.overall-advice {
  background: linear-gradient(135deg, #e0f2fe 0%, #dbeafe 100%);
  padding: 24rpx;
  margin: 0 24rpx 24rpx;
  border-radius: 12rpx;
}

.overall-text {
  font-size: 28rpx;
  color: #1e40af;
  line-height: 1.6;
  font-weight: 500;
}
</style>
