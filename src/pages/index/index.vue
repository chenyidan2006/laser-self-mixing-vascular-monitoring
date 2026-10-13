<template>
  <view class="container">
    <view class="header">
      <view class="user-info">
        <view class="avatar-wrapper">
          <text class="avatar">{{ displayName.charAt(0) }}</text>
        </view>
        <view class="greeting">
          <text class="greeting-text">{{ greeting }} {{ displayName }}</text>
          <text class="date-text">{{ currentDate }}</text>
        </view>
      </view>
      <view class="header-actions">
        <view class="lang-switch" @click="toggleLanguage">
          <text>{{ currentLang === 'zh' ? '英文' : 'Chinese' }}</text>
        </view>
      </view>
    </view>

    <view class="status-section">
      <view class="connection-status" :class="{ connected: isConnected, disconnected: !isConnected }">
        <view class="status-dot"></view>
        <text class="status-text">{{ deviceStatusText }}</text>
        <text class="status-hint">{{ deviceStatusHint }}</text>
      </view>

    </view>

    <!-- Latest stored measurement remains visible while the ESP32 is offline. -->
    <view class="main-content">
      <view class="realtime-section">
        <view class="section-header">
          <text class="section-title">{{ tr('Latest reading') }}</text>
          <view class="live-badge" :class="{ offline: !isConnected }">
            <view class="live-dot"></view>
            <text>{{ tr('Saved measurement') }}</text>
          </view>
        </view>

        <view class="metrics-vertical">
          <view class="metric-row heart-rate">
            <view class="metric-left">
              <text class="metric-icon">❤️</text>
              <view class="metric-label-group">
                <text class="metric-label">{{ t('heartRate') }}</text>
                <text class="metric-status" :class="getHeartRateStatus(realtimeData.heart_rate)">
                  {{ getHeartRateText(realtimeData.heart_rate) }}
                </text>
              </view>
            </view>
            <view class="metric-right">
              <text class="metric-value">{{ realtimeData.heart_rate || '--' }}</text>
              <text class="metric-unit">{{ tr('bpm') }}</text>
            </view>
          </view>

          <view class="metric-row blood-pressure">
            <view class="metric-left">
              <text class="metric-icon">🩸</text>
              <view class="metric-label-group">
                <text class="metric-label">{{ t('bloodPressure') }}</text>
                <text class="metric-status" :class="getBPStatus(realtimeData.systolic_bp)">
                  {{ getBPText(realtimeData.systolic_bp) }}
                </text>
              </view>
            </view>
            <view class="metric-right">
              <text class="metric-value">
                {{ realtimeData.systolic_bp || '--' }}/{{ realtimeData.diastolic_bp || '--' }}
              </text>
              <text class="metric-unit">{{ tr('mmHg') }}</text>
            </view>
          </view>

          <view class="metric-row spo2">
            <view class="metric-left">
              <text class="metric-icon">🫁</text>
              <view class="metric-label-group">
                <text class="metric-label">{{ tr('SpO2') }}</text>
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

      <view class="hour-panel">
        <view class="panel-heading"><view><text class="panel-title">{{ tr('Last hour') }}</text><text class="panel-sub">{{ tr('Measurement averages') }}</text></view><text class="count-chip">{{ miniChartData.length }} {{ tr('measurements') }}</text></view>
        <view class="hour-stats"><view><text class="small-label">{{ tr('Heart Rate') }}</text><text class="stat-value red">{{ hourAverage('heart_rate') }}</text><text class="stat-unit">bpm</text></view><view><text class="small-label">{{ tr('Blood Pressure') }}</text><text class="stat-value violet">{{ hourAverage('systolic_bp') }}<text class="stat-slash">/</text>{{ hourAverage('diastolic_bp') }}</text><text class="stat-unit">mmHg</text></view><view><text class="small-label">{{ tr('SpO2') }}</text><text class="stat-value teal">{{ hourAverage('spo2') }}</text><text class="stat-unit">%</text></view></view>
        <view class="metric-switch"><text v-for="metric in hourMetrics" :key="metric.key" :class="{active:hourMetric===metric.key}" @click="hourMetric=metric.key">{{ tr(metric.title) }}</text></view>
        <MeasurementChart v-if="miniChartData.length" chart-id="hourChart" :labels="miniChartData.map(r => r.measure_time.slice(11,16))" :x-values="miniChartData.map(r => measurementTime(r.measure_time))" :x-domain="[chartNow-3600000,chartNow]" :axis-ticks="hourTicks" :series="hourSeries" :minimum-span="hourMetric==='spo2'?2:10" />
        <view v-else class="chart-empty"><text>⌁</text><text>{{ tr('No measurements in the last hour') }}</text><text class="panel-sub">{{ tr('The latest saved reading remains above') }}</text></view>
        <view class="chart-foot"><view class="subtle-dot"/><text>{{ tr('Each dot is one measurement · 10-minute ticks') }}</text></view>
      </view>

      <view class="health-tips-section">
        <view class="section-header">
          <text class="section-title">{{ tr("💡 Health Tips") }}</text>
        </view>
        <view class="tips-card">
          <view class="tips-header">
            <text class="tips-icon">{{ currentTip.icon }}</text>
            <text class="tips-category">{{ tr(currentTip.category) }}</text>
          </view>
          <text class="tips-content">{{ tr(currentTip.content) }}</text>
          <view class="tips-footer">
            <text class="tips-date">{{ tr(currentTip.date) }}</text>
            <view class="tips-action" @click="refreshTip">
              <text>{{ tr("换一个") }}</text>
              <text class="refresh-icon">🔄</text>
            </view>
          </view>
        </view>
      </view>

    </view>
    <view class="goals-panel" @click="goToGoalSettings"><view class="panel-heading"><view><text class="panel-title">{{ tr('Daily Goals') }}</text><text class="panel-sub">{{ tr('Your daily wellness plan') }}</text></view><text class="manage-link">{{ tr('Manage') }} ›</text></view><view class="goal-grid"><view class="home-goal" v-for="goal in homeGoals" :key="goal.key"><view class="home-goal-top"><text :style="{color:goal.color}">{{ goal.icon }}</text><text>{{ tr(goal.title) }}</text></view><view class="home-goal-numbers"><text>{{ goal.value }}</text><text> / {{ goal.target }} {{ tr(goal.unit) }}</text></view><view class="home-goal-track"><view :style="{width:goal.progress+'%',background:goal.color}" /></view></view></view></view>
    <view v-if="!hasData" class="chart-empty"><text>⌁</text><text>{{ tr(selectedDeviceId ? 'Waiting for the first measurement' : 'No device bound') }}</text></view>
  </view>
</template>

<script>
import { t as translate, getLang } from '@/utils/i18n.js';
import MeasurementChart from '@/components/MeasurementChart.vue';
import {lastHour, average, validNumber, measurementTime, tenMinuteTicks} from '@/utils/measurements.js';
import {goalDefinitions,readGoals,saveGoals,goalProgress} from '@/utils/goals.js';
import {getDataUrl,getAuthUrl} from '@/config/api.js';

export default {
  components:{MeasurementChart},
  data(){return {chartNow:Date.now(),goalState:{targets:{},values:{}},hourMetric:'heart_rate',hourMetrics:[{key:'heart_rate',title:'Heart Rate'},{key:'blood_pressure',title:'Blood Pressure'},{key:'spo2',title:'SpO2'}],user:null,currentLang:getLang(),selectedDeviceId:'',isConnected:false,hasData:false,lastMeasurementTime:null,realtimeData:{},miniChartData:[],healthTips:[],currentTip:{},dailyGoal:{heartRateChecked:0,heartRateTarget:3,progress:0},timer:null,refreshVersion:0};},
  computed:{
    homeGoals(){return goalDefinitions.map(d=>({...d,value:this.goalState.values[d.key]||0,target:this.goalState.targets[d.key]||d.target,progress:goalProgress(this.goalState.values[d.key]||0,this.goalState.targets[d.key]||d.target)}));},
    hourSeries(){return this.hourMetric==='blood_pressure'?[{name:this.tr('Systolic'),color:'#e56d7e',data:this.miniChartData.map(r=>r.systolic_bp)},{name:this.tr('Diastolic'),color:'#38afa3',data:this.miniChartData.map(r=>r.diastolic_bp)}]:[{name:this.tr(this.hourMetric==='heart_rate'?'Heart Rate':'SpO2'),color:this.hourMetric==='heart_rate'?'#e56d7e':'#38afa3',data:this.miniChartData.map(r=>r[this.hourMetric])}];},
    hourTicks(){return tenMinuteTicks(this.chartNow-3600000,this.chartNow);},
    displayName(){const name=this.user?.nickname || this.tr('User');return this.currentLang==='zh' && /[A-Za-z]/.test(name)?this.tr('User'):name;},
    greeting(){const hour=new Date().getHours();return this.tr(hour<12?'Good morning':hour<18?'Good afternoon':'Good evening');},
    currentDate(){const now=new Date();const weekdays=['Sunday','Monday','Tuesday','Wednesday','Thursday','Friday','Saturday'];return `${this.tr(weekdays[now.getDay()])} ${now.getMonth()+1}/${now.getDate()}`;},
    deviceStatusText(){return this.tr(!this.selectedDeviceId?'No device bound':this.isConnected?'Online':'Offline · showing latest reading');},
    deviceStatusHint(){return this.lastMeasurementTime?`${this.tr('Latest measurement')}: ${this.lastMeasurementTime}`:this.tr('Waiting for the first measurement');}
  },
  onShow(){this.currentLang=getLang();this.user=uni.getStorageSync('user');this.initHealthTips();this.initDailyGoal();this.restoreLatest();this.refresh();clearInterval(this.timer);this.timer=setInterval(()=>this.refresh(),30000);},
  onHide(){clearInterval(this.timer);this.timer=null;this.refreshVersion++;},
  onUnload(){clearInterval(this.timer);this.refreshVersion++;},
  methods:{
    measurementTime,
    t(key){return translate(key);},
    request(url,data={}){return new Promise((resolve,reject)=>uni.request({url,data,success:r=>r.data.errcode===0?resolve(r.data.data):reject(Error(r.data.msg)),fail:reject}));},
    async refresh(){
      this.chartNow=Date.now();
      const version=++this.refreshVersion;
      try{
        const devices=await this.request(getAuthUrl('devices'));
        if(version!==this.refreshVersion)return;
        const selected=uni.getStorageSync('selectedDeviceId');
        const device=devices.find(d=>(d.deviceId||d.device_id)===selected)||devices[0];
        if(!device){this.selectedDeviceId='';this.isConnected=false;this.hasData=false;this.miniChartData=[];this.dailyGoal.heartRateChecked=0;this.dailyGoal.progress=0;uni.removeStorageSync('selectedDeviceId');return;}
        const id=device.deviceId||device.device_id;
        if(id!==this.selectedDeviceId){this.hasData=false;this.realtimeData={};this.miniChartData=[];this.lastMeasurementTime=null;}
        this.selectedDeviceId=id;uni.setStorageSync('selectedDeviceId',id);
        const now=new Date(),historyStart=new Date(now.getTime()-30*86400000);
        const ymd=d=>`${d.getFullYear()}-${String(d.getMonth()+1).padStart(2,'0')}-${String(d.getDate()).padStart(2,'0')}`;
        const results=await Promise.allSettled([
          this.request(getDataUrl('deviceStatus'),{deviceId:id}),
          this.request(getDataUrl('realtime'),{deviceId:id}),
          this.request(getDataUrl('rangeRecords'),{deviceId:id,startDate:ymd(historyStart),endDate:ymd(now)})
        ]);
        if(version!==this.refreshVersion)return;
        this.isConnected=results[0].status==='fulfilled' && Boolean(results[0].value?.isConnected);
        if(results[2].status==='fulfilled'){
          const rows=results[2].value||[];
          const latest=results[1].status==='fulfilled'&&results[1].value?results[1].value:rows[0];
          if(latest){this.realtimeData=latest;uni.setStorageSync('latestMeasurement_'+this.user?.id+'_'+id,this.realtimeData);this.lastMeasurementTime=latest.measure_time;this.hasData=['heart_rate','systolic_bp','diastolic_bp','spo2'].some(key=>validNumber(latest[key]));}
          this.miniChartData=lastHour(rows);
          this.dailyGoal.heartRateChecked=new Set(rows.filter(r=>r.measure_time.slice(0,10)===ymd(now)).map(r=>r.id??r.measure_time)).size;this.goalState.values.measurements=this.dailyGoal.heartRateChecked;saveGoals(this.user?.id,this.goalState);
          this.dailyGoal.progress=Math.min(100,Math.round(this.dailyGoal.heartRateChecked/this.dailyGoal.heartRateTarget*100));
        } else this.miniChartData=lastHour(this.miniChartData);
      }catch(error){this.isConnected=false;this.miniChartData=lastHour(this.miniChartData);}
    },
    hourAverage(key){return average(this.miniChartData,key);},
    restoreLatest(){const id=uni.getStorageSync('selectedDeviceId');const cached=uni.getStorageSync('latestMeasurement_'+this.user?.id+'_'+id);if(id&&cached){this.selectedDeviceId=id;this.realtimeData=cached;this.lastMeasurementTime=cached.measure_time;this.hasData=['heart_rate','systolic_bp','diastolic_bp','spo2'].some(key=>validNumber(cached[key]));}},
    initDailyGoal(){this.goalState=readGoals(this.user?.id);const target=Number(uni.getStorageSync('measurementGoal_'+this.user?.id))||3;this.dailyGoal.heartRateTarget=target;},
    initHealthTips(){this.healthTips=[{icon:'💧',category:'Hydration',content:'Drink at least 8 glasses of water daily to maintain proper hydration and support metabolism.',date:'Today'},{icon:'🏃',category:'Exercise',content:'Aim for 30 minutes of moderate exercise daily, such as walking, jogging, or cycling.',date:'Today'},{icon:'😴',category:'Sleep',content:'Get 7-8 hours of quality sleep each night to allow your body to rest and rejuvenate.',date:'Today'}];this.currentTip=this.healthTips[0];},
    refreshTip(){const index=this.healthTips.indexOf(this.currentTip);this.currentTip=this.healthTips[(index+1)%this.healthTips.length];},
    toggleLanguage(){uni.setStorageSync('language',this.currentLang==='zh'?'en':'zh');uni.reLaunch({url:'/pages/index/index'});},
    goToGoalSettings(){uni.navigateTo({url:'/pages/goal-settings/goal-settings'});},
    getHeartRateStatus(hr){return !validNumber(hr)?'unknown':hr<60?'low':hr>100?'high':'normal';},
    getHeartRateText(hr){return !validNumber(hr)?'--':this.t(hr<60?'bradycardia':hr>100?'tachycardia':'normal');},
    getBPStatus(sbp){return !validNumber(sbp)?'unknown':sbp<120?'normal':sbp<140?'elevated':sbp<160?'high':'critical';},
    getBPText(sbp){return !validNumber(sbp)?'--':this.t(sbp<120?'normal':sbp<140?'elevated':sbp<160?'high':'critical');},
    getSpO2Status(value){return !validNumber(value)?'unknown':value>=95?'normal':value>=90?'low':'critical';},
    getSpO2Text(value){return !validNumber(value)?'--':this.t(value>=95?'normal':value>=90?'low':'critical');}
  }
};
</script>

<style lang="scss" scoped>
.container {
  min-height: 100vh;
  background: linear-gradient(180deg, #f0f9ff 0%, #f8fafc 50%, #ffffff 100%);
  padding: 0 24rpx calc(140rpx + env(safe-area-inset-bottom));
  box-sizing: border-box;
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

.health-tips-section {
  background: white;
  border-radius: 24rpx;
  padding: 28rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.tips-card {
  background: linear-gradient(135deg, #f0fdf4 0%, #dcfce7 100%);
  border-radius: 16rpx;
  padding: 24rpx;
}

.tips-header {
  display: flex;
  align-items: center;
  gap: 12rpx;
  margin-bottom: 16rpx;
}

.tips-icon {
  font-size: 36rpx;
}

.tips-category {
  font-size: 24rpx;
  font-weight: 600;
  color: #16a34a;
  background: rgba(22, 163, 74, 0.1);
  padding: 4rpx 16rpx;
  border-radius: 8rpx;
}

.tips-content {
  font-size: 28rpx;
  color: #475569;
  line-height: 1.6;
  margin-bottom: 16rpx;
}

.tips-footer {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.tips-date {
  font-size: 22rpx;
  color: #94a3b8;
}

.tips-action {
  display: flex;
  align-items: center;
  gap: 8rpx;
  font-size: 24rpx;
  color: #6366f1;
  font-weight: 500;
}

.refresh-icon {
  font-size: 24rpx;
}

.quick-actions-section {
  background: white;
  border-radius: 24rpx;
  padding: 28rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.actions-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 20rpx;
}

.action-card {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 12rpx;
  padding: 24rpx 16rpx;
  background: #f8fafc;
  border-radius: 16rpx;
  transition: all 0.3s ease;
}

.action-card:active {
  transform: scale(0.95);
  background: #f1f5f9;
}

.action-icon-wrapper {
  font-size: 48rpx;
}

.action-name {
  font-size: 26rpx;
  font-weight: 600;
  color: #1e293b;
}

.action-desc {
  font-size: 20rpx;
  color: #94a3b8;
}

.daily-goal-section {
  background: white;
  border-radius: 24rpx;
  padding: 28rpx;
  margin-bottom: 24rpx;
  box-shadow: 0 4rpx 24rpx rgba(0, 0, 0, 0.06);
}

.goal-progress {
  font-size: 28rpx;
  font-weight: 600;
  color: #6366f1;
}

.goal-card {
  display: flex;
  flex-direction: column;
  gap: 24rpx;
}

.goal-item {
  display: flex;
  flex-direction: column;
  gap: 12rpx;
}

.goal-icon {
  font-size: 32rpx;
}

.goal-info {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.goal-label {
  font-size: 26rpx;
  font-weight: 600;
  color: #1e293b;
}

.goal-status {
  font-size: 24rpx;
  color: #64748b;
}

.goal-bar-container {
  height: 12rpx;
  background: #e2e8f0;
  border-radius: 6rpx;
  overflow: hidden;
}

.goal-bar {
  height: 100%;
  background: linear-gradient(90deg, #6366f1 0%, #8b5cf6 100%);
  border-radius: 6rpx;
  transition: width 0.5s ease;
}

.empty-state {
  display: flex;
  flex-direction: column;
  gap: 32rpx;
}

.offline-device-card {
  padding: 34rpx 30rpx 28rpx;
  border-radius: 30rpx;
  background: linear-gradient(145deg, #ffffff 0%, #f8fafc 100%);
  border: 2rpx solid #e2e8f0;
  box-shadow: 0 12rpx 38rpx rgba(15, 23, 42, 0.08);
}

.offline-device-top {
  display: flex;
  align-items: center;
}

.offline-device-icon {
  width: 76rpx;
  height: 76rpx;
  border-radius: 22rpx;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #fff7ed;
  color: #ea580c;
  font-size: 50rpx;
  font-weight: 700;
}

.offline-device-copy {
  flex: 1;
  display: flex;
  flex-direction: column;
  margin-left: 20rpx;
}

.offline-device-label {
  color: #64748b;
  font-size: 23rpx;
}

.offline-device-name {
  margin-top: 4rpx;
  color: #0f172a;
  font-size: 32rpx;
  font-weight: 700;
}

.offline-chip {
  padding: 8rpx 16rpx;
  border-radius: 999rpx;
  background: #fff7ed;
}

.offline-chip text {
  color: #c2410c;
  font-size: 22rpx;
  font-weight: 600;
}

.offline-divider {
  height: 2rpx;
  margin: 28rpx 0 24rpx;
  background: #e2e8f0;
}

.offline-message {
  display: flex;
  flex-direction: column;
}

.offline-message-title {
  color: #1e293b;
  font-size: 30rpx;
  font-weight: 700;
}

.offline-message-desc {
  margin-top: 12rpx;
  color: #64748b;
  font-size: 25rpx;
  line-height: 1.65;
}

.offline-actions {
  display: flex;
  gap: 18rpx;
  margin-top: 28rpx;
}

.offline-actions button {
  flex: 1;
  margin: 0;
  padding: 18rpx 0;
  border-radius: 16rpx;
  font-size: 26rpx;
  font-weight: 600;
  line-height: 1.25;
}

.offline-primary {
  border: 0;
  color: #ffffff;
  background: #2563eb;
}

.offline-secondary {
  border: 2rpx solid #dbeafe;
  color: #2563eb;
  background: #eff6ff;
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

.device-empty-card {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 36rpx 30rpx;
  margin-bottom: 26rpx;
  border-radius: 28rpx;
  background: linear-gradient(135deg, #eef2ff 0%, #f5f3ff 100%);
  border: 2rpx solid #ddd6fe;
}

.device-empty-badge {
  padding: 8rpx 18rpx;
  border-radius: 999rpx;
  background: #ddd6fe;
  color: #5b21b6;
  font-size: 22rpx;
  font-weight: 600;
}

.device-empty-title {
  margin-top: 18rpx;
  font-size: 34rpx;
  font-weight: 700;
  color: #312e81;
}

.device-empty-desc {
  margin-top: 14rpx;
  color: #5b6478;
  font-size: 26rpx;
  line-height: 1.6;
}

.device-bind-button {
  width: 100%;
  margin-top: 24rpx;
  border: none;
  border-radius: 18rpx;
  background: linear-gradient(135deg, #6366f1, #8b5cf6);
  color: #fff;
  font-size: 28rpx;
  font-weight: 600;
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

.hour-panel,.goals-panel{background:#fff;border-radius:28rpx;padding:30rpx;margin:28rpx 0;box-shadow:0 5rpx 24rpx #26385505}.panel-heading{display:flex;align-items:center;justify-content:space-between;margin-bottom:28rpx;gap:16rpx}.panel-title{display:block;font-size:31rpx;font-weight:700;color:#26314a}.panel-sub{display:block;font-size:22rpx;color:#969fb0;margin-top:9rpx}.count-chip{background:#f1effc;color:#8570db;font-size:22rpx;border-radius:20rpx;padding:10rpx 16rpx;white-space:nowrap}.hour-stats{display:flex;margin-bottom:30rpx}.hour-stats>view{flex:1;display:flex;align-items:center;flex-direction:column;border-right:1px solid #f0f2f7}.hour-stats>view:last-child{border:0}.small-label{color:#8791a5;font-size:23rpx}.stat-value{font-size:35rpx;font-weight:700;margin:15rpx 0 6rpx;white-space:nowrap}.stat-slash{font-size:22rpx;margin:0 3rpx}.stat-unit{font-size:21rpx;color:#9aa3b3}.red{color:#e56d7e}.violet{color:#8973d8}.teal{color:#38afa3}.metric-switch{display:flex;background:#f5f6fa;border-radius:18rpx;padding:6rpx;margin-bottom:24rpx}.metric-switch>text{flex:1;text-align:center;font-size:23rpx;padding:13rpx 6rpx;color:#9099ad;border-radius:14rpx}.metric-switch>text.active{background:white;color:#7661d9;box-shadow:0 3rpx 9rpx #2638550d;font-weight:600}.chart-foot{display:flex;gap:12rpx;align-items:center;font-size:21rpx;color:#9ca5b5;border-top:1px solid #f1f3f8;padding-top:20rpx;margin-top:16rpx}.subtle-dot{width:10rpx;height:10rpx;border-radius:50%;background:#b9c0cf}.chart-empty{height:230rpx;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:12rpx;color:#9ca5b5;font-size:25rpx}.chart-empty>text:first-child{font-size:70rpx;color:#c4bcf2}.wave-link{display:flex;align-items:center;gap:22rpx;padding:28rpx;background:white;border-radius:26rpx;margin:28rpx 0}.wave-icon{background:#eeebfc;color:#8870d8;width:80rpx;height:80rpx;border-radius:22rpx;text-align:center;line-height:80rpx;font-size:55rpx}.link-arrow{margin-left:auto;font-size:44rpx;color:#a1aabd}.manage-link{color:#8870d8;font-size:24rpx;white-space:nowrap}.goal-grid{display:grid;grid-template-columns:1fr 1fr;gap:20rpx}.home-goal{background:#f8f9fc;border-radius:22rpx;padding:22rpx}.home-goal-top{display:flex;align-items:center;gap:12rpx;font-size:24rpx;color:#748097}.home-goal-top>text:first-child{font-size:33rpx}.home-goal-numbers{margin:18rpx 0;font-size:21rpx;color:#9aa3b5}.home-goal-numbers>text:first-child{color:#34425d;font-size:34rpx;font-weight:600}.home-goal-track{height:8rpx;border-radius:20rpx;background:#e9edf4;overflow:hidden}.home-goal-track>view{height:100%;border-radius:20rpx}.connection-status{background:#f2f4f9!important;border-radius:22rpx!important;align-items:flex-start!important}.status-text{color:#6c7890!important;font-size:25rpx!important}.status-hint{font-size:22rpx!important;color:#9aa3b2!important}.live-badge,.live-badge.offline{background:#f0eefb!important;color:#8570d2!important}.live-badge .live-dot{background:#aa97e3!important}.connection-status.disconnected .status-dot{background:#a7afbd!important}

</style>
