<template>
  <view class="container">
    <view class="header">
      <view class="user-info">
        <view class="avatar-wrapper">
          <text class="avatar">{{ user?.nickname?.charAt(0) || 'U' }}</text>
        </view>
        <view class="greeting">
          <text class="greeting-text">{{ greeting }} {{ user?.nickname || 'User' }}</text>
          <text class="date-text">{{ currentDate }}</text>
        </view>
      </view>
      <view class="header-actions">
        <view class="action-btn" @click="goToNotifications">
          <text class="action-icon">🔔</text>
          <view class="badge" v-if="notificationCount > 0">{{ notificationCount }}</view>
        </view>
      </view>
    </view>

    <view class="status-section">
      <view class="connection-status" :class="{ connected: isConnected, disconnected: !isConnected }">
        <view class="status-dot"></view>
        <text class="status-text">{{ isConnected ? t('deviceConnected') : t('noDeviceConnected') }}</text>
        <text class="status-hint">{{ isConnected ? t('realTimeMonitoring') : t('pleaseConnectDevice') }}</text>
      </view>

      <view class="demo-control" v-if="!isConnected || isDemoRunning">
        <view class="mode-toggle">
          <text class="toggle-label">Sim</text>
          <view class="toggle-switch" :class="{ active: isSimulationMode }" @click="switchMode(isSimulationMode ? 'real' : 'simulation')">
            <view class="toggle-thumb"></view>
          </view>
          <text class="toggle-label">Real</text>
        </view>
        <view class="btn-row">
          <button class="demo-btn start" @click="startDemo" :disabled="isDemoRunning">
            {{ isDemoRunning ? '测量中' : '开始测量' }}
          </button>
          <button class="demo-btn refresh" @click="refreshDemoData" :disabled="!isDemoRunning || isRefreshing">
            {{ isRefreshing ? '更新中...' : '刷新数据' }}
          </button>
          <button class="demo-btn server" @click="loadServerData">
            读取服务器数据
          </button>
        </view>
      </view>
    </view>

    <view class="main-content" v-if="hasData && isConnected">
      <view class="realtime-section">
        <view class="section-header">
          <text class="section-title">{{ t('realTimeData') }}</text>
          <view class="live-badge">
            <view class="live-dot"></view>
            <text>{{ t('live') }}</text>
          </view>
        </view>

        <view class="metrics-vertical">
          <view class="metric-row heart-rate">
            <view class="metric-left">
              <text class="metric-icon">❤️</text>
              <view class="metric-label-group">
                <text class="metric-label">Heart Rate</text>
                <text class="metric-status" :class="getHeartRateStatus(realtimeData.heart_rate)">
                  {{ getHeartRateText(realtimeData.heart_rate) }}
                </text>
              </view>
            </view>
            <view class="metric-right">
              <text class="metric-value">{{ realtimeData.heart_rate || '--' }}</text>
              <text class="metric-unit">bpm</text>
            </view>
          </view>

          <view class="metric-row blood-pressure">
            <view class="metric-left">
              <text class="metric-icon">🩸</text>
              <view class="metric-label-group">
                <text class="metric-label">Blood Pressure</text>
                <text class="metric-status" :class="getBPStatus(realtimeData.systolic_bp)">
                  {{ getBPText(realtimeData.systolic_bp) }}
                </text>
              </view>
            </view>
            <view class="metric-right">
              <text class="metric-value">
                {{ realtimeData.systolic_bp || '--' }}/{{ realtimeData.diastolic_bp || '--' }}
              </text>
              <text class="metric-unit">mmHg</text>
            </view>
          </view>

          <view class="metric-row spo2">
            <view class="metric-left">
              <text class="metric-icon">🫁</text>
              <view class="metric-label-group">
                <text class="metric-label">SpO2</text>
                <text class="metric-status" :class="getSpO2Status(realtimeData.spo2)">
                  {{ getSpO2Text(realtimeData.spo2) }}
                </text>
              </view>
            </view>
            <view class="metric-right">
              <text class="metric-value">{{ realtimeData.spo2 || '--' }}</text>
              <text class="metric-unit">%</text>
            </view>
          </view>
        </view>
      </view>

      <view class="waveform-section">
        <view class="section-header">
          <text class="section-title">实时波形</text>
          <view class="live-badge">
            <view class="live-dot"></view>
            <text>实时</text>
          </view>
        </view>
        <view class="waveform-container">
          <canvas canvas-id="waveformCanvas" class="waveform-canvas" :style="{ width: canvasWidth + 'px', height: canvasHeight + 'px' }"></canvas>
        </view>
      </view>

      <view class="mini-chart-section">
        <view class="section-header">
          <text class="section-title">Last Hour Average</text>
        </view>
        <view class="mini-chart">
          <view class="chart-items">
            <view
              v-for="(item, index) in miniChartData"
              :key="index"
              class="chart-bar"
              :style="{ height: item.heart_rate ? (item.heart_rate / 120 * 100) + '%' : '10%' }"
            ></view>
          </view>
          <view class="chart-labels">
            <text>-60m</text>
            <text>-30m</text>
            <text>Now</text>
          </view>
        </view>
      </view>

      <view class="ai-advice-section" v-if="aiAdvice">
        <view class="section-header">
          <text class="section-title">🤖 AI Health Advice</text>
          <text class="advice-date" v-if="aiAdviceDate">{{ aiAdviceDate }}</text>
        </view>
        
        <view v-if="aiAdvice.physiologicalAge" class="age-card">
          <view class="age-header">
            <text class="age-icon">🧬</text>
            <text class="age-title">Physiological Age</text>
          </view>
          <view class="age-content">
            <view class="age-comparison">
              <view class="age-item">
                <text class="age-label">Actual Age</text>
                <text class="age-value">{{ aiAdvice.physiologicalAge.actualAge }}</text>
              </view>
              <view class="age-arrow">→</view>
              <view class="age-item">
                <text class="age-label">Body Age</text>
                <text class="age-value" :class="aiAdvice.physiologicalAge.status">
                  {{ aiAdvice.physiologicalAge.physiologicalAge }}
                </text>
              </view>
            </view>
            <view class="age-difference" :class="aiAdvice.physiologicalAge.status">
              <text class="diff-badge">{{ aiAdvice.physiologicalAge.difference }}</text>
              <text class="diff-text">{{ getAgeDiffText(aiAdvice.physiologicalAge.status) }}</text>
            </view>
            <text class="age-explanation">{{ aiAdvice.physiologicalAge.explanation }}</text>
          </view>
        </view>

        <view class="ai-summary-card">
          <text class="ai-summary-text">{{ aiAdvice.summary || aiAdvice.content }}</text>
        </view>

        <view v-if="aiAdvice.indicators && aiAdvice.indicators.length" class="indicators-grid">
          <view
            v-for="(indicator, idx) in aiAdvice.indicators"
            :key="idx"
            class="indicator-card"
            :class="indicator.status"
          >
            <view class="indicator-header">
              <text class="indicator-icon">{{ indicator.icon }}</text>
              <view class="indicator-info">
                <text class="indicator-name">{{ indicator.name }}</text>
                <text class="indicator-value">{{ indicator.value }}</text>
              </view>
              <view class="indicator-badge" :class="indicator.status">
                <text class="badge-text">{{ indicator.statusText }}</text>
              </view>
            </view>
            <text class="indicator-advice">{{ indicator.advice }}</text>
          </view>
        </view>

        <view v-if="aiAdvice.lifestyle && aiAdvice.lifestyle.length" class="lifestyle-section">
          <text class="lifestyle-title">💡 Lifestyle Tips</text>
          <view
            v-for="(life, idx) in aiAdvice.lifestyle"
            :key="idx"
            class="lifestyle-card"
          >
            <view class="lifestyle-header">
              <text class="lifestyle-icon">{{ life.icon }}</text>
              <text class="lifestyle-category">{{ life.category }}</text>
            </view>
            <view class="lifestyle-tips">
              <view
                v-for="(tip, tipIdx) in life.tips"
                :key="tipIdx"
                class="tip-item"
              >
                <view class="tip-dot"></view>
                <text class="tip-text">{{ tip }}</text>
              </view>
            </view>
          </view>
        </view>

        <view v-if="aiAdvice.overallAdvice" class="overall-advice-card">
          <view class="overall-header">
            <text class="overall-icon">🎯</text>
            <text class="overall-title">Overall Advice</text>
          </view>
          <text class="overall-text">{{ aiAdvice.overallAdvice }}</text>
        </view>
      </view>
    </view>

    <view class="empty-state" v-else>
      <view class="empty-illustration">
        <view class="device-icon">📱</view>
        <view class="connection-lines">
          <view class="line line1"></view>
          <view class="line line2"></view>
          <view class="line line3"></view>
        </view>
        <view class="sensor-icon">🩺</view>
      </view>

      <view class="empty-content">
        <text class="empty-title">{{ isConnected ? t('waitingPulseWave') : t('connectYourDevice') }}</text>
        <text class="empty-desc">
          {{ isConnected
            ? t('placeFingerOnSensor')
            : t('connectDeviceDesc')
          }}
        </text>

        <view class="empty-actions">
          <view class="action-card" @click="goToDeviceList">
            <text class="action-icon">🔗</text>
            <text class="action-text">{{ t('connectDevice') }}</text>
          </view>
          <view class="action-card" @click="goToHistory">
            <text class="action-icon">📊</text>
            <text class="action-text">{{ t('viewHistory') }}</text>
          </view>
        </view>
      </view>

      <view class="health-tips">
        <text class="tips-title">💡 {{ t('healthTips') }}</text>
        <view class="tips-list">
          <view class="tip-item" v-for="(tip, index) in healthTips" :key="index">
            <text class="tip-icon">{{ tip.icon }}</text>
            <text class="tip-text">{{ tip.text }}</text>
          </view>
        </view>
      </view>

      <view class="quick-guide">
        <text class="guide-title">📖 {{ t('quickGuide') }}</text>
        <view class="guide-steps">
          <view class="guide-step">
            <text class="step-number">1</text>
            <text class="step-text">{{ t('guideStep1') }}</text>
          </view>
          <view class="guide-step">
            <text class="step-number">2</text>
            <text class="step-text">{{ t('guideStep2') }}</text>
          </view>
          <view class="guide-step">
            <text class="step-number">3</text>
            <text class="step-text">{{ t('guideStep3') }}</text>
          </view>
        </view>
      </view>
    </view>
  </view>
</template>

<script>
import { t as translate } from '@/utils/i18n.js';
import { getDataUrl, getAuthUrl } from '@/config/api.js';

export default {
  data() {
    return {
      user: null,
      selectedDeviceId: '',
      isConnected: false,
      hasData: false,
      realtimeData: {
        heart_rate: null,
        systolic_bp: null,
        diastolic_bp: null,
        spo2: null
      },
      miniChartData: [],
      notificationCount: 0,
      healthTips: [],
      aiAdvice: null,
      aiAdviceDate: '',
      timer: null,
      statusTimer: null,
      dataFetchCount: 0,
      isDemoRunning: false,
      demoTimer: null,
      waveTimer: null,
      isRefreshing: false,
      isSimulationMode: true,
      canvasWidth: 350,
      canvasHeight: 150,
      waveData: [],
      waveOffset: 0,
      baseHeartRate: 74,
      baseSystolic: 139,
      baseDiastolic: 73,
      baseSpO2: 98,
      realWaveTemplate: [0.35,0.50,0.37,0.36,0.48,0.69,0.50,0.50,0.72,0.68,0.83,0.20,1.00,0.59,0.52,0.73,0.80,0.73,0.64,0.63,0.53,0.49,0.54,0.43,0.32,0.45,0.61,0.63,0.21,0.64,0.25,0.60,0.50,0.49,0.50,0.44,0.54,0.50,0.45,0.52,0.45,0.39,0.60,0.61,0.73,0.61,0.37,0.23,0.24,0.50,0.20,0.30,0.65,0.21,0.30,0.46,0.55,0.28,0.31,0.38,0.48,0.49,0.44,0.51,0.62,0.39,0.65,0.22,0.28,0.82,0.69,0.42,0.53,0.35,0.61,0.56,0.36,0.69,0.37,0.64,0.50,0.48,0.41,0.36,0.48,0.47,0.66,0.66,0.32,0.53,0.55,0.71,0.51,0.34,0.40,0.58,0.52,0.42,0.49,0.59],
      waveIndex: 0,
      serverWaveData: [],
      serverDataIndex: 0,
      serverWaveTimer: null
    };
  },
  computed: {
    greeting() {
      const hour = new Date().getHours();
      if (hour < 12) return 'Good morning';
      if (hour < 18) return 'Good afternoon';
      return 'Good evening';
    },
    currentDate() {
      const now = new Date();
      const weekdays = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
      return `${weekdays[now.getDay()]}, ${now.getMonth() + 1}/${now.getDate()}`;
    }
  },
  onLoad() {
    this.initHealthTips();
    this.loadUserInfo();
    this.loadSelectedDevice();
    this.startRealtimeUpdate();
    this.startDeviceStatusCheck();
    
    if (this.isSimulationMode) {
      this.isConnected = true;
      this.hasData = true;
      
      this.realtimeData = {
        heart_rate: 74,
        systolic_bp: 139,
        diastolic_bp: 73,
        spo2: Math.floor(95 + Math.random() * 5)
      };
      
      this.generateMockTrendData();
      this.generateMockAIAdvice();
      
      setTimeout(() => {
        this.startDemo();
      }, 500);
    }
    
    this.fetchAIAdvice();
  },
  onUnload() {
    if (this.timer) {
      clearInterval(this.timer);
    }
    if (this.statusTimer) {
      clearInterval(this.statusTimer);
    }
    if (this.demoTimer) {
      clearInterval(this.demoTimer);
    }
    if (this.waveTimer) {
      clearInterval(this.waveTimer);
    }
  },
  onReady() {
    this.initWaveData();
  },
  methods: {
    t(key) {
      return translate(key);
    },
    initWaveData() {
      this.waveData = [];
      for (let i = 0; i < 100; i++) {
        const idx = i % this.realWaveTemplate.length;
        this.waveData.push(this.realWaveTemplate[idx]);
      }
    },
    switchMode(mode) {
      this.isSimulationMode = mode === 'simulation';
      uni.setStorageSync('isSimulationMode', this.isSimulationMode.toString());
      if (this.isSimulationMode) {
        this.generateMockTrendData();
        this.generateMockAIAdvice();
      }
      uni.showToast({ 
        title: this.isSimulationMode ? '已切换到模拟模式' : '已切换到真实模式', 
        icon: 'none' 
      });
    },
    async loadServerData() {
      uni.showLoading({ title: '读取服务器数据...' });
      try {
        const serverUrl = 'http://192.144.185.64:3002/api/data/txt';
        
        const res = await uni.request({
          url: serverUrl,
          method: 'GET',
          responseType: 'text'
        });
        
        if (res.statusCode === 200) {
          this.parseServerData(res.data);
          uni.showToast({ title: '数据读取成功', icon: 'success' });
        } else {
          uni.showToast({ title: '读取失败', icon: 'none' });
        }
      } catch (err) {
        uni.showToast({ title: '网络错误', icon: 'none' });
      } finally {
        uni.hideLoading();
      }
    },
    parseServerData(data) {
      const lines = data.trim().split('\n');
      const waveValues = [];
      const metrics = { heartRate: [], systolic: [], diastolic: [], spo2: [] };
      
      lines.forEach(line => {
        const parts = line.trim().split(/[\t, ]+/);
        if (parts.length >= 2) {
          const value = parseFloat(parts[0]);
          if (!isNaN(value)) {
            waveValues.push(value);
          }
          if (parts.length >= 5) {
            metrics.heartRate.push(parseFloat(parts[1]) || 75);
            metrics.systolic.push(parseFloat(parts[2]) || 120);
            metrics.diastolic.push(parseFloat(parts[3]) || 80);
            metrics.spo2.push(parseFloat(parts[4]) || 98);
          }
        }
      });
      
      if (waveValues.length > 0) {
        const min = Math.min(...waveValues);
        const max = Math.max(...waveValues);
        this.serverWaveData = waveValues.map(v => (v - min) / (max - min || 1));
        this.serverDataIndex = 0;
        
        if (metrics.heartRate.length > 0) {
          const avgHr = Math.round(metrics.heartRate.reduce((a, b) => a + b, 0) / metrics.heartRate.length);
          const avgSys = Math.round(metrics.systolic.reduce((a, b) => a + b, 0) / metrics.systolic.length);
          const avgDia = Math.round(metrics.diastolic.reduce((a, b) => a + b, 0) / metrics.diastolic.length);
          const avgSpo2 = Math.round(metrics.spo2.reduce((a, b) => a + b, 0) / metrics.spo2.length);
          
          this.realtimeData = {
            heart_rate: avgHr,
            systolic_bp: avgSys,
            diastolic_bp: avgDia,
            spo2: avgSpo2
          };
          
          this.hasData = true;
          this.isConnected = true;
        }
        
        this.startServerWavePlayback();
      }
    },
    startServerWavePlayback() {
      if (this.serverWaveTimer) {
        clearInterval(this.serverWaveTimer);
      }
      
      this.waveData = [];
      for (let i = 0; i < 50; i++) {
        const idx = (this.serverDataIndex + i) % this.serverWaveData.length;
        this.waveData.push(this.serverWaveData[idx]);
      }
      
      this.serverWaveTimer = setInterval(() => {
        if (this.serverWaveData.length > 0) {
          this.waveData.shift();
          this.waveData.push(this.serverWaveData[this.serverDataIndex]);
          this.serverDataIndex = (this.serverDataIndex + 1) % this.serverWaveData.length;
          this.drawWaveform();
        }
      }, 30);
    },
    generateMockTrendData() {
      const mockData = [];
      const now = new Date();
      
      for (let i = 29; i >= 0; i--) {
        const date = new Date(now);
        date.setDate(date.getDate() - i);
        const dateStr = date.toISOString().split('T')[0];
        
        const baseHr = Math.floor(65 + Math.random() * 15);
        const baseSys = Math.floor(105 + Math.random() * 15);
        const baseDia = Math.floor(65 + Math.random() * 15);
        const baseSpo2 = Math.floor(95 + Math.random() * 3);
        
        const recordCount = i === 0 ? Math.floor(5 + Math.random() * 5) : Math.floor(2 + Math.random() * 4);
        
        for (let j = 0; j < recordCount; j++) {
          mockData.push({
            date: dateStr,
            heart_rate: Math.floor(baseHr + (Math.random() - 0.5) * 10),
            systolic_bp: Math.floor(baseSys + (Math.random() - 0.5) * 8),
            diastolic_bp: Math.floor(baseDia + (Math.random() - 0.5) * 6),
            spo2: Math.max(94, Math.min(100, baseSpo2 + Math.floor((Math.random() - 0.5) * 3))),
            measure_time: dateStr + ' ' + String(Math.floor(6 + Math.random() * 14)).padStart(2, '0') + ':' + String(Math.floor(0 + Math.random() * 60)).padStart(2, '0') + ':' + String(Math.floor(0 + Math.random() * 60)).padStart(2, '0')
          });
        }
      }
      
      uni.setStorageSync('mockTrendData', JSON.stringify(mockData));
    },
    generateMockAIAdvice() {
      const hr = this.realtimeData.heart_rate || 75;
      const sys = this.realtimeData.systolic_bp || 120;
      const dia = this.realtimeData.diastolic_bp || 80;
      const spo2 = this.realtimeData.spo2 || 98;
      
      let physiologicalAge = Math.floor(25 + Math.random() * 10);
      let actualAge = Math.floor(28 + Math.random() * 5);
      let difference = physiologicalAge - actualAge;
      
      this.aiAdvice = {
        physiologicalAge: {
          actualAge: actualAge,
          physiologicalAge: physiologicalAge,
          status: difference < 0 ? 'good' : difference > 3 ? 'warning' : 'normal',
          difference: difference > 0 ? `+${difference}` : difference.toString(),
          explanation: difference < 0 
            ? 'Your physiological age is younger than your actual age. Keep up the good work!'
            : difference > 3 
              ? 'Your physiological age is older than your actual age. Consider improving your lifestyle.'
              : 'Your physiological age matches your actual age.'
        },
        summary: this.getMockSummary(hr, sys, dia, spo2),
        indicators: [
          { name: 'Heart Rate', value: hr, status: hr < 60 ? 'low' : hr > 100 ? 'high' : 'normal', advice: hr < 60 ? 'Consider light exercise' : hr > 100 ? 'Try to relax and reduce stress' : 'Heart rate is within normal range' },
          { name: 'Systolic BP', value: sys, status: sys < 90 ? 'low' : sys > 140 ? 'high' : 'normal', advice: sys > 140 ? 'Reduce salt intake and exercise regularly' : 'Blood pressure is normal' },
          { name: 'Diastolic BP', value: dia, status: dia < 60 ? 'low' : dia > 90 ? 'high' : 'normal', advice: dia > 90 ? 'Consider consulting a doctor' : 'Diastolic pressure is normal' },
          { name: 'SpO2', value: spo2, status: spo2 < 95 ? 'low' : 'normal', advice: spo2 < 95 ? 'Ensure proper ventilation' : 'Oxygen level is excellent' }
        ],
        lifestyle: [
          { title: 'Exercise', advice: 'Aim for 30 minutes of moderate exercise daily', icon: '🏃' },
          { title: 'Sleep', advice: 'Get 7-8 hours of quality sleep each night', icon: '😴' },
          { title: 'Diet', advice: 'Maintain a balanced diet with plenty of fruits and vegetables', icon: '🥗' },
          { title: 'Hydration', advice: 'Drink at least 1.5 liters of water daily', icon: '💧' }
        ],
        overallAdvice: 'Overall, your health indicators are within normal range. Continue maintaining healthy habits including regular exercise, balanced nutrition, and sufficient sleep.'
      };
      this.aiAdviceDate = new Date().toLocaleDateString();
    },
    getMockSummary(hr, sys, dia, spo2) {
      const messages = [];
      if (hr >= 60 && hr <= 100) messages.push('Your heart rate is in good condition');
      if (sys < 130 && dia < 85) messages.push('Blood pressure is well controlled');
      if (spo2 >= 95) messages.push('Oxygen saturation is excellent');
      if (messages.length === 0) messages.push('Your vital signs show good health');
      return messages.join('. ') + '.';
    },
    toggleConnection() {
      this.isConnected = !this.isConnected;
      if (this.isConnected) {
        this.hasData = true;
        const mockData = this.generateMockData();
        this.realtimeData = mockData;
        uni.showToast({ title: '设备已连接', icon: 'success' });
      } else {
        this.isDemoRunning = false;
        if (this.waveTimer) {
          clearInterval(this.waveTimer);
          this.waveTimer = null;
        }
        uni.showToast({ title: '设备已断开', icon: 'none' });
      }
    },
    startDemo() {
      this.isDemoRunning = true;
      this.hasData = true;
      this.isConnected = true;
      this.waveOffset = 0;
      
      const mockData = this.generateMockData();
      this.realtimeData = mockData;

      const waveInterval = setInterval(() => {
        this.waveOffset += 0.15;
        this.updateWaveData();
        this.drawWaveform();
      }, 50);

      this.waveTimer = waveInterval;
      
      uni.showToast({ title: '开始测量', icon: 'success' });
    },
    refreshDemoData() {
      if (!this.isDemoRunning || this.isRefreshing) return;
      
      this.isRefreshing = true;
      
      setTimeout(() => {
        this.baseHeartRate = 74;
        this.baseSystolic = 139;
        this.baseDiastolic = 73;
        this.baseSpO2 = Math.max(94, Math.min(100, this.baseSpO2 + (Math.random() - 0.5) * 2));
        
        const mockData = this.generateMockData();
        this.realtimeData = mockData;

        const now = Date.now();
        this.miniChartData.push({
          heart_rate: mockData.heart_rate,
          systolic_bp: mockData.systolic_bp,
          diastolic_bp: mockData.diastolic_bp,
          spo2: mockData.spo2,
          timestamp: now
        });

        if (this.miniChartData.length > 10) {
          this.miniChartData.shift();
        }

        this.isRefreshing = false;
        uni.showToast({ title: '数据已更新', icon: 'success' });
      }, 2000 + Math.random() * 1500);
    },
    updateWaveData() {
      const heartRate = this.realtimeData.heart_rate || 75;
      const template = this.realWaveTemplate;
      const templateLength = template.length;
      
      const speed = heartRate / 75;
      this.waveIndex += speed * 0.8;
      
      if (this.waveIndex >= templateLength) {
        this.waveIndex = this.waveIndex % templateLength;
      }
      
      const idx1 = Math.floor(this.waveIndex);
      const idx2 = (idx1 + 1) % templateLength;
      const frac = this.waveIndex - idx1;
      
      let value = template[idx1] * (1 - frac) + template[idx2] * frac;
      
      const noise = (Math.random() - 0.5) * 0.015;
      value = Math.max(0.02, Math.min(1, value + noise));
      
      this.waveData.push(value);
      if (this.waveData.length > 100) {
        this.waveData.shift();
      }
    },
    drawWaveform() {
      const ctx = uni.createCanvasContext('waveformCanvas', this);
      const width = this.canvasWidth;
      const height = this.canvasHeight;
      const centerY = height / 2;
      
      ctx.clearRect(0, 0, width, height);
      
      ctx.setStrokeStyle('#10b981');
      ctx.setLineWidth(2);
      ctx.beginPath();
      
      const step = width / (this.waveData.length - 1);
      
      for (let i = 0; i < this.waveData.length; i++) {
        const x = i * step;
        const y = centerY - this.waveData[i] * (height * 0.4);
        
        if (i === 0) {
          ctx.moveTo(x, y);
        } else {
          ctx.lineTo(x, y);
        }
      }
      
      ctx.stroke();
      
      ctx.setStrokeStyle('rgba(16, 185, 129, 0.1)');
      ctx.setLineWidth(1);
      ctx.beginPath();
      ctx.moveTo(0, centerY);
      ctx.lineTo(width, centerY);
      ctx.stroke();
      
      ctx.draw();
    },
    generateMockData() {
      const heartRate = Math.floor(this.baseHeartRate + (Math.random() - 0.5) * 4);
      const systolicBP = Math.floor(this.baseSystolic + (Math.random() - 0.5) * 3);
      const diastolicBP = Math.floor(this.baseDiastolic + (Math.random() - 0.5) * 2);
      const spo2 = Math.floor(this.baseSpO2 + (Math.random() - 0.5) * 1);
      
      return {
        heart_rate: heartRate,
        systolic_bp: systolicBP,
        diastolic_bp: diastolicBP,
        spo2: spo2
      };
    },
    initHealthTips() {
      this.healthTips = [
        { icon: '💧', text: this.t('healthTip1') },
        { icon: '🏃', text: this.t('healthTip2') },
        { icon: '😴', text: this.t('healthTip3') },
        { icon: '🥗', text: this.t('healthTip4') }
      ];
    },
    loadUserInfo() {
      const storedUser = uni.getStorageSync('user');
      if (storedUser) {
        this.user = storedUser;
      } else {
        this.user = { nickname: 'Guest' };
      }
    },
    loadSelectedDevice() {
      const storedDeviceId = uni.getStorageSync('selectedDeviceId');
      if (storedDeviceId) {
        this.selectedDeviceId = storedDeviceId;
        this.fetchRealtimeData();
      }
      uni.request({
        url: getAuthUrl('devices'),
        header: { Authorization: 'Bearer ' + uni.getStorageSync('token') },
        success: (res) => {
          if (res.data.errcode === 0 && Array.isArray(res.data.data) && res.data.data.length > 0) {
            const boundDevice = res.data.data[0];
            const deviceId = boundDevice?.deviceId || boundDevice?.device_id;
            if (deviceId) {
              this.selectedDeviceId = deviceId;
              uni.setStorageSync('selectedDeviceId', deviceId);
              this.fetchRealtimeData();
            }
          }
        },
        fail: () => {
          // ignore, keep stored device id if available
        }
      });
    },
    checkDeviceStatus() {
      if (!this.selectedDeviceId) {
        this.isConnected = false;
        return;
      }
      uni.request({
        url: getDataUrl('deviceStatus'),
        data: { deviceId: this.selectedDeviceId },
        success: (res) => {
          if (res.data.errcode === 0) {
            this.isConnected = res.data.data?.isConnected || false;
          }
        },
        fail: () => {
          this.isConnected = false;
        }
      });
    },
    startRealtimeUpdate() {
      this.timer = setInterval(() => {
        this.fetchRealtimeData();
      }, 60000);
      this.fetchRealtimeData();
    },
    startDeviceStatusCheck() {
      if (this.isSimulationMode) {
        return;
      }
      this.statusTimer = setInterval(() => {
        this.checkDeviceStatus();
      }, 30000); // 每30秒检查一次设备状态
      this.checkDeviceStatus();
    },
    fetchRealtimeData() {
      if (this.isSimulationMode) {
        return;
      }
      if (!this.selectedDeviceId) {
        this.isConnected = false;
        this.hasData = false;
        return;
      }
      uni.request({
        url: getDataUrl('realtime'),
        data: {
          deviceId: this.selectedDeviceId
        },
        success: (res) => {
          if (res.data.errcode === 0) {
            const data = res.data.data;
            if (data && data.heart_rate !== undefined && data.heart_rate !== null) {
              this.realtimeData = {
                heart_rate: data.heart_rate,
                systolic_bp: data.systolic_bp,
                diastolic_bp: data.diastolic_bp,
                spo2: data.spo2
              };
              this.hasData = true;
              this.isConnected = true;

              const now = Date.now();
              const lastEntry = this.miniChartData[this.miniChartData.length - 1];

              if (!lastEntry || (now - lastEntry.timestamp) >= 60000) {
                this.miniChartData.push({
                  heart_rate: this.realtimeData.heart_rate,
                  systolic_bp: this.realtimeData.systolic_bp,
                  diastolic_bp: this.realtimeData.diastolic_bp,
                  spo2: this.realtimeData.spo2,
                  timestamp: now
                });

                if (this.miniChartData.length > 10) {
                  this.miniChartData.shift();
                }
              }
            }
          }
        },
        fail: () => {
          this.isConnected = false;
          this.hasData = false;
        }
      });
    },
    async fetchAIAdvice() {
      if (this.isSimulationMode) {
        this.generateMockAIAdvice();
        return;
      }
      
      if (!this.selectedDeviceId) return;
      
      try {
        const today = new Date().toISOString().split('T')[0];
        const res = await new Promise((resolve, reject) => {
          uni.request({
            url: getDataUrl('aiHealthAdvice'),
            method: 'POST',
            header: {
              'Content-Type': 'application/json',
              Authorization: 'Bearer ' + uni.getStorageSync('token')
            },
            data: {
              deviceId: this.selectedDeviceId,
              date: today
            },
            success: resolve,
            fail: reject
          });
        });

        if (res.data.errcode === 0 && res.data.data) {
          this.aiAdvice = res.data.data.advice;
          this.aiAdviceDate = res.data.data.date;
        }
      } catch (err) {
        console.error('Failed to fetch AI advice:', err);
        this.generateMockAIAdvice();
      }
    },
    getHeartRateStatus(hr) {
      if (!hr) return 'unknown';
      if (hr < 60) return 'low';
      if (hr > 100) return 'high';
      return 'normal';
    },
    getHeartRateText(hr) {
      if (!hr) return '--';
      if (hr < 60) return this.t('bradycardia');
      if (hr > 100) return this.t('tachycardia');
      return this.t('normal');
    },
    getBPStatus(systolic) {
      if (!systolic) return 'unknown';
      if (systolic < 120) return 'normal';
      if (systolic < 140) return 'elevated';
      if (systolic < 160) return 'high';
      return 'critical';
    },
    getBPText(systolic) {
      if (!systolic) return '--';
      if (systolic < 120) return this.t('normal');
      if (systolic < 140) return this.t('elevated');
      if (systolic < 160) return this.t('high');
      return this.t('critical');
    },
    getSpO2Status(spo2) {
      if (!spo2) return 'unknown';
      if (spo2 >= 95) return 'normal';
      if (spo2 >= 90) return 'low';
      return 'critical';
    },
    getSpO2Text(spo2) {
      if (!spo2) return '--';
      if (spo2 >= 95) return this.t('normal');
      if (spo2 >= 90) return this.t('low');
      return this.t('critical');
    },
    getAgeDiffText(status) {
      if (status === 'younger') return 'younger than actual age';
      if (status === 'older') return 'older than actual age';
      return 'matches actual age';
    },
    goToDeviceList() {
      uni.navigateTo({ url: '/pages/device/device' });
    },
    goToHelp() {
      uni.navigateTo({ url: '/pages/help/help' });
    },
    goToSettings() {
      uni.navigateTo({ url: '/pages/settings/settings' });
    },
    goToNotifications() {
      uni.showToast({ title: 'Notifications', icon: 'none' });
    },
    goToHistory() {
      uni.navigateTo({ url: '/pages/health/trend' });
    },
    goToInput() {
      uni.navigateTo({ url: '/pages/health/input' });
    }
  }
};
</script>

<style lang="scss" scoped>
.container {
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
  box-shadow: 0 8rpx 24rpx rgba(99, 102, 241, 0.3);
}

.avatar {
  font-size: 44rpx;
  font-weight: 700;
  color: white;
}

.greeting {
  display: flex;
  flex-direction: column;
  gap: 6rpx;
}

.greeting-text {
  font-size: 36rpx;
  font-weight: 600;
  color: #1e293b;
}

.date-text {
  font-size: 26rpx;
  color: #64748b;
}

.header-actions {
  display: flex;
  gap: 20rpx;
}

.action-btn {
  width: 80rpx;
  height: 80rpx;
  border-radius: 50%;
  background: white;
  display: flex;
  align-items: center;
  justify-content: center;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.08);
  position: relative;
}

.action-icon {
  font-size: 36rpx;
}

.badge {
  position: absolute;
  top: -4rpx;
  right: -4rpx;
  min-width: 36rpx;
  height: 36rpx;
  border-radius: 18rpx;
  background: #ef4444;
  color: white;
  font-size: 22rpx;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 0 8rpx;
}

.status-section {
  margin-bottom: 30rpx;
}

.connection-status {
  display: flex;
  align-items: center;
  gap: 12rpx;
  padding: 20rpx 28rpx;
  border-radius: 20rpx;
}

.connection-status.connected {
  background: linear-gradient(135deg, #dcfce7 0%, #bbf7d0 100%);
}

.connection-status.disconnected {
  background: linear-gradient(135deg, #fef3c7 0%, #fde68a 100%);
}

.status-dot {
  width: 16rpx;
  height: 16rpx;
  border-radius: 50%;
}

.connected .status-dot {
  background: #22c55e;
  box-shadow: 0 0 12rpx #22c55e;
}

.disconnected .status-dot {
  background: #f59e0b;
}

.status-text {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
}

.status-hint {
  font-size: 24rpx;
  color: #64748b;
  margin-left: auto;
}

.main-content {
  margin-bottom: 30rpx;
}

.realtime-section {
  background: white;
  border-radius: 24rpx;
  padding: 28rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 24rpx;
}

.section-title {
  font-size: 32rpx;
  font-weight: 600;
  color: #1e293b;
}

.live-badge {
  display: flex;
  align-items: center;
  gap: 8rpx;
  padding: 8rpx 16rpx;
  background: #fef2f2;
  border-radius: 20rpx;
}

.live-dot {
  width: 12rpx;
  height: 12rpx;
  border-radius: 50%;
  background: #ef4444;
  animation: pulse 1.5s infinite;
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.4; }
}

.live-badge text {
  font-size: 22rpx;
  color: #ef4444;
  font-weight: 500;
}

.metrics-vertical {
  display: flex;
  flex-direction: column;
  gap: 16rpx;
}

.metric-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 24rpx;
  border-radius: 16rpx;
  background: #f8fafc;
}

.metric-row.heart-rate {
  background: linear-gradient(135deg, #fef2f2 0%, #fee2e2 100%);
}

.metric-row.blood-pressure {
  background: linear-gradient(135deg, #fef3c7 0%, #fde68a 100%);
}

.metric-row.spo2 {
  background: linear-gradient(135deg, #f0f9ff 0%, #e0f2fe 100%);
}

.metric-left {
  display: flex;
  align-items: center;
  gap: 16rpx;
}

.metric-icon {
  font-size: 40rpx;
}

.metric-label-group {
  display: flex;
  flex-direction: column;
  gap: 4rpx;
}

.metric-label {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
}

.metric-status {
  font-size: 22rpx;
  padding: 2rpx 12rpx;
  border-radius: 8rpx;
  width: fit-content;
}

.metric-status.normal {
  background: #dcfce7;
  color: #16a34a;
}

.metric-status.low, .metric-status.high {
  background: #fef3c7;
  color: #d97706;
}

.metric-status.elevated {
  background: #ffedd5;
  color: #ea580c;
}

.metric-status.critical {
  background: #fef2f2;
  color: #dc2626;
}

.metric-status.unknown {
  background: #f1f5f9;
  color: #64748b;
}

.metric-right {
  display: flex;
  flex-direction: column;
  align-items: flex-end;
  gap: 2rpx;
}

.metric-value {
  font-size: 40rpx;
  font-weight: 700;
  color: #1e293b;
}

.metric-unit {
  font-size: 22rpx;
  color: #64748b;
}

.mini-chart-section {
  background: white;
  border-radius: 24rpx;
  padding: 28rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.mini-chart {
  height: 200rpx;
  display: flex;
  flex-direction: column;
}

.chart-items {
  flex: 1;
  display: flex;
  align-items: flex-end;
  justify-content: space-between;
  gap: 8rpx;
  padding: 0 10rpx;
}

.chart-bar {
  flex: 1;
  background: linear-gradient(180deg, #ef4444 0%, #f87171 100%);
  border-radius: 8rpx 8rpx 0 0;
  min-height: 20rpx;
  transition: height 0.3s ease;
}

.chart-labels {
  display: flex;
  justify-content: space-between;
  padding: 12rpx 10rpx 0;
  font-size: 20rpx;
  color: #94a3b8;
}

.ai-advice-section {
  background: white;
  border-radius: 24rpx;
  padding: 28rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.ai-advice-card {
  background: linear-gradient(135deg, #f5f3ff 0%, #ede9fe 100%);
  border-radius: 16rpx;
  padding: 24rpx;
}

.ai-advice-text {
  font-size: 26rpx;
  color: #475569;
  line-height: 1.6;
}

.empty-state {
  display: flex;
  flex-direction: column;
  gap: 32rpx;
}

.empty-illustration {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 60rpx 0;
  position: relative;
}

.device-icon, .sensor-icon {
  font-size: 80rpx;
}

.connection-lines {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  display: flex;
  gap: 60rpx;
}

.line {
  width: 60rpx;
  height: 4rpx;
  background: linear-gradient(90deg, #6366f1, #8b5cf6);
  border-radius: 2rpx;
  animation: lineMove 2s infinite;
}

.line1 { animation-delay: 0s; }
.line2 { animation-delay: 0.3s; }
.line3 { animation-delay: 0.6s; }

@keyframes lineMove {
  0%, 100% { opacity: 0.3; transform: scaleX(0.5); }
  50% { opacity: 1; transform: scaleX(1); }
}

.empty-content {
  text-align: center;
  padding: 0 40rpx;
}

.empty-title {
  font-size: 36rpx;
  font-weight: 600;
  color: #1e293b;
  margin-bottom: 16rpx;
  display: block;
}

.empty-desc {
  font-size: 28rpx;
  color: #64748b;
  line-height: 1.6;
  display: block;
  margin-bottom: 32rpx;
}

.empty-actions {
  display: flex;
  gap: 24rpx;
  justify-content: center;
}

.action-card {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 12rpx;
  padding: 32rpx 40rpx;
  background: white;
  border-radius: 20rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.08);
  min-width: 200rpx;
}

.action-card .action-icon {
  font-size: 48rpx;
}

.action-card .action-text {
  font-size: 26rpx;
  color: #64748b;
}

.health-tips {
  background: white;
  border-radius: 24rpx;
  padding: 28rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.tips-title {
  font-size: 30rpx;
  font-weight: 600;
  color: #1e293b;
  margin-bottom: 20rpx;
  display: block;
}

.tips-list {
  display: flex;
  flex-direction: column;
  gap: 16rpx;
}

.tip-item {
  display: flex;
  align-items: center;
  gap: 16rpx;
  padding: 16rpx 20rpx;
  background: #f8fafc;
  border-radius: 12rpx;
}

.tip-icon {
  font-size: 32rpx;
}

.tip-text {
  font-size: 26rpx;
  color: #475569;
}

.quick-guide {
  background: white;
  border-radius: 24rpx;
  padding: 28rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.guide-title {
  font-size: 30rpx;
  font-weight: 600;
  color: #1e293b;
  margin-bottom: 20rpx;
  display: block;
}

.guide-steps {
  display: flex;
  flex-direction: column;
  gap: 16rpx;
}

.guide-step {
  display: flex;
  align-items: center;
  gap: 16rpx;
}

.step-number {
  width: 48rpx;
  height: 48rpx;
  border-radius: 50%;
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
  color: white;
  font-size: 26rpx;
  font-weight: 600;
  display: flex;
  align-items: center;
  justify-content: center;
}

.step-text {
  font-size: 26rpx;
  color: #475569;
}

.ai-advice-section {
  background: white;
  border-radius: 24rpx;
  padding: 28rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.advice-date {
  font-size: 24rpx;
  color: #94a3b8;
}

.age-card {
  background: linear-gradient(135deg, #f5f3ff 0%, #ede9fe 100%);
  border-radius: 16rpx;
  padding: 24rpx;
  margin-bottom: 24rpx;
  border-left: 6rpx solid #8b5cf6;
}

.age-header {
  display: flex;
  align-items: center;
  gap: 12rpx;
  margin-bottom: 20rpx;
}

.age-icon {
  font-size: 36rpx;
}

.age-title {
  font-size: 30rpx;
  font-weight: 600;
  color: #1e293b;
}

.age-content {
  display: flex;
  flex-direction: column;
  gap: 16rpx;
}

.age-comparison {
  display: flex;
  align-items: center;
  justify-content: space-around;
  background: white;
  border-radius: 12rpx;
  padding: 20rpx;
}

.age-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8rpx;
}

.age-label {
  font-size: 24rpx;
  color: #64748b;
}

.age-value {
  font-size: 36rpx;
  font-weight: 700;
  color: #1e293b;
}

.age-value.younger {
  color: #22c55e;
}

.age-value.older {
  color: #ef4444;
}

.age-arrow {
  font-size: 32rpx;
  color: #94a3b8;
}

.age-difference {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12rpx;
}

.diff-badge {
  padding: 6rpx 20rpx;
  border-radius: 20rpx;
  font-size: 26rpx;
  font-weight: 600;
}

.age-difference.younger .diff-badge {
  background: #dcfce7;
  color: #16a34a;
}

.age-difference.older .diff-badge {
  background: #fee2e2;
  color: #dc2626;
}

.age-difference.same .diff-badge {
  background: #e0f2fe;
  color: #0284c7;
}

.diff-text {
  font-size: 24rpx;
  color: #64748b;
}

.age-explanation {
  font-size: 26rpx;
  color: #475569;
  line-height: 1.6;
  padding: 0 8rpx;
}

.ai-summary-card {
  background: linear-gradient(135deg, #f0f9ff 0%, #e0f2fe 100%);
  border-radius: 16rpx;
  padding: 24rpx;
  margin-bottom: 24rpx;
  border-left: 6rpx solid #0ea5e9;
}

.ai-summary-text {
  font-size: 28rpx;
  color: #1e293b;
  line-height: 1.6;
  font-weight: 500;
}

.indicators-grid {
  display: flex;
  flex-direction: column;
  gap: 16rpx;
  margin-bottom: 24rpx;
}

.indicator-card {
  background: #f8fafc;
  border-radius: 16rpx;
  padding: 20rpx;
  border-left: 6rpx solid #e2e8f0;
}

.indicator-card.normal {
  background: linear-gradient(135deg, #f0fdf4 0%, #dcfce7 100%);
  border-left-color: #22c55e;
}

.indicator-card.warning {
  background: linear-gradient(135deg, #fffbeb 0%, #fef3c7 100%);
  border-left-color: #f59e0b;
}

.indicator-card.danger {
  background: linear-gradient(135deg, #fef2f2 0%, #fee2e2 100%);
  border-left-color: #ef4444;
}

.indicator-header {
  display: flex;
  align-items: center;
  gap: 12rpx;
  margin-bottom: 12rpx;
}

.indicator-icon {
  font-size: 36rpx;
}

.indicator-info {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 4rpx;
}

.indicator-name {
  font-size: 26rpx;
  font-weight: 600;
  color: #1e293b;
}

.indicator-value {
  font-size: 24rpx;
  color: #64748b;
}

.indicator-badge {
  padding: 6rpx 16rpx;
  border-radius: 20rpx;
  font-size: 22rpx;
  font-weight: 500;
}

.indicator-badge.normal {
  background: #dcfce7;
  color: #16a34a;
}

.indicator-badge.warning {
  background: #fef3c7;
  color: #d97706;
}

.indicator-badge.danger {
  background: #fee2e2;
  color: #dc2626;
}

.badge-text {
  font-size: 22rpx;
}

.indicator-advice {
  font-size: 26rpx;
  color: #475569;
  line-height: 1.5;
  padding-left: 48rpx;
}

.lifestyle-section {
  margin-bottom: 24rpx;
}

.lifestyle-title {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
  margin-bottom: 16rpx;
  display: block;
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
  gap: 10rpx;
  padding-left: 44rpx;
}

.tip-item {
  display: flex;
  align-items: flex-start;
  gap: 12rpx;
}

.tip-dot {
  width: 10rpx;
  height: 10rpx;
  border-radius: 50%;
  background: #6366f1;
  margin-top: 10rpx;
  flex-shrink: 0;
}

.tip-text {
  font-size: 26rpx;
  color: #475569;
  line-height: 1.5;
}

.overall-advice-card {
  background: linear-gradient(135deg, #f5f3ff 0%, #ede9fe 100%);
  border-radius: 16rpx;
  padding: 24rpx;
  border-left: 6rpx solid #8b5cf6;
}

.overall-header {
  display: flex;
  align-items: center;
  gap: 12rpx;
  margin-bottom: 12rpx;
}

.overall-icon {
  font-size: 32rpx;
}

.overall-title {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
}

.overall-text {
  font-size: 26rpx;
  color: #475569;
  line-height: 1.6;
  padding-left: 44rpx;
}

.demo-control {
  margin-top: 20rpx;
  display: flex;
  flex-direction: column;
  gap: 16rpx;
}

.mode-toggle {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12rpx;
}

.toggle-label {
  font-size: 22rpx;
  font-weight: 500;
  color: #64748b;
}

.toggle-switch {
  width: 80rpx;
  height: 44rpx;
  background: #cbd5e1;
  border-radius: 22rpx;
  position: relative;
  transition: background 0.3s;
}

.toggle-switch.active {
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
}

.toggle-thumb {
  width: 36rpx;
  height: 36rpx;
  background: white;
  border-radius: 50%;
  position: absolute;
  top: 4rpx;
  left: 4rpx;
  transition: transform 0.3s;
  box-shadow: 0 2rpx 8rpx rgba(0, 0, 0, 0.15);
}

.toggle-switch.active .toggle-thumb {
  transform: translateX(36rpx);
}

.btn-row {
  display: flex;
  flex-direction: row;
  gap: 16rpx;
}

.demo-btn {
  flex: 1;
  height: 88rpx;
  border-radius: 20rpx;
  font-size: 32rpx;
  font-weight: 600;
  border: none;
}

.demo-btn.connect {
  background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%);
  color: white;
  box-shadow: 0 8rpx 24rpx rgba(245, 158, 11, 0.3);
}

.demo-btn.start {
  background: linear-gradient(135deg, #10b981 0%, #059669 100%);
  color: white;
  box-shadow: 0 8rpx 24rpx rgba(16, 185, 129, 0.3);
}

.demo-btn.start[disabled] {
  background: #94a3b8;
  color: #cbd5e1;
  box-shadow: none;
}

.demo-btn.refresh {
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
  color: white;
  box-shadow: 0 8rpx 24rpx rgba(99, 102, 241, 0.3);
}

.demo-btn.refresh[disabled] {
  background: #94a3b8;
  color: #cbd5e1;
  box-shadow: none;
}

.demo-btn.server {
  background: linear-gradient(135deg, #06b6d4 0%, #0891b2 100%);
  color: white;
  box-shadow: 0 8rpx 24rpx rgba(6, 182, 212, 0.3);
}

.waveform-section {
  margin-top: 30rpx;
  background: white;
  border-radius: 24rpx;
  padding: 30rpx;
  box-shadow: 0 8rpx 32rpx rgba(0, 0, 0, 0.08);
}

.waveform-container {
  margin-top: 20rpx;
  background: #0f172a;
  border-radius: 16rpx;
  padding: 20rpx;
  display: flex;
  justify-content: center;
}

.waveform-canvas {
  display: block;
}
</style>