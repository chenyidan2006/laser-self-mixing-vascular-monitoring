<template>
  <view class="health-data">
    <!-- 日期选择 -->
    <view class="date-filter">
      <picker mode="date" :value="selectedDate" :start="startDate" :end="endDate" @change="onDateChange">
        <view class="date-picker">
          <text>{{ selectedDate }}</text>
          <text class="icon">📅</text>
        </view>
      </picker>
    </view>

    <!-- 数据卡片 -->
    <view v-if="hasData" class="data-card">
      <view class="data-item">
        <text class="label">平均心率</text>
        <text class="value">{{ dailyData.avgHeartRate || '--' }} <text class="unit">bpm</text></text>
      </view>
      <view class="data-item">
        <text class="label">收缩压</text>
        <text class="value">{{ dailyData.avgSystolic || '--' }} <text class="unit">mmHg</text></text>
      </view>
      <view class="data-item">
        <text class="label">舒张压</text>
        <text class="value">{{ dailyData.avgDiastolic || '--' }} <text class="unit">mmHg</text></text>
      </view>
      <view class="data-item">
        <text class="label">血氧</text>
        <text class="value">{{ dailyData.avgSpo2 || '--' }} <text class="unit">%</text></text>
      </view>
      <view class="data-item">
        <text class="label">记录总数</text>
        <text class="value">{{ dailyData.totalRecords || 0 }} <text class="unit">条</text></text>
      </view>
    </view>

    <view v-else class="empty">
      <text>该日期暂无数据</text>
    </view>

    <!-- 历史趋势入口 -->
    <navigator url="/pages/health/trend" class="trend-link">
      <text>查看历史趋势</text>
    </navigator>
  </view>
</template>

<script>
export default {
  data() {
    const today = new Date();
    const year = today.getFullYear();
    let month = today.getMonth() + 1;
    let day = today.getDate();
    month = month < 10 ? '0' + month : month;
    day = day < 10 ? '0' + day : day;
    const todayStr = `${year}-${month}-${day}`;

    return {
      selectedDate: todayStr,
      startDate: '2023-01-01',
      endDate: todayStr,
      dailyData: {},
      hasData: false
    };
  },
  onLoad() {
    this.loadData();
  },
  methods: {
    onDateChange(e) {
      this.selectedDate = e.detail.value;
      this.loadData();
    },

    async loadData() {
      const isSimulationMode = uni.getStorageSync('isSimulationMode') === 'true' || false;
      
      if (isSimulationMode) {
        this.loadMockData();
        return;
      }
      
      uni.showLoading({ title: '加载数据..' });
      try {
        const res = await uni.request({
          url: 'http://192.144.185.64:3002/api/data/dailySummary',
          data: { date: this.selectedDate },
          header: { Authorization: 'Bearer ' + uni.getStorageSync('token') }
        });
        if (res.data.errcode === 0) {
          this.dailyData = res.data.data;
          this.hasData = this.dailyData.totalRecords > 0;
        } else {
          uni.showToast({ title: res.data.msg, icon: 'none' });
        }
      } catch (err) {
        this.loadMockData();
      } finally {
        uni.hideLoading();
      }
    },
    loadMockData() {
      const mockDataStr = uni.getStorageSync('mockTrendData');
      if (mockDataStr) {
        try {
          const allData = JSON.parse(mockDataStr);
          const filtered = allData.filter(item => {
            const itemDate = item.date || (item.measure_time ? item.measure_time.split(' ')[0] : '');
            return itemDate === this.selectedDate;
          });
          
          if (filtered.length > 0) {
            const hrSum = filtered.reduce((sum, item) => sum + (item.heart_rate || 0), 0);
            const sysSum = filtered.reduce((sum, item) => sum + (item.systolic_bp || 0), 0);
            const diaSum = filtered.reduce((sum, item) => sum + (item.diastolic_bp || 0), 0);
            const spo2Sum = filtered.reduce((sum, item) => sum + (item.spo2 || 0), 0);
            
            this.dailyData = {
              avgHeartRate: Math.round(hrSum / filtered.length),
              avgSystolic: Math.round(sysSum / filtered.length),
              avgDiastolic: Math.round(diaSum / filtered.length),
              avgSpo2: Math.round(spo2Sum / filtered.length),
              totalRecords: filtered.length
            };
            this.hasData = true;
          } else {
            this.hasData = false;
          }
        } catch (e) {
          this.hasData = false;
        }
      } else {
        this.generateTempMockData();
      }
    },
    generateTempMockData() {
      this.dailyData = {
        avgHeartRate: Math.floor(65 + Math.random() * 25),
        avgSystolic: Math.floor(105 + Math.random() * 25),
        avgDiastolic: Math.floor(65 + Math.random() * 20),
        avgSpo2: Math.floor(95 + Math.random() * 5),
        totalRecords: Math.floor(3 + Math.random() * 5)
      };
      this.hasData = true;
    }
  }
};
</script>

<style lang="scss">
.health-data {
  background: #f5f7fa;
  min-height: 100vh;
  padding: 30rpx;

  .date-filter {
    background: white;
    border-radius: 20rpx;
    padding: 20rpx 30rpx;
    margin-bottom: 30rpx;

    .date-picker {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 32rpx;
      color: #333;

      .icon {
        font-size: 36rpx;
      }
    }
  }

  .data-card {
    background: white;
    border-radius: 20rpx;
    padding: 30rpx;

    .data-item {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 20rpx 0;
      border-bottom: 2rpx solid #f0f0f0;

      &:last-child {
        border-bottom: none;
      }

      .label {
        font-size: 30rpx;
        color: #666;
      }

      .value {
        font-size: 36rpx;
        font-weight: 600;
        color: #333;

        .unit {
          font-size: 24rpx;
          font-weight: normal;
          color: #999;
          margin-left: 6rpx;
        }
      }
    }
  }

  .empty {
    background: white;
    border-radius: 20rpx;
    padding: 60rpx;
    text-align: center;
    color: #999;
    font-size: 30rpx;
  }

  .trend-link {
    margin-top: 40rpx;
    text-align: center;
    color: #2b85e4;
    font-size: 30rpx;
    padding: 20rpx;
  }
}
</style>