<template>
  <view class="page-container">
    <!-- 顶部导航栏 -->
    <view class="nav-bar">
      <view class="nav-title">
        <text class="app-name">健康监测系统</text>
        <text class="app-version">v1.0</text>
      </view>
      
      <!-- 设备状态 -->
      <view class="nav-status" @click="showDeviceStatus">
        <view class="status-dot" :class="{'online': deviceStatus.isConnected, 'offline': !deviceStatus.isConnected}"></view>
        <text class="status-text">{{ deviceStatus.deviceId || '未连接' }}</text>
      </view>
    </view>

    <!-- 日期选择区 -->
    <view class="date-section">
      <view class="date-display">
        <text class="current-date">{{ formattedSelectedDate }}</text>
        <text class="date-week">{{ selectedWeekday }}</text>
      </view>
      
      <view class="date-controls">
        <view class="date-btn prev-date" @click="changeDate(-1)">
          <text class="btn-icon">←</text>
          <text class="btn-text">前一天</text>
        </view>
        
        <view class="date-picker" @click="showDatePicker = true">
          <text class="calendar-icon">📅</text>
          <text class="picker-text">选择日期</text>
        </view>
        
        <view class="date-btn next-date" @click="changeDate(1)" :class="{'disabled': isToday}">
          <text class="btn-text">后一天</text>
          <text class="btn-icon">→</text>
        </view>
      </view>
    </view>

    <!-- 数据加载状态 -->
    <view v-if="loading" class="loading-overlay">
      <view class="loading-content">
        <view class="spinner"></view>
        <text class="loading-text">数据加载中...</text>
        <text v-if="loadingProgress" class="loading-progress">{{ loadingProgress }}</text>
      </view>
    </view>

    <!-- 主内容区 -->
    <scroll-view class="main-content" scroll-y>
      <!-- 数据状态提示 -->
      <view v-if="!hasData && !loading" class="empty-state">
        <view class="empty-icon">📊</view>
        <text class="empty-title">暂无数据</text>
        <text class="empty-desc">该日期没有监测数据</text>
        <view class="empty-actions">
          <button class="action-btn" @click="refreshData">
            <text class="btn-icon">🔄</text>
            <text class="btn-text">刷新</text>
          </button>
          <button class="action-btn primary" @click="showDatePicker = true">
            <text class="btn-icon">📅</text>
            <text class="btn-text">选择日期</text>
          </button>
        </view>
      </view>

      <!-- 设备信息卡片 -->
      <view v-if="hasData" class="card device-card">
        <view class="card-header">
          <view class="card-title">
            <view class="title-icon">📱</view>
            <text class="title-text">设备信息</text>
          </view>
          <view class="card-badge" :class="{'online': deviceStatus.isConnected, 'offline': !deviceStatus.isConnected}">
            {{ deviceStatus.isConnected ? '在线' : '离线' }}
          </view>
        </view>
        
        <view class="card-body">
          <view class="device-grid">
            <view class="grid-item">
              <text class="item-label">设备ID</text>
              <text class="item-value">{{ deviceStatus.deviceId || '--' }}</text>
            </view>
            
            <view class="grid-item">
              <text class="item-label">最后更新</text>
              <text class="item-value">{{ deviceStatus.lastUpdate | formatTime }}</text>
            </view>
            
            <view class="grid-item">
              <text class="item-label">数据延迟</text>
              <text class="item-value">{{ deviceStatus.dataDelay }}分钟前</text>
            </view>
            
            <view class="grid-item">
              <text class="item-label">今日记录</text>
              <text class="item-value">{{ dailyData.totalRecords || 0 }}条</text>
            </view>
          </view>
          
          <view class="device-footer">
            <text class="footer-text">数据来自4G模块，每8秒更新一次</text>
          </view>
        </view>
      </view>

      <!-- 身心平衡卡片 -->
      <view v-if="hasData" class="card balance-card">
        <view class="card-header">
          <view class="card-title">
            <view class="title-icon">🧘</view>
            <text class="title-text">身心平衡</text>
          </view>
          <view class="card-time">
            <text class="time-label">最后更新</text>
            <text class="time-value">{{ dailyData.lastUpdate | formatTime }}</text>
          </view>
        </view>
        
        <view class="card-body">
          <!-- 总体评分 -->
          <view class="overall-score">
            <view class="score-circle">
              <view class="circle-bg"></view>
              <view class="circle-progress" :style="{'--progress': dailyData.overallBalance}"></view>
              <view class="score-text">
                <text class="score-value">{{ dailyData.overallBalance || 0 }}</text>
                <text class="score-unit">分</text>
              </view>
            </view>
            <text class="score-label">总体评分</text>
          </view>
          
          <!-- 分项指标 -->
          <view class="balance-metrics">
            <view class="metric-item">
              <view class="metric-icon">🩺</view>
              <text class="metric-label">体征状态</text>
              <text class="metric-value">{{ dailyData.vitalSigns || 0 }}分</text>
              <view class="metric-bar">
                <view class="bar-fill" :style="{width: `${dailyData.vitalSigns || 0}%`}"></view>
              </view>
              <text class="metric-status">{{ getScoreStatus(dailyData.vitalSigns) }}</text>
            </view>
            
            <view class="metric-item">
              <view class="metric-icon">🏃</view>
              <text class="metric-label">活动状态</text>
              <text class="metric-value">{{ dailyData.activityLevel || 0 }}分</text>
              <view class="metric-bar">
                <view class="bar-fill" :style="{width: `${dailyData.activityLevel || 0}%`}"></view>
              </view>
              <text class="metric-status">{{ getScoreStatus(dailyData.activityLevel) }}</text>
            </view>
            
            <view class="metric-item">
              <view class="metric-icon">😌</view>
              <text class="metric-label">放松状态</text>
              <text class="metric-value">{{ dailyData.relaxationState || 0 }}分</text>
              <view class="metric-bar">
                <view class="bar-fill" :style="{width: `${dailyData.relaxationState || 0}%`}"></view>
              </view>
              <text class="metric-status">{{ getScoreStatus(dailyData.relaxationState) }}</text>
            </view>
          </view>
          
          <view class="balance-note">
            <text class="note-text">评分基于当日心率、血压、血氧数据的综合分析</text>
          </view>
        </view>
      </view>

      <!-- 实时数据卡片 -->
      <view v-if="hasData" class="card realtime-card">
        <view class="card-header">
          <view class="card-title">
            <view class="title-icon">📈</view>
            <text class="title-text">实时生理数据</text>
          </view>
          <view class="card-badge" v-if="isToday && deviceStatus.isConnected">
            <view class="pulse-animation"></view>
            <text class="badge-text">实时更新中</text>
          </view>
        </view>
        
        <view class="card-body">
          <view class="realtime-grid">
            <!-- 心率 -->
            <view class="data-card">
              <view class="data-header">
                <view class="data-icon">❤️</view>
                <text class="data-title">心率</text>
              </view>
              <view class="data-content">
                <text class="data-value">{{ dailyData.avgHeartRate || '--' }}</text>
                <text class="data-unit">bpm</text>
              </view>
              <view class="data-trend">
                <text class="trend-text">{{ getTrendText(dailyData.heartRateHistory) }}</text>
                <text class="trend-icon">{{ getTrendIcon(dailyData.heartRateHistory) }}</text>
              </view>
              <view class="data-range">
                <text class="range-text">正常: 60-100</text>
              </view>
            </view>
            
            <!-- 收缩压 -->
            <view class="data-card">
              <view class="data-header">
                <view class="data-icon">🩸</view>
                <text class="data-title">收缩压</text>
              </view>
              <view class="data-content">
                <text class="data-value">{{ dailyData.avgSystolic || '--' }}</text>
                <text class="data-unit">mmHg</text>
              </view>
              <view class="data-trend">
                <text class="trend-text">{{ getTrendText(dailyData.bpHistory?.systolic) }}</text>
                <text class="trend-icon">{{ getTrendIcon(dailyData.bpHistory?.systolic) }}</text>
              </view>
              <view class="data-range">
                <text class="range-text">正常: 90-139</text>
              </view>
            </view>
            
            <!-- 舒张压 -->
            <view class="data-card">
              <view class="data-header">
                <view class="data-icon">💓</view>
                <text class="data-title">舒张压</text>
              </view>
              <view class="data-content">
                <text class="data-value">{{ dailyData.avgDiastolic || '--' }}</text>
                <text class="data-unit">mmHg</text>
              </view>
              <view class="data-trend">
                <text class="trend-text">{{ getTrendText(dailyData.bpHistory?.diastolic) }}</text>
                <text class="trend-icon">{{ getTrendIcon(dailyData.bpHistory?.diastolic) }}</text>
              </view>
              <view class="data-range">
                <text class="range-text">正常: 60-89</text>
              </view>
            </view>
            
            <!-- 血氧 -->
            <view class="data-card">
              <view class="data-header">
                <view class="data-icon">🫁</view>
                <text class="data-title">血氧</text>
              </view>
              <view class="data-content">
                <text class="data-value">{{ dailyData.avgSpo2 || '--' }}</text>
                <text class="data-unit">%</text>
              </view>
              <view class="data-trend">
                <text class="trend-text">{{ getTrendText(dailyData.spo2History) }}</text>
                <text class="trend-icon">{{ getTrendIcon(dailyData.spo2History) }}</text>
              </view>
              <view class="data-range">
                <text class="range-text">正常: 95-100</text>
              </view>
            </view>
          </view>
        </view>
      </view>

      <!-- 趋势图表卡片 -->
      <view v-if="hasData && showCharts" class="card chart-card">
        <view class="card-header">
          <view class="card-title">
            <view class="title-icon">📊</view>
            <text class="title-text">趋势分析</text>
          </view>
          <view class="chart-tabs">
            <view class="chart-tab" :class="{'active': activeChart === 'heart'}" @click="activeChart = 'heart'">
              <text>心率</text>
            </view>
            <view class="chart-tab" :class="{'active': activeChart === 'blood'}" @click="activeChart = 'blood'">
              <text>血压</text>
            </view>
            <view class="chart-tab" :class="{'active': activeChart === 'oxygen'}" @click="activeChart = 'oxygen'">
              <text>血氧</text>
            </view>
          </view>
        </view>
        
        <view class="card-body">
          <!-- 心率趋势图 -->
          <view v-if="activeChart === 'heart'" class="chart-container">
            <view class="chart-title">
              <text class="chart-name">心率变化趋势 (24小时)</text>
              <text class="chart-unit">单位: bpm</text>
            </view>
            <canvas canvas-id="heartChart" class="chart-canvas"></canvas>
            <view class="chart-legend">
              <view class="legend-item">
                <view class="legend-dot heart-dot"></view>
                <text class="legend-text">心率</text>
              </view>
            </view>
          </view>
          
          <!-- 血压趋势图 -->
          <view v-if="activeChart === 'blood'" class="chart-container">
            <view class="chart-title">
              <text class="chart-name">血压变化趋势 (24小时)</text>
              <text class="chart-unit">单位: mmHg</text>
            </view>
            <canvas canvas-id="bloodChart" class="chart-canvas"></canvas>
            <view class="chart-legend">
              <view class="legend-item">
                <view class="legend-dot systolic-dot"></view>
                <text class="legend-text">收缩压</text>
              </view>
              <view class="legend-item">
                <view class="legend-dot diastolic-dot"></view>
                <text class="legend-text">舒张压</text>
              </view>
            </view>
          </view>
          
          <!-- 血氧趋势图 -->
          <view v-if="activeChart === 'oxygen'" class="chart-container">
            <view class="chart-title">
              <text class="chart-name">血氧变化趋势 (24小时)</text>
              <text class="chart-unit">单位: %</text>
            </view>
            <canvas canvas-id="oxygenChart" class="chart-canvas"></canvas>
            <view class="chart-legend">
              <view class="legend-item">
                <view class="legend-dot oxygen-dot"></view>
                <text class="legend-text">血氧饱和度</text>
              </view>
            </view>
          </view>
        </view>
      </view>

      <!-- 数据说明卡片 -->
      <view class="card info-card">
        <view class="card-header">
          <view class="card-title">
            <view class="title-icon">ℹ️</view>
            <text class="title-text">数据说明</text>
          </view>
        </view>
        
        <view class="card-body">
          <view class="info-list">
            <view class="info-item">
              <view class="info-icon">📡</view>
              <text class="info-text">数据来源: 4G模块实时采集脉搏波数据</text>
            </view>
            
            <view class="info-item">
              <view class="info-icon">⚙️</view>
              <text class="info-text">数据处理: 云端算法计算心率、血压、血氧</text>
            </view>
            
            <view class="info-item">
              <view class="info-icon">📊</view>
              <text class="info-text">数据更新: 每8秒上传一次，实时显示</text>
            </view>
            
            <view class="info-item">
              <view class="info-icon">⭐</view>
              <text class="info-text">评分标准: 基于当日所有数据的综合分析</text>
            </view>
          </view>
        </view>
      </view>

      <!-- 底部操作栏 -->
      <view class="action-bar">
        <button class="action-btn" @click="refreshData" :disabled="loading">
          <text class="btn-icon">🔄</text>
          <text class="btn-text">刷新数据</text>
        </button>
        
        <button class="action-btn primary" @click="showDatePicker = true">
          <text class="btn-icon">📅</text>
          <text class="btn-text">选择日期</text>
        </button>
        
        <button class="action-btn" @click="exportData" :disabled="!hasData">
          <text class="btn-icon">📥</text>
          <text class="btn-text">导出数据</text>
        </button>
      </view>
    </scroll-view>

    <!-- 日期选择器弹窗 -->
    <uni-datetime-picker
      v-if="showDatePicker"
      type="date"
      :value="selectedDate"
      :start="minDate"
      :end="maxDate"
      @maskClick="showDatePicker = false"
      @change="onDateChange"
    >
      <view class="date-picker-modal">
        <view class="picker-header">
          <text class="picker-title">选择日期</text>
          <text class="picker-close" @click="showDatePicker = false">×</text>
        </view>
        <view class="picker-body">
          <!-- uni-datetime-picker 内部会自动渲染 -->
        </view>
      </view>
    </uni-datetime-picker>

    <!-- 设备状态详情弹窗 -->
    <view v-if="showStatusModal" class="status-modal" @click="showStatusModal = false">
      <view class="modal-content" @click.stop>
        <view class="modal-header">
          <text class="modal-title">设备详情</text>
          <text class="modal-close" @click="showStatusModal = false">×</text>
        </view>
        
        <view class="modal-body">
          <view class="status-details">
            <view class="detail-row">
              <text class="detail-label">设备ID:</text>
              <text class="detail-value">{{ deviceStatus.deviceId || '--' }}</text>
            </view>
            
            <view class="detail-row">
              <text class="detail-label">连接状态:</text>
              <text class="detail-value" :class="{'online': deviceStatus.isConnected, 'offline': !deviceStatus.isConnected}">
                {{ deviceStatus.isConnected ? '在线' : '离线' }}
              </text>
            </view>
            
            <view class="detail-row">
              <text class="detail-label">最后更新:</text>
              <text class="detail-value">{{ deviceStatus.lastUpdate | formatFullTime }}</text>
            </view>
            
            <view class="detail-row">
              <text class="detail-label">数据延迟:</text>
              <text class="detail-value">{{ deviceStatus.dataDelay }}分钟</text>
            </view>
            
            <view class="detail-row">
              <text class="detail-label">今日记录:</text>
              <text class="detail-value">{{ dailyData.totalRecords || 0 }}条</text>
            </view>
            
            <view class="detail-row">
              <text class="detail-label">数据频率:</text>
              <text class="detail-value">每8秒一次</text>
            </view>
            
            <view class="detail-row">
              <text class="detail-label">更新时间:</text>
              <text class="detail-value">{{ dailyData.lastUpdate | formatFullTime }}</text>
            </view>
          </view>
          
          <view class="modal-actions">
            <button class="modal-btn" @click="showStatusModal = false">关闭</button>
            <button class="modal-btn primary" @click="refreshDeviceStatus">刷新状态</button>
          </view>
        </view>
      </view>
    </view>
  </view>
</template>

<script>
export default {
  data() {
    // 获取当前日期
    const today = new Date();
    const todayStr = today.toISOString().split('T')[0];
    
    return {
      // 日期相关
      selectedDate: todayStr,
      formattedSelectedDate: '',
      selectedWeekday: '',
      showDatePicker: false,
      minDate: '2023-01-01',
      maxDate: todayStr,
      isToday: true,
      
      // 加载状态
      loading: false,
      loadingProgress: '',
      hasData: false,
      
      // 设备状态
      deviceStatus: {
        deviceId: '',
        isConnected: false,
        lastUpdate: null,
        dataDelay: 0,
        todayRecords: 0
      },
      showStatusModal: false,
      
      // 数据相关
      dailyData: {
        date: '',
        overallBalance: 0,
        vitalSigns: 0,
        activityLevel: 0,
        relaxationState: 0,
        avgHeartRate: 0,
        avgSystolic: 0,
        avgDiastolic: 0,
        avgSpo2: 0,
        heartRateHistory: [],
        bpHistory: {
          systolic: [],
          diastolic: []
        },
        spo2History: [],
        totalRecords: 0,
        lastUpdate: null
      },
      
      // 图表相关
      showCharts: false,
      activeChart: 'heart',
      
      // 定时器
      updateTimer: null,
      refreshInterval: 8000, // 8秒更新一次
      
      // 配置
      config: {
        deviceId: 'ESP32_001', // 默认设备ID
        userId: 'default_user'
      }
    };
  },
  
  filters: {
    formatTime(timestamp) {
      if (!timestamp) return '--';
      const date = new Date(timestamp);
      return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
    },
    
    formatFullTime(timestamp) {
      if (!timestamp) return '--';
      const date = new Date(timestamp);
      return date.toLocaleString([], { 
        year: 'numeric', 
        month: '2-digit', 
        day: '2-digit',
        hour: '2-digit', 
        minute: '2-digit',
        second: '2-digit'
      });
    }
  },
  
  async onLoad() {
    // 初始化
    await this.initApp();
    
    // 监听页面显示
    uni.$on('pageShow', () => {
      this.refreshData();
    });
  },
  
  onShow() {
    // 页面显示时触发刷新
    this.refreshData();
  },
  
  onHide() {
    // 页面隐藏时停止定时器
    this.stopAutoUpdate();
  },
  
  onUnload() {
    // 页面卸载时清理
    this.stopAutoUpdate();
    uni.$off('pageShow');
  },
  
  methods: {
    // 初始化应用
    async initApp() {
      try {
        this.loading = true;
        this.loadingProgress = '正在初始化...';
        
        // 1. 格式化日期显示
        this.updateDateDisplay();
        
        // 2. 加载设备状态
        await this.loadDeviceStatus();
        
        // 3. 加载今日数据
        await this.loadDailyData();
        
        // 4. 如果是今天且设备在线，启动自动更新
        if (this.isToday && this.deviceStatus.isConnected) {
          this.startAutoUpdate();
        }
        
        this.loading = false;
      } catch (error) {
        console.error('初始化失败:', error);
        this.loading = false;
        uni.showToast({
          title: '初始化失败',
          icon: 'error'
        });
      }
    },
    
    // 更新日期显示
    updateDateDisplay() {
      const date = new Date(this.selectedDate);
      const today = new Date();
      this.isToday = date.toDateString() === today.toDateString();
      
      // 格式化日期
      const year = date.getFullYear();
      const month = date.getMonth() + 1;
      const day = date.getDate();
      this.formattedSelectedDate = `${year}年${month}月${day}日`;
      
      // 获取星期几
      const weekdays = ['周日', '周一', '周二', '周三', '周四', '周五', '周六'];
      this.selectedWeekday = weekdays[date.getDay()];
    },
    
    // 加载设备状态
    async loadDeviceStatus() {
		  // 模拟设备状态
		  const mockStatus = {
		    deviceId: 'ESP32_001',
		    isConnected: true,
		    lastUpdate: Date.now() - 5 * 60 * 1000, // 5分钟前
		    dataDelay: 5,
		    todayRecords: 128
		  };
		  this.deviceStatus = mockStatus;
		  return mockStatus;
		  /*
      try {
        const result = await uniCloud.callFunction({
          name: 'get-health-data',
          data: {
            action: 'getDeviceStatus',
            deviceId: this.config.deviceId
          }
        });
        
        if (result.result.errcode === 0) {
          this.deviceStatus = {
            deviceId: result.result.data.deviceId,
            isConnected: result.result.data.isConnected,
            lastUpdate: result.result.data.lastUpdate,
            dataDelay: result.result.data.dataDelay || 0,
            todayRecords: result.result.data.todayRecords || 0
          };
        } else {
          throw new Error(result.result.msg);
        }
      } catch (error) {
        console.error('获取设备状态失败:', error);
        // 设置默认状态
        this.deviceStatus = {
          deviceId: this.config.deviceId,
          isConnected: false,
          lastUpdate: null,
          dataDelay: 999,
          todayRecords: 0
        };
      }
	  */
    },
    
    // 刷新设备状态
    async refreshDeviceStatus() {
      await this.loadDeviceStatus();
    },
    
    // 加载每日数据
    async loadDailyData() {
		  // 模拟今日健康数据
		  const mockDailyData = {
		    date: this.selectedDate,
		    overallBalance: 85,
		    vitalSigns: 82,
		    activityLevel: 78,
		    relaxationState: 90,
		    avgHeartRate: 72,
		    avgSystolic: 118,
		    avgDiastolic: 76,
		    avgSpo2: 98,
		    heartRateHistory: [68, 72, 75, 70, 73, 78, 80, 76, 72, 68, 65, 70, 74, 78, 82, 85, 80, 76, 72, 68, 70, 73, 75, 72],
		    bpHistory: {
		      systolic: [115, 118, 122, 120, 118, 116, 114, 112, 115, 118, 120, 122, 124, 122, 120, 118, 116, 114, 112, 115, 118, 120, 118, 116],
		      diastolic: [75, 78, 80, 78, 76, 74, 72, 70, 72, 74, 76, 78, 80, 78, 76, 74, 72, 70, 72, 74, 76, 78, 76, 74]
		    },
		    spo2History: [97, 98, 98, 97, 98, 99, 98, 97, 98, 98, 97, 98, 99, 98, 98, 97, 98, 99, 98, 97, 98, 98, 97, 98],
		    totalRecords: 128,
		    lastUpdate: Date.now() - 2 * 60 * 1000 // 2分钟前
		  };
		  
		  this.dailyData = mockDailyData;
		  this.hasData = true;
		  
		  // 绘制图表
		  this.$nextTick(() => {
		    this.drawCharts();
		  });
		  
		  return mockDailyData;
	/*
      try {
        this.loadingProgress = '正在加载数据...';
        
        const result = await uniCloud.callFunction({
          name: 'get-health-data',
          data: {
            action: 'getDailySummary',
            deviceId: this.config.deviceId,
            date: this.selectedDate
          }
        });
        
        if (result.result.errcode === 0 && result.result.data) {
          const data = result.result.data;
          
          // 处理数据
          this.dailyData = {
            date: data.date,
            overallBalance: data.overallBalance || 0,
            vitalSigns: data.vitalSigns || 0,
            activityLevel: data.activityLevel || 0,
            relaxationState: data.relaxationState || 0,
            avgHeartRate: data.avgHeartRate || 0,
            avgSystolic: data.avgSystolic || 0,
            avgDiastolic: data.avgDiastolic || 0,
            avgSpo2: data.avgSpo2 || 0,
            heartRateHistory: data.heartRateHistory || [],
            bpHistory: data.bpHistory || { systolic: [], diastolic: [] },
            spo2History: data.spo2History || [],
            totalRecords: data.totalRecords || 0,
            lastUpdate: data.lastUpdate
          };
          
          this.hasData = this.checkHasData();
          
          // 如果有数据，绘制图表
          if (this.hasData) {
            this.$nextTick(() => {
              this.drawCharts();
            });
          }
        } else {
          this.dailyData = this.getEmptyData();
          this.hasData = false;
        }
        
        this.loadingProgress = '';
      } catch (error) {
        console.error('加载数据失败:', error);
        this.dailyData = this.getEmptyData();
        this.hasData = false;
        this.loadingProgress = '';
        
        uni.showToast({
          title: '加载数据失败',
          icon: 'error'
        });
      }
	*/
    },
    
    // 检查是否有数据
    checkHasData() {
      return (
        this.dailyData.totalRecords > 0 ||
        this.dailyData.overallBalance > 0 ||
        this.dailyData.avgHeartRate > 0 ||
        this.dailyData.avgSystolic > 0 ||
        this.dailyData.avgSpo2 > 0
      );
    },
    
    // 获取空数据
    getEmptyData() {
      return {
        date: this.selectedDate,
        overallBalance: 0,
        vitalSigns: 0,
        activityLevel: 0,
        relaxationState: 0,
        avgHeartRate: 0,
        avgSystolic: 0,
        avgDiastolic: 0,
        avgSpo2: 0,
        heartRateHistory: [],
        bpHistory: { systolic: [], diastolic: [] },
        spo2History: [],
        totalRecords: 0,
        lastUpdate: null
      };
    },
    
    // 绘制图表
    drawCharts() {
      this.showCharts = true;
      
      // 绘制心率图表
      if (this.dailyData.heartRateHistory && this.dailyData.heartRateHistory.length > 0) {
        this.drawLineChart(
          'heartChart',
          '心率变化趋势',
          this.dailyData.heartRateHistory,
          '#FF4444',
          'bpm'
        );
      }
      
      // 绘制血压图表
      if (this.dailyData.bpHistory && 
          this.dailyData.bpHistory.systolic.length > 0 && 
          this.dailyData.bpHistory.diastolic.length > 0) {
        this.drawMultiLineChart(
          'bloodChart',
          '血压变化趋势',
          [
            { data: this.dailyData.bpHistory.systolic, color: '#FF6B6B', label: '收缩压' },
            { data: this.dailyData.bpHistory.diastolic, color: '#FF9F43', label: '舒张压' }
          ],
          'mmHg'
        );
      }
      
      // 绘制血氧图表
      if (this.dailyData.spo2History && this.dailyData.spo2History.length > 0) {
        this.drawLineChart(
          'oxygenChart',
          '血氧变化趋势',
          this.dailyData.spo2History,
          '#36A3F7',
          '%'
        );
      }
    },
    
    // 绘制单线图表
    drawLineChart(canvasId, title, data, color, unit) {
      const ctx = uni.createCanvasContext(canvasId);
      const width = 300; // 画布宽度
      const height = 200; // 画布高度
      const padding = {
        top: 20,
        right: 10,
        bottom: 30,
        left: 40
      };
      
      const chartWidth = width - padding.left - padding.right;
      const chartHeight = height - padding.top - padding.bottom;
      
      // 清除画布
      ctx.clearRect(0, 0, width, height);
      
      if (!data || data.length === 0) {
        // 显示无数据提示
        ctx.setFontSize(14);
        ctx.setFillStyle('#999');
        ctx.fillText('暂无数据', width / 2 - 30, height / 2);
        ctx.draw();
        return;
      }
      
      // 计算数据范围
      const maxValue = Math.max(...data) * 1.1;
      const minValue = Math.min(...data) * 0.9;
      const valueRange = maxValue - minValue;
      
      // 绘制网格
      ctx.setStrokeStyle('#F0F0F0');
      ctx.setLineWidth(1);
      
      // 水平网格
      for (let i = 0; i <= 5; i++) {
        const y = padding.top + (i * chartHeight / 5);
        ctx.moveTo(padding.left, y);
        ctx.lineTo(width - padding.right, y);
      }
      
      // 垂直网格
      const dataCount = data.length;
      const verticalLines = Math.min(12, dataCount);
      for (let i = 0; i <= verticalLines; i++) {
        const x = padding.left + (i * chartWidth / verticalLines);
        ctx.moveTo(x, padding.top);
        ctx.lineTo(x, height - padding.bottom);
      }
      ctx.stroke();
      
      // 绘制数据线
      ctx.beginPath();
      ctx.setStrokeStyle(color);
      ctx.setLineWidth(2);
      
      for (let i = 0; i < dataCount; i++) {
        const x = padding.left + (i / (dataCount - 1)) * chartWidth;
        const y = height - padding.bottom - ((data[i] - minValue) / valueRange) * chartHeight;
        
        if (i === 0) {
          ctx.moveTo(x, y);
        } else {
          ctx.lineTo(x, y);
        }
      }
      ctx.stroke();
      
      // 绘制数据点
      ctx.setFillStyle(color);
      for (let i = 0; i < dataCount; i += Math.max(1, Math.floor(dataCount / 10))) {
        const x = padding.left + (i / (dataCount - 1)) * chartWidth;
        const y = height - padding.bottom - ((data[i] - minValue) / valueRange) * chartHeight;
        
        ctx.beginPath();
        ctx.arc(x, y, 3, 0, Math.PI * 2);
        ctx.fill();
      }
      
      // 绘制坐标轴标签
      ctx.setFontSize(10);
      ctx.setFillStyle('#666');
      ctx.textAlign = 'center';
      
      // X轴标签（时间）
      for (let i = 0; i <= 6; i++) {
        const x = padding.left + (i * chartWidth / 6);
        const hour = i * 4; // 0, 4, 8, 12, 16, 20, 24
        ctx.fillText(`${hour}:00`, x, height - 10);
      }
      
      // Y轴标签（数值）
      ctx.textAlign = 'right';
      for (let i = 0; i <= 5; i++) {
        const y = padding.top + (i * chartHeight / 5);
        const value = maxValue - (i * valueRange / 5);
        ctx.fillText(Math.round(value).toString(), padding.left - 5, y + 3);
      }
      
      // 绘制标题
      ctx.setFontSize(12);
      ctx.setFillStyle('#333');
      ctx.textAlign = 'center';
      ctx.fillText(title, width / 2, padding.top - 5);
      
      // 绘制单位
      ctx.setFontSize(10);
      ctx.setFillStyle('#999');
      ctx.textAlign = 'right';
      ctx.fillText(`单位: ${unit}`, width - padding.right, padding.top - 5);
      
      ctx.draw();
    },
    
    // 绘制多线图表
    drawMultiLineChart(canvasId, title, lines, unit) {
      const ctx = uni.createCanvasContext(canvasId);
      const width = 300;
      const height = 200;
      const padding = {
        top: 20,
        right: 10,
        bottom: 30,
        left: 40
      };
      
      const chartWidth = width - padding.left - padding.right;
      const chartHeight = height - padding.top - padding.bottom;
      
      ctx.clearRect(0, 0, width, height);
      
      // 计算所有数据的范围
      const allData = [];
      lines.forEach(line => {
        allData.push(...line.data);
      });
      
      if (allData.length === 0) {
        ctx.setFontSize(14);
        ctx.setFillStyle('#999');
        ctx.fillText('暂无数据', width / 2 - 30, height / 2);
        ctx.draw();
        return;
      }
      
      const maxValue = Math.max(...allData) * 1.1;
      const minValue = Math.min(...allData) * 0.9;
      const valueRange = maxValue - minValue;
      
      // 绘制网格
      ctx.setStrokeStyle('#F0F0F0');
      ctx.setLineWidth(1);
      
      for (let i = 0; i <= 5; i++) {
        const y = padding.top + (i * chartHeight / 5);
        ctx.moveTo(padding.left, y);
        ctx.lineTo(width - padding.right, y);
      }
      
      const dataCount = lines[0].data.length;
      const verticalLines = Math.min(12, dataCount);
      for (let i = 0; i <= verticalLines; i++) {
        const x = padding.left + (i * chartWidth / verticalLines);
        ctx.moveTo(x, padding.top);
        ctx.lineTo(x, height - padding.bottom);
      }
      ctx.stroke();
      
      // 绘制每条线
      lines.forEach(line => {
        ctx.beginPath();
        ctx.setStrokeStyle(line.color);
        ctx.setLineWidth(2);
        
        for (let i = 0; i < line.data.length; i++) {
          const x = padding.left + (i / (line.data.length - 1)) * chartWidth;
          const y = height - padding.bottom - ((line.data[i] - minValue) / valueRange) * chartHeight;
          
          if (i === 0) {
            ctx.moveTo(x, y);
          } else {
            ctx.lineTo(x, y);
          }
        }
        ctx.stroke();
      });
      
      // 绘制坐标轴标签
      ctx.setFontSize(10);
      ctx.setFillStyle('#666');
      ctx.textAlign = 'center';
      
      for (let i = 0; i <= 6; i++) {
        const x = padding.left + (i * chartWidth / 6);
        const hour = i * 4;
        ctx.fillText(`${hour}:00`, x, height - 10);
      }
      
      ctx.textAlign = 'right';
      for (let i = 0; i <= 5; i++) {
        const y = padding.top + (i * chartHeight / 5);
        const value = maxValue - (i * valueRange / 5);
        ctx.fillText(Math.round(value).toString(), padding.left - 5, y + 3);
      }
      
      // 标题
      ctx.setFontSize(12);
      ctx.setFillStyle('#333');
      ctx.textAlign = 'center';
      ctx.fillText(title, width / 2, padding.top - 5);
      
      ctx.setFontSize(10);
      ctx.setFillStyle('#999');
      ctx.textAlign = 'right';
      ctx.fillText(`单位: ${unit}`, width - padding.right, padding.top - 5);
      
      ctx.draw();
    },
    
    // 日期变更
    changeDate(delta) {
      if (delta > 0 && this.isToday) {
        // 不能选择今天之后的日期
        return;
      }
      
      const date = new Date(this.selectedDate);
      date.setDate(date.getDate() + delta);
      this.selectedDate = date.toISOString().split('T')[0];
      
      this.updateDateDisplay();
      this.refreshData();
    },
    
    // 日期选择器变更
    onDateChange(e) {
      this.selectedDate = e;
      this.showDatePicker = false;
      
      this.updateDateDisplay();
      this.refreshData();
    },
    
    // 刷新所有数据
    async refreshData() {
      try {
        this.loading = true;
        
        // 停止之前的定时器
        this.stopAutoUpdate();
        
        // 并行加载设备状态和每日数据
        await Promise.all([
          this.loadDeviceStatus(),
          this.loadDailyData()
        ]);
        
        // 如果是今天且设备在线，重新启动定时器
        if (this.isToday && this.deviceStatus.isConnected) {
          this.startAutoUpdate();
        }
        
        this.loading = false;
        
        // 显示刷新成功提示
        uni.showToast({
          title: '数据已刷新',
          icon: 'success',
          duration: 1500
        });
      } catch (error) {
        console.error('刷新数据失败:', error);
        this.loading = false;
      }
    },
    
    // 启动自动更新
    startAutoUpdate() {
      // 停止之前的定时器
      this.stopAutoUpdate();
      
      // 创建新的定时器
      this.updateTimer = setInterval(async () => {
        if (this.isToday) {
          await this.loadDeviceStatus();
          await this.loadDailyData();
        }
      }, this.refreshInterval);
    },
    
    // 停止自动更新
    stopAutoUpdate() {
      if (this.updateTimer) {
        clearInterval(this.updateTimer);
        this.updateTimer = null;
      }
    },
    
    // 显示设备状态
    showDeviceStatus() {
      this.showStatusModal = true;
    },
    
    // 导出数据
    async exportData() {
      if (!this.hasData) {
        uni.showToast({
          title: '没有数据可以导出',
          icon: 'none'
        });
        return;
      }
      
      try {
        uni.showLoading({
          title: '正在导出数据...'
        });
        
        // 这里可以实现数据导出功能
        // 可以将数据导出为CSV、PDF或分享到其他应用
        
        // 示例：保存为JSON文件
        const exportData = {
          date: this.selectedDate,
          deviceId: this.config.deviceId,
          data: this.dailyData
        };
        
        // 在真实应用中，这里可以使用uni.saveFile保存文件
        console.log('导出数据:', exportData);
        
        uni.showToast({
          title: '数据导出成功',
          icon: 'success'
        });
        
      } catch (error) {
        console.error('导出数据失败:', error);
        uni.showToast({
          title: '导出失败',
          icon: 'error'
        });
      }
    },
    
    // 获取评分状态
    getScoreStatus(score) {
      if (!score) return '--';
      if (score >= 90) return '优秀';
      if (score >= 80) return '良好';
      if (score >= 70) return '一般';
      if (score >= 60) return '需关注';
      return '需改善';
    },
    
    // 获取趋势文本
    getTrendText(data) {
      if (!data || data.length < 2) return '--';
      
      const last = data[data.length - 1];
      const prev = data[data.length - 2];
      const diff = last - prev;
      
      if (Math.abs(diff) <= 1) return '平稳';
      return diff > 0 ? '上升' : '下降';
    },
    
    // 获取趋势图标
    getTrendIcon(data) {
      if (!data || data.length < 2) return '➡️';
      
      const last = data[data.length - 1];
      const prev = data[data.length - 2];
      const diff = last - prev;
      
      if (Math.abs(diff) <= 1) return '➡️';
      return diff > 0 ? '↗️' : '↘️';
    }
  }
};
</script>

<style scoped>
/* 基础样式 */
.page-container {
  background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
  min-height: 100vh;
}

/* 导航栏 */
.nav-bar {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  padding: 60rpx 30rpx 20rpx;
  color: white;
  display: flex;
  justify-content: space-between;
  align-items: center;
  position: sticky;
  top: 0;
  z-index: 100;
}

.nav-title {
  display: flex;
  align-items: center;
  gap: 12rpx;
}

.app-name {
  font-size: 36rpx;
  font-weight: 700;
}

.app-version {
  font-size: 22rpx;
  opacity: 0.8;
}

.nav-status {
  display: flex;
  align-items: center;
  gap: 10rpx;
  background: rgba(255, 255, 255, 0.2);
  padding: 12rpx 20rpx;
  border-radius: 20rpx;
  backdrop-filter: blur(10rpx);
}

.status-dot {
  width: 12rpx;
  height: 12rpx;
  border-radius: 50%;
}

.status-dot.online {
  background: #07c160;
  animation: pulse 2s infinite;
}

.status-dot.offline {
  background: #ff4444;
}

.status-text {
  font-size: 24rpx;
  font-weight: 500;
}

@keyframes pulse {
  0% { opacity: 1; }
  50% { opacity: 0.5; }
  100% { opacity: 1; }
}

/* 日期选择区 */
.date-section {
  background: white;
  margin: 0 30rpx;
  padding: 30rpx;
  border-radius: 20rpx;
  margin-top: -20rpx;
  box-shadow: 0 4rpx 20rpx rgba(0, 0, 0, 0.1);
}

.date-display {
  text-align: center;
  margin-bottom: 30rpx;
}

.current-date {
  font-size: 40rpx;
  font-weight: 700;
  color: #333;
  display: block;
  margin-bottom: 8rpx;
}

.date-week {
  font-size: 26rpx;
  color: #666;
}

.date-controls {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.date-btn {
  display: flex;
  align-items: center;
  gap: 8rpx;
  padding: 15rpx 25rpx;
  border-radius: 12rpx;
  background: #f5f7fa;
  color: #333;
  font-size: 26rpx;
}

.date-btn.disabled {
  opacity: 0.5;
}

.btn-icon {
  font-size: 24rpx;
}

.date-picker {
  display: flex;
  align-items: center;
  gap: 10rpx;
  padding: 15rpx 30rpx;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border-radius: 12rpx;
  font-size: 26rpx;
  font-weight: 500;
}

.calendar-icon {
  font-size: 28rpx;
}

/* 加载状态 */
.loading-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(255, 255, 255, 0.9);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}

.loading-content {
  text-align: center;
}

.spinner {
  width: 60rpx;
  height: 60rpx;
  border: 6rpx solid #f3f3f3;
  border-top: 6rpx solid #667eea;
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin: 0 auto 30rpx;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

.loading-text {
  font-size: 28rpx;
  color: #333;
  display: block;
  margin-bottom: 10rpx;
}

.loading-progress {
  font-size: 24rpx;
  color: #666;
}

/* 主内容区 */
.main-content {
  height: calc(100vh - 280rpx);
  padding: 30rpx;
}

/* 空状态 */
.empty-state {
  text-align: center;
  padding: 100rpx 40rpx;
}

.empty-icon {
  font-size: 120rpx;
  color: #ccc;
  display: block;
  margin-bottom: 30rpx;
}

.empty-title {
  font-size: 36rpx;
  color: #333;
  font-weight: 600;
  display: block;
  margin-bottom: 15rpx;
}

.empty-desc {
  font-size: 28rpx;
  color: #666;
  display: block;
  margin-bottom: 40rpx;
}

.empty-actions {
  display: flex;
  gap: 20rpx;
  justify-content: center;
}

/* 卡片通用样式 */
.card {
  background: white;
  border-radius: 20rpx;
  margin-bottom: 30rpx;
  box-shadow: 0 4rpx 20rpx rgba(0, 0, 0, 0.08);
  overflow: hidden;
}

.card-header {
  padding: 30rpx;
  border-bottom: 1rpx solid #f0f0f0;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.card-title {
  display: flex;
  align-items: center;
  gap: 15rpx;
}

.title-icon {
  font-size: 36rpx;
}

.title-text {
  font-size: 32rpx;
  font-weight: 600;
  color: #333;
}

.card-badge {
  padding: 6rpx 18rpx;
  border-radius: 20rpx;
  font-size: 22rpx;
  font-weight: 500;
}

.card-badge.online {
  background: #e7f7e9;
  color: #07c160;
}

.card-badge.offline {
  background: #ffeaea;
  color: #ff4444;
}

.card-time {
  display: flex;
  align-items: center;
  gap: 8rpx;
}

.time-label {
  font-size: 22rpx;
  color: #666;
}

.time-value {
  font-size: 22rpx;
  color: #333;
  font-weight: 500;
}

.card-body {
  padding: 30rpx;
}

/* 设备卡片 */
.device-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 25rpx;
  margin-bottom: 20rpx;
}

.grid-item {
  display: flex;
  flex-direction: column;
}

.item-label {
  font-size: 22rpx;
  color: #666;
  margin-bottom: 8rpx;
}

.item-value {
  font-size: 28rpx;
  color: #333;
  font-weight: 500;
}

.device-footer {
  padding-top: 20rpx;
  border-top: 1rpx solid #f0f0f0;
}

.footer-text {
  font-size: 22rpx;
  color: #888;
}

/* 身心平衡卡片 */
.overall-score {
  text-align: center;
  margin-bottom: 40rpx;
}

.score-circle {
  position: relative;
  width: 200rpx;
  height: 200rpx;
  margin: 0 auto 15rpx;
}

.circle-bg {
  position: absolute;
  width: 100%;
  height: 100%;
  border: 15rpx solid #f0f0f0;
  border-radius: 50%;
}

.circle-progress {
  position: absolute;
  width: 100%;
  height: 100%;
  border: 15rpx solid #667eea;
  border-radius: 50%;
  clip-path: polygon(50% 50%, 50% 0, calc(50% + 100 * sin(var(--progress) * 3.6deg)) calc(50% - 100 * cos(var(--progress) * 3.6deg)), 50% 50%);
}

.score-text {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  text-align: center;
}

.score-value {
  font-size: 56rpx;
  font-weight: 700;
  color: #333;
}

.score-unit {
  font-size: 28rpx;
  color: #666;
  margin-left: 5rpx;
}

.score-label {
  font-size: 28rpx;
  color: #333;
  font-weight: 500;
}

.balance-metrics {
  display: flex;
  flex-direction: column;
  gap: 25rpx;
}

.metric-item {
  display: flex;
  align-items: center;
  gap: 20rpx;
}

.metric-icon {
  font-size: 40rpx;
  width: 80rpx;
  text-align: center;
}

.metric-info {
  flex: 1;
}

.metric-label {
  font-size: 28rpx;
  color: #333;
  display: block;
  margin-bottom: 8rpx;
}

.metric-value {
  font-size: 32rpx;
  font-weight: 700;
  color: #333;
}

.metric-bar {
  flex: 1;
  height: 12rpx;
  background: #f0f0f0;
  border-radius: 6rpx;
  overflow: hidden;
}

.bar-fill {
  height: 100%;
  background: linear-gradient(90deg, #667eea, #764ba2);
  border-radius: 6rpx;
  transition: width 0.5s ease;
}

.metric-status {
  font-size: 22rpx;
  color: #666;
  min-width: 80rpx;
  text-align: right;
}

.balance-note {
  margin-top: 25rpx;
  padding: 20rpx;
  background: #f8f9fa;
  border-radius: 12rpx;
}

.note-text {
  font-size: 24rpx;
  color: #666;
}

/* 实时数据卡片 */
.realtime-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 25rpx;
}

.data-card {
  padding: 25rpx;
  background: #f8f9fa;
  border-radius: 16rpx;
  border: 2rpx solid #e9ecef;
}

.data-header {
  display: flex;
  align-items: center;
  gap: 12rpx;
  margin-bottom: 15rpx;
}

.data-icon {
  font-size: 36rpx;
}

.data-title {
  font-size: 26rpx;
  color: #333;
  font-weight: 500;
}

.data-content {
  display: flex;
  align-items: baseline;
  gap: 8rpx;
  margin-bottom: 10rpx;
}

.data-value {
  font-size: 48rpx;
  font-weight: 700;
  color: #333;
}

.data-unit {
  font-size: 24rpx;
  color: #666;
}

.data-trend {
  display: flex;
  align-items: center;
  gap: 8rpx;
  margin-bottom: 10rpx;
}

.trend-text {
  font-size: 22rpx;
  color: #666;
}

.trend-icon {
  font-size: 20rpx;
}

.data-range {
  font-size: 20rpx;
  color: #999;
}

.pulse-animation {
  width: 12rpx;
  height: 12rpx;
  background: #07c160;
  border-radius: 50%;
  animation: pulse 1.5s infinite;
  margin-right: 8rpx;
}

/* 图表卡片 */
.chart-tabs {
  display: flex;
  gap: 20rpx;
}

.chart-tab {
  padding: 10rpx 20rpx;
  border-radius: 20rpx;
  background: #f5f7fa;
  color: #666;
  font-size: 24rpx;
}

.chart-tab.active {
  background: #667eea;
  color: white;
}

.chart-container {
  margin-top: 20rpx;
}

.chart-title {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 15rpx;
}

.chart-name {
  font-size: 26rpx;
  color: #333;
  font-weight: 500;
}

.chart-unit {
  font-size: 22rpx;
  color: #666;
}

.chart-canvas {
  width: 100%;
  height: 400rpx;
  display: block;
}

.chart-legend {
  display: flex;
  justify-content: center;
  gap: 30rpx;
  margin-top: 20rpx;
}

.legend-item {
  display: flex;
  align-items: center;
  gap: 8rpx;
}

.legend-dot {
  width: 16rpx;
  height: 16rpx;
  border-radius: 50%;
}

.heart-dot {
  background: #FF4444;
}

.systolic-dot {
  background: #FF6B6B;
}

.diastolic-dot {
  background: #FF9F43;
}

.oxygen-dot {
  background: #36A3F7;
}

.legend-text {
  font-size: 22rpx;
  color: #666;
}

/* 信息卡片 */
.info-list {
  display: flex;
  flex-direction: column;
  gap: 25rpx;
}

.info-item {
  display: flex;
  align-items: flex-start;
  gap: 20rpx;
}

.info-icon {
  font-size: 32rpx;
  min-width: 40rpx;
  text-align: center;
}

.info-text {
  font-size: 26rpx;
  color: #333;
  line-height: 1.5;
}

/* 操作栏 */
.action-bar {
  display: flex;
  gap: 20rpx;
  margin: 40rpx 0;
}

.action-btn {
  flex: 1;
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 10rpx;
  padding: 25rpx;
  background: #f5f7fa;
  border: none;
  border-radius: 12rpx;
  color: #333;
  font-size: 26rpx;
}

.action-btn.primary {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
}

.action-btn:disabled {
  opacity: 0.5;
}

.btn-text {
  font-weight: 500;
}

/* 弹窗样式 */
.status-modal {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.7);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 2000;
}

.modal-content {
  width: 650rpx;
  background: white;
  border-radius: 24rpx;
  overflow: hidden;
}

.modal-header {
  padding: 40rpx 30rpx;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.modal-title {
  font-size: 34rpx;
  font-weight: 600;
}

.modal-close {
  font-size: 48rpx;
  color: rgba(255, 255, 255, 0.9);
  padding: 0 20rpx;
}

.modal-body {
  padding: 40rpx 30rpx;
}

.status-details {
  margin-bottom: 40rpx;
}

.detail-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 25rpx 0;
  border-bottom: 1rpx solid #f0f0f0;
}

.detail-row:last-child {
  border-bottom: none;
}

.detail-label {
  font-size: 28rpx;
  color: #666;
}

.detail-value {
  font-size: 28rpx;
  color: #333;
  font-weight: 500;
}

.detail-value.online {
  color: #07c160;
}

.detail-value.offline {
  color: #ff4444;
}

.modal-actions {
  display: flex;
  gap: 20rpx;
}

.modal-btn {
  flex: 1;
  padding: 25rpx;
  background: #f5f7fa;
  border: none;
  border-radius: 12rpx;
  color: #333;
  font-size: 28rpx;
  font-weight: 500;
}

.modal-btn.primary {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
}

/* 日期选择器弹窗 */
.date-picker-modal {
  width: 650rpx;
  background: white;
  border-radius: 24rpx;
  overflow: hidden;
}

.picker-header {
  padding: 40rpx 30rpx;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.picker-title {
  font-size: 34rpx;
  font-weight: 600;
}

.picker-close {
  font-size: 48rpx;
  color: rgba(255, 255, 255, 0.9);
  padding: 0 20rpx;
}

.picker-body {
  padding: 30rpx;
}

/* 响应式调整 */
@media screen and (max-width: 768px) {
  .nav-bar {
    padding: 40rpx 20rpx 20rpx;
  }
  
  .date-section {
    margin: 0 20rpx;
    padding: 25rpx;
  }
  
  .main-content {
    padding: 20rpx;
  }
  
  .realtime-grid {
    grid-template-columns: 1fr;
  }
  
  .device-grid {
    grid-template-columns: 1fr;
  }
  
  .action-bar {
    flex-direction: column;
  }
}
</style>