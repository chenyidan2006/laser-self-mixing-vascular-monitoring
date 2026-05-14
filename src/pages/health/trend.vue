<template>
  <view class="trend-container">
    <view class="nav-bar">
      <view class="nav-back" @click="goBack">
        <text class="back-icon">‹</text>
        <text class="back-text">Back</text>
      </view>
      <text class="nav-title">History Trends</text>
      <view class="nav-placeholder"></view>
    </view>

    <view class="range-tabs">
      <view
        class="range-tab"
        :class="{ active: currentRange === 'day' }"
        @click="switchRange('day')"
      >Day</view>
      <view
        class="range-tab"
        :class="{ active: currentRange === 'week' }"
        @click="switchRange('week')"
      >Week</view>
      <view
        class="range-tab"
        :class="{ active: currentRange === 'month' }"
        @click="switchRange('month')"
      >Month</view>
    </view>

    <view class="metric-tabs">
      <view
        class="metric-tab"
        :class="{ active: currentMetric === 'heart_rate' }"
        @click="switchMetric('heart_rate')"
      >
        <text class="metric-icon">❤️</text>
        <text>HR</text>
      </view>
      <view
        class="metric-tab"
        :class="{ active: currentMetric === 'systolic_bp' }"
        @click="switchMetric('systolic_bp')"
      >
        <text class="metric-icon">🩸</text>
        <text>SYS</text>
      </view>
      <view
        class="metric-tab"
        :class="{ active: currentMetric === 'diastolic_bp' }"
        @click="switchMetric('diastolic_bp')"
      >
        <text class="metric-icon">💓</text>
        <text>DIA</text>
      </view>
      <view
        class="metric-tab"
        :class="{ active: currentMetric === 'spo2' }"
        @click="switchMetric('spo2')"
      >
        <text class="metric-icon">🫁</text>
        <text>SpO2</text>
      </view>
    </view>

    <view class="stats-card">
      <view class="stat-item">
        <text class="stat-label">Min</text>
        <text class="stat-value">{{ stats.min }}</text>
        <text class="stat-unit">{{ metricUnit }}</text>
      </view>
      <view class="stat-item main">
        <text class="stat-label">Avg</text>
        <text class="stat-value">{{ stats.avg }}</text>
        <text class="stat-unit">{{ metricUnit }}</text>
      </view>
      <view class="stat-item">
        <text class="stat-label">Max</text>
        <text class="stat-value">{{ stats.max }}</text>
        <text class="stat-unit">{{ metricUnit }}</text>
      </view>
    </view>

    <view class="chart-card" v-if="currentRange !== 'month'">
      <view class="chart-header">
        <text class="chart-title">{{ metricName }} Trend</text>
        <view class="chart-badge" :style="{ background: metricColor }">
          <text class="badge-text">{{ metricUnit }}</text>
        </view>
      </view>
      <canvas
        type="2d"
        id="trendChart"
        class="chart-canvas"
      ></canvas>
      <view class="chart-note" v-if="chartData.length === 0">
        <view class="empty-icon">📊</view>
        <text>No data available</text>
      </view>
    </view>

    <view class="calendar-section" v-if="currentRange === 'month'">
      <view class="section-header">
        <text class="section-title">Daily Averages - {{ metricName }}</text>
        <view class="chart-badge" :style="{ background: metricColor }">
          <text class="badge-text">{{ metricUnit }}</text>
        </view>
      </view>
      <view class="calendar-grid">
        <view class="calendar-weekdays">
          <text v-for="day in ['S','M','T','W','T','F','S']" :key="day" class="weekday">{{ day }}</text>
        </view>
        <view class="calendar-days">
          <view
            v-for="(day, index) in calendarDays"
            :key="index"
            class="calendar-day"
            :class="{ 'empty': !day.value, 'has-data': day.value }"
            :style="day.value ? { background: getHeatmapColor(day.value) } : {}"
          >
            <text class="day-number">{{ day.day }}</text>
            <text class="day-value" v-if="day.value">{{ Math.round(day.value) }}</text>
          </view>
        </view>
      </view>
      <view class="calendar-legend">
        <text class="legend-label">Low</text>
        <view class="legend-gradient" :style="{ background: metricGradient }"></view>
        <text class="legend-label">High</text>
      </view>
    </view>

    <view class="bar-chart-section" v-if="currentRange !== 'month'">
      <view class="section-header">
        <text class="section-title">Daily Averages</text>
      </view>
      <scroll-view scroll-x class="bar-chart-scroll">
        <view class="bar-chart">
          <view
            v-for="(item, index) in aggregatedData"
            :key="index"
            class="bar-item"
          >
            <view class="bar-wrapper">
              <view
                class="bar-fill"
                :style="{
                  height: getBarHeight(item.value) + '%',
                  background: metricGradient
                }"
              ></view>
            </view>
            <text class="bar-label">{{ formatBarLabel(item.date) }}</text>
            <text class="bar-value">{{ Math.round(item.value) }}</text>
          </view>
        </view>
      </scroll-view>
    </view>

    <view class="records-card">
      <view class="card-header">
        <text class="card-title">Recent Records</text>
        <text class="card-count">{{ recordsList.length }} records</text>
      </view>
      <scroll-view scroll-y class="records-list">
        <view v-if="recordsList.length === 0" class="empty-records">
          <view class="empty-icon">📝</view>
          <text>No recent records</text>
        </view>
        <view
          v-for="(item, index) in recordsList"
          :key="index"
          class="record-item"
        >
          <view class="record-date">{{ formatDate(item.measure_time) }}</view>
          <view class="record-values">
            <text class="record-value">❤️ {{ item.heart_rate || '--' }} bpm</text>
            <text class="record-value">🩸 {{ item.systolic_bp || '--' }}/{{ item.diastolic_bp || '--' }} mmHg</text>
            <text class="record-value">🫁 {{ item.spo2 || '--' }}%</text>
          </view>
        </view>
      </scroll-view>
    </view>
  </view>
</template>

<script>
import uCharts from '@/uni_modules/qiun-data-charts/js_sdk/u-charts/u-charts.min.js'
import { getDataUrl, getAuthUrl } from '@/config/api.js'

export default {
  data() {
    return {
      selectedDeviceId: '',
      currentRange: 'week',
      currentMetric: 'heart_rate',
      metricConfig: {
        heart_rate: {
          name: 'Heart Rate',
          unit: 'bpm',
          color: '#ef4444',
          gradient: 'linear-gradient(180deg, #ef4444 0%, #f87171 100%)',
          min: 40,
          max: 120
        },
        systolic_bp: {
          name: 'Systolic BP',
          unit: 'mmHg',
          color: '#22c55e',
          gradient: 'linear-gradient(180deg, #22c55e 0%, #4ade80 100%)',
          min: 90,
          max: 160
        },
        diastolic_bp: {
          name: 'Diastolic BP',
          unit: 'mmHg',
          color: '#14b8a6',
          gradient: 'linear-gradient(180deg, #14b8a6 0%, #2dd4bf 100%)',
          min: 50,
          max: 100
        },
        spo2: {
          name: 'SpO2',
          unit: '%',
          color: '#06b6d4',
          gradient: 'linear-gradient(180deg, #06b6d4 0%, #22d3ee 100%)',
          min: 94,
          max: 100
        }
      },
      chartData: [],
      aggregatedData: [],
      calendarDays: [],
      recordsList: [],
      chartInstance: null,
      stats: { min: '--', avg: '--', max: '--' }
    };
  },
  computed: {
    metricName() {
      return this.metricConfig[this.currentMetric]?.name || 'Metric';
    },
    metricUnit() {
      return this.metricConfig[this.currentMetric]?.unit || '';
    },
    metricColor() {
      return this.metricConfig[this.currentMetric]?.color || '#4361ee';
    },
    metricGradient() {
      return this.metricConfig[this.currentMetric]?.gradient || '';
    },
    metricMin() {
      return this.metricConfig[this.currentMetric]?.min || 0;
    },
    metricMax() {
      return this.metricConfig[this.currentMetric]?.max || 100;
    }
  },
  onLoad() {
    this.loadSelectedDevice();
    this.loadData();
  },
  onUnload() {
    if (this.chartInstance) {
      this.chartInstance.destroy();
    }
  },
  methods: {
    goBack() {
      uni.navigateBack();
    },
    switchRange(range) {
      this.currentRange = range;
      this.loadData();
    },
    switchMetric(metric) {
      this.currentMetric = metric;
      this.updateStats();
      if (this.currentRange !== 'month') {
        this.drawChart();
      }
    },
    getHeatmapColor(value) {
      const range = this.metricMax - this.metricMin;
      const percent = Math.min(1, Math.max(0, (value - this.metricMin) / range));

      const colors = {
        heart_rate: ['#fef2f2', '#ef4444'],
        systolic_bp: ['#f0fdf4', '#22c55e'],
        diastolic_bp: ['#f0fdfa', '#14b8a6'],
        spo2: ['#ecfeff', '#06b6d4']
      };

      const [low, high] = colors[this.currentMetric] || ['#f1f5f9', '#4361ee'];

      if (percent < 0.5) {
        return this.lerpColor(low, high, percent * 2);
      }
      return high;
    },
    lerpColor(color1, color2, t) {
      const c1 = this.hexToRgb(color1);
      const c2 = this.hexToRgb(color2);
      const r = Math.round(c1.r + (c2.r - c1.r) * t);
      const g = Math.round(c1.g + (c2.g - c1.g) * t);
      const b = Math.round(c1.b + (c2.b - c1.b) * t);
      return `rgb(${r}, ${g}, ${b})`;
    },
    hexToRgb(hex) {
      const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
      return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
      } : { r: 241, g: 245, b: 249 };
    },
    buildCalendarDays(data) {
      const now = new Date();
      const year = now.getFullYear();
      const month = now.getMonth();
      const firstDay = new Date(year, month, 1).getDay();
      const daysInMonth = new Date(year, month + 1, 0).getDate();

      const dataMap = {};
      data.forEach(item => {
        const date = new Date(item.date);
        const day = date.getDate();
        dataMap[day] = item.value;
      });

      const days = [];

      for (let i = 0; i < firstDay; i++) {
        days.push({ day: '', value: null });
      }

      for (let d = 1; d <= daysInMonth; d++) {
        days.push({
          day: d,
          value: dataMap[d] || null
        });
      }

      return days;
    },
    getBarHeight(value) {
      if (!value || value <= 0) return 0;
      const range = this.metricMax - this.metricMin;
      return Math.min(100, Math.max(5, ((value - this.metricMin) / range) * 100));
    },
    formatBarLabel(dateStr) {
      if (!dateStr) return '';
      const date = new Date(dateStr);
      if (this.currentRange === 'day') {
        const h = date.getHours();
        return `${h}:00`;
      } else if (this.currentRange === 'week') {
        const weekdays = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
        return weekdays[date.getDay()];
      } else {
        return `${date.getMonth() + 1}/${date.getDate()}`;
      }
    },
    formatDate(dateStr) {
      if (!dateStr) return '--';
      
      let date;
      if (typeof dateStr === 'string') {
        // 处理 YYYY-MM-DD HH:mm:ss 格式
        if (dateStr.includes(' ')) {
          const parts = dateStr.split(' ');
          const dateParts = parts[0].split('-');
          const timeParts = parts[1].split(':');
          date = new Date(
            parseInt(dateParts[0]),
            parseInt(dateParts[1]) - 1,
            parseInt(dateParts[2]),
            parseInt(timeParts[0]),
            parseInt(timeParts[1]),
            parseInt(timeParts[2])
          );
        } else if (dateStr.includes('-')) {
          // 处理 YYYY-MM-DD 格式
          const parts = dateStr.split('-');
          return `${parseInt(parts[1])}/${parseInt(parts[2])}`;
        } else {
          date = new Date(dateStr);
        }
      } else {
        date = new Date(dateStr);
      }
      
      if (isNaN(date.getTime())) return String(dateStr);
      return `${date.getMonth() + 1}/${date.getDate()} ${date.getHours()}:${String(date.getMinutes()).padStart(2, '0')}`;
    },
    formatYMD(date) {
      return `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-${String(date.getDate()).padStart(2, '0')}`;
    },
    loadSelectedDevice() {
      const storedDeviceId = uni.getStorageSync('selectedDeviceId');
      if (storedDeviceId) {
        this.selectedDeviceId = storedDeviceId;
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
            }
          }
        },
        fail: () => {
          // ignore, will use stored device id if available
        }
      });
    },
    async loadData() {
      try {
        const now = new Date();
        let startDate, endDate;
        if (this.currentRange === 'day') {
          startDate = this.formatYMD(now);
          endDate = startDate;
        } else if (this.currentRange === 'week') {
          const oneWeekAgo = new Date(now);
          oneWeekAgo.setDate(now.getDate() - 6);
          startDate = this.formatYMD(oneWeekAgo);
          endDate = this.formatYMD(now);
        } else {
          const oneMonthAgo = new Date(now);
          oneMonthAgo.setMonth(now.getMonth() - 1);
          startDate = this.formatYMD(oneMonthAgo);
          endDate = this.formatYMD(now);
        }

        const trendRes = await this.requestTrendData(startDate, endDate);
        this.chartData = this.filterDataByMetric(trendRes);
        this.aggregatedData = this.aggregateData(trendRes);
        this.updateStats();

        if (this.currentRange === 'month') {
          this.calendarDays = this.buildCalendarDays(this.aggregatedData);
        } else {
          this.drawChart();
        }

        const recordsRes = await this.requestRecordsList(startDate, endDate);
        this.recordsList = recordsRes.slice(0, 20);
      } catch (err) {
        console.error('Failed to load history data', err);
        uni.showToast({ title: 'Load failed', icon: 'none' });
      }
    },
    filterDataByMetric(data) {
      return data.map(item => ({
        date: item.date || (item.measure_time ? item.measure_time.split('T')[0] : ''),
        value: item[this.currentMetric] !== undefined ? item[this.currentMetric] : item.value
      })).filter(item => item.value !== undefined && item.value !== null && item.value !== '');
    },
    aggregateData(data) {
      if (!data.length) return [];

      const grouped = {};
      data.forEach(item => {
        const dateKey = item.date || (item.measure_time ? item.measure_time.split('T')[0] : new Date().toISOString().split('T')[0]);
        if (!grouped[dateKey]) {
          grouped[dateKey] = [];
        }
        const value = item[this.currentMetric] !== undefined ? item[this.currentMetric] : item.value;
        if (value !== undefined && value !== null) {
          grouped[dateKey].push(Number(value));
        }
      });

      return Object.keys(grouped).map(date => ({
        date,
        value: grouped[date].reduce((a, b) => a + b, 0) / grouped[date].length
      })).sort((a, b) => new Date(a.date) - new Date(b.date));
    },
    updateStats() {
      const values = this.aggregatedData.map(item => item.value).filter(v => v > 0);

      if (values.length) {
        this.stats = {
          min: Math.round(Math.min(...values)),
          avg: Math.round(values.reduce((a, b) => a + b, 0) / values.length),
          max: Math.round(Math.max(...values))
        };
      } else {
        this.stats = { min: '--', avg: '--', max: '--' };
      }
    },
    async requestTrendData(startDate, endDate) {
      const isSimulationMode = uni.getStorageSync('isSimulationMode') === 'true' || false;
      
      if (isSimulationMode) {
        const mockDataStr = uni.getStorageSync('mockTrendData');
        if (mockDataStr) {
          try {
            const allData = JSON.parse(mockDataStr);
            const filtered = allData.filter(item => {
              const itemDate = item.date || (item.measure_time ? item.measure_time.split(' ')[0] : '');
              return itemDate >= startDate && itemDate <= endDate;
            });
            return Promise.resolve(filtered);
          } catch (e) {
            return Promise.resolve([]);
          }
        }
        return Promise.resolve([]);
      }
      
      const deviceId = this.selectedDeviceId || uni.getStorageSync('selectedDeviceId');
      return new Promise((resolve, reject) => {
        uni.request({
          url: getDataUrl('historyTrend'),
          data: {
            deviceId,
            startDate,
            endDate,
            metric: this.currentMetric
          },
          success: (res) => {
            if (res.data.errcode === 0) {
              resolve(res.data.data || []);
            } else {
              reject(new Error(res.data.msg));
            }
          },
          fail: reject
        });
      });
    },
    async requestRecordsList(startDate, endDate) {
      const isSimulationMode = uni.getStorageSync('isSimulationMode') === 'true' || false;
      
      if (isSimulationMode) {
        const mockDataStr = uni.getStorageSync('mockTrendData');
        if (mockDataStr) {
          try {
            const allData = JSON.parse(mockDataStr);
            const filtered = allData.filter(item => {
              const itemDate = item.date || (item.measure_time ? item.measure_time.split(' ')[0] : '');
              return itemDate >= startDate && itemDate <= endDate;
            });
            const formatted = filtered.map(item => ({
              heart_rate: item.heart_rate,
              systolic_bp: item.systolic_bp,
              diastolic_bp: item.diastolic_bp,
              spo2: item.spo2,
              measure_time: item.measure_time
            }));
            return Promise.resolve(formatted);
          } catch (e) {
            return Promise.resolve([]);
          }
        }
        return Promise.resolve([]);
      }
      
      const deviceId = this.selectedDeviceId || uni.getStorageSync('selectedDeviceId');
      return new Promise((resolve, reject) => {
        uni.request({
          url: getDataUrl('rangeRecords'),
          data: {
            deviceId,
            startDate,
            endDate
          },
          success: (res) => {
            if (res.data.errcode === 0) {
              const rawData = res.data.data || [];
              const formatted = rawData.map(item => ({
                heart_rate: item.heart_rate,
                systolic_bp: item.systolic_bp,
                diastolic_bp: item.diastolic_bp,
                spo2: item.spo2,
                measure_time: item.measure_time
              }));
              resolve(formatted);
            } else {
              reject(new Error(res.data.msg));
            }
          },
          fail: reject
        });
      });
    },
    drawChart() {
      if (!this.chartData || !this.chartData.length) {
        this.chartInstance = null;
        return;
      }

      const query = uni.createSelectorQuery().in(this);
      query.select('#trendChart').fields({ node: true, size: true }).exec((res) => {
        if (!res[0] || !res[0].node) return;

        const canvas = res[0].node;
        const ctx = canvas.getContext('2d');
        const width = res[0].width;
        const height = res[0].height;

        const config = this.metricConfig[this.currentMetric];
        const displayData = this.currentRange === 'day'
          ? this.chartData.slice(-12)
          : this.aggregatedData;

        if (!displayData || !displayData.length) {
          return;
        }

        const validData = displayData.filter(item => {
          const val = Number(item.value);
          return !isNaN(val) && item.date;
        });

        if (!validData.length) {
          return;
        }

        const categories = validData.map(item => this.formatBarLabel(item.date)).filter(label => label && label.length > 0);
        const seriesData = validData.map(item => ({
          value: Number(item.value)
        }));

        // Ensure arrays have the same length
        const minLength = Math.min(categories.length, seriesData.length);
        const finalCategories = categories.slice(0, minLength);
        const finalSeriesData = seriesData.slice(0, minLength);

        if (!finalCategories.length || !finalSeriesData.length) {
          return;
        }

        console.log('drawChart - finalCategories:', finalCategories);
        console.log('drawChart - finalSeriesData:', finalSeriesData);

        try {
          this.chartInstance = new uCharts({
            $this: this,
            canvasId: 'trendChart',
            context: ctx,
            type: 'line',
            animation: false,
            categories: finalCategories,
            series: [{
              name: this.metricName,
              data: finalSeriesData,
              color: config.color
            }],
            width: width,
            height: height,
            padding: [20, 15, 30, 40],
            background: '#fff',
            enableScroll: false,
            enableMarkLine: false,
            dataLabel: false,
            dataPoint: false,
            lineType: 'straight',
            touchMoveLimit: 24,
            tooltip: {
              show: false
            },
            xAxis: {
              disableGrid: true,
              labelCount: Math.min(6, finalCategories.length),
              fontColor: '#64748b',
              fontSize: 10,
              rotateLabel: this.currentRange === 'month'
            },
            yAxis: {
              gridType: 'dash',
              dashLength: 4,
              gridColor: '#e2e8f0',
              fontColor: '#64748b',
              fontSize: 10,
              min: config.min,
              max: config.max
            },
            legend: { show: false },
            tooltip: { show: true }
          });
        } catch (err) {
          console.error('drawChart error:', err);
        }
      });
    },
    hexToRgba(hex, alpha) {
      const r = parseInt(hex.slice(1, 3), 16);
      const g = parseInt(hex.slice(3, 5), 16);
      const b = parseInt(hex.slice(5, 7), 16);
      return `rgba(${r}, ${g}, ${b}, ${alpha})`;
    }
  }
};
</script>

<style lang="scss" scoped>
.trend-container {
  min-height: 100vh;
  background: #f8fafc;
  padding: 0 24rpx 120rpx;
}

.nav-bar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 60rpx 0 20rpx;
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

.range-tabs {
  display: flex;
  gap: 16rpx;
  margin-bottom: 24rpx;
}

.range-tab {
  flex: 1;
  padding: 16rpx;
  text-align: center;
  background: white;
  border-radius: 16rpx;
  font-size: 28rpx;
  color: #64748b;
  box-shadow: 0 2rpx 8rpx rgba(0, 0, 0, 0.04);
}

.range-tab.active {
  background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%);
  color: white;
}

.metric-tabs {
  display: flex;
  gap: 12rpx;
  margin-bottom: 24rpx;
}

.metric-tab {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8rpx;
  padding: 16rpx 8rpx;
  background: white;
  border-radius: 16rpx;
  box-shadow: 0 2rpx 8rpx rgba(0, 0, 0, 0.04);
}

.metric-tab.active {
  border: 2rpx solid #6366f1;
}

.metric-icon {
  font-size: 32rpx;
}

.metric-tab text:not(.metric-icon) {
  font-size: 22rpx;
  color: #64748b;
}

.metric-tab.active text:not(.metric-icon) {
  color: #6366f1;
  font-weight: 600;
}

.stats-card {
  display: flex;
  background: white;
  border-radius: 20rpx;
  padding: 24rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.04);
}

.stat-item {
  flex: 1;
  text-align: center;
}

.stat-item.main {
  border-left: 1rpx solid #e2e8f0;
  border-right: 1rpx solid #e2e8f0;
}

.stat-label {
  font-size: 24rpx;
  color: #94a3b8;
  display: block;
  margin-bottom: 8rpx;
}

.stat-value {
  font-size: 40rpx;
  font-weight: 700;
  color: #1e293b;
  display: block;
}

.stat-unit {
  font-size: 20rpx;
  color: #94a3b8;
}

.chart-card {
  background: white;
  border-radius: 20rpx;
  padding: 24rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.04);
}

.chart-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 16rpx;
}

.chart-title {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
}

.chart-badge {
  padding: 6rpx 16rpx;
  border-radius: 12rpx;
}

.badge-text {
  font-size: 22rpx;
  color: white;
}

.chart-canvas {
  width: 100%;
  height: 300rpx;
}

.chart-note {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 16rpx;
  padding: 60rpx 0;
}

.empty-icon {
  font-size: 64rpx;
}

.calendar-section {
  background: white;
  border-radius: 20rpx;
  padding: 24rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.04);
}

.section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20rpx;
}

.section-title {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
}

.calendar-grid {
  margin-bottom: 20rpx;
}

.calendar-weekdays {
  display: flex;
  margin-bottom: 12rpx;
}

.weekday {
  flex: 1;
  text-align: center;
  font-size: 22rpx;
  color: #94a3b8;
  font-weight: 500;
}

.calendar-days {
  display: grid;
  grid-template-columns: repeat(7, 1fr);
  gap: 8rpx;
}

.calendar-day {
  aspect-ratio: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  border-radius: 12rpx;
  background: #f1f5f9;
  min-height: 70rpx;
}

.calendar-day.empty {
  background: transparent;
}

.calendar-day.has-data {
  color: white;
}

.day-number {
  font-size: 22rpx;
  font-weight: 500;
}

.day-value {
  font-size: 18rpx;
  opacity: 0.9;
}

.calendar-legend {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12rpx;
}

.legend-label {
  font-size: 20rpx;
  color: #94a3b8;
}

.legend-gradient {
  width: 120rpx;
  height: 16rpx;
  border-radius: 8rpx;
}

.bar-chart-section {
  background: white;
  border-radius: 20rpx;
  padding: 24rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.04);
}

.bar-chart-scroll {
  width: 100%;
  white-space: nowrap;
}

.bar-chart {
  display: flex;
  gap: 12rpx;
  padding: 10rpx 0;
}

.bar-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  min-width: 60rpx;
}

.bar-wrapper {
  width: 40rpx;
  height: 160rpx;
  background: #f1f5f9;
  border-radius: 8rpx;
  display: flex;
  align-items: flex-end;
  overflow: hidden;
}

.bar-fill {
  width: 100%;
  border-radius: 8rpx;
  transition: height 0.3s ease;
  min-height: 4rpx;
}

.bar-label {
  font-size: 18rpx;
  color: #64748b;
  margin-top: 8rpx;
}

.bar-value {
  font-size: 18rpx;
  color: #1e293b;
  font-weight: 500;
}

.records-card {
  background: white;
  border-radius: 20rpx;
  padding: 24rpx;
  box-shadow: 0 4rpx 16rpx rgba(0, 0, 0, 0.04);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 16rpx;
}

.card-title {
  font-size: 28rpx;
  font-weight: 600;
  color: #1e293b;
}

.card-count {
  font-size: 22rpx;
  color: #94a3b8;
}

.records-list {
  max-height: 400rpx;
}

.empty-records {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 12rpx;
  padding: 40rpx 0;
}

.empty-icon {
  font-size: 48rpx;
}

.record-item {
  padding: 16rpx 0;
  border-bottom: 1rpx solid #f1f5f9;
}

.record-item:last-child {
  border-bottom: none;
}

.record-date {
  font-size: 24rpx;
  color: #64748b;
  margin-bottom: 8rpx;
}

.record-values {
  display: flex;
  gap: 16rpx;
  flex-wrap: wrap;
}

.record-value {
  font-size: 22rpx;
  color: #475569;
}
</style>