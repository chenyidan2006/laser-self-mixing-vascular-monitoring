<template>
  <view class="trend-container">
    <view class="nav-bar">
      <view class="nav-back" @click="goBack">
        <text class="back-icon">‹</text>
        <text class="back-text">{{ tr("Back") }}</text>
      </view>
      <text class="nav-title">{{ tr("History Trends") }}</text>
      <view class="nav-placeholder"></view>
    </view>

    <view class="range-tabs">
      <view
        class="range-tab"
        :class="{ active: currentRange === 'day' }"
        @click="switchRange('day')"
      >{{ tr("Day") }}</view>
      <view
        class="range-tab"
        :class="{ active: currentRange === 'week' }"
        @click="switchRange('week')"
      >{{ tr("Week") }}</view>
      <view
        class="range-tab"
        :class="{ active: currentRange === 'month' }"
        @click="switchRange('month')"
      >{{ tr("Month") }}</view>
    </view>

    <view class="metric-tabs">
      <view
        class="metric-tab"
        :class="{ active: currentMetric === 'heart_rate' }"
        @click="switchMetric('heart_rate')"
      >
        <text class="metric-icon">❤️</text>
        <text>{{ tr("HR") }}</text>
      </view>
      <view
        class="metric-tab"
        :class="{ active: currentMetric === 'blood_pressure' }"
        @click="switchMetric('blood_pressure')"
      >
        <text class="metric-icon">🩸</text>
        <text>{{ tr("BP") }}</text>
      </view>
      <view
        class="metric-tab"
        :class="{ active: currentMetric === 'spo2' }"
        @click="switchMetric('spo2')"
      >
        <text class="metric-icon">🫁</text>
        <text>{{ tr("SpO2") }}</text>
      </view>
    </view>

    <view class="stats-card" v-if="currentMetric !== 'blood_pressure'">
      <view class="stat-item">
        <text class="stat-label">{{ tr("Min") }}</text>
        <text class="stat-value">{{ stats.min }}</text>
        <text class="stat-unit">{{ metricUnit }}</text>
      </view>
      <view class="stat-item main">
        <text class="stat-label">{{ tr("Avg") }}</text>
        <text class="stat-value">{{ stats.avg }}</text>
        <text class="stat-unit">{{ metricUnit }}</text>
      </view>
      <view class="stat-item">
        <text class="stat-label">{{ tr("Max") }}</text>
        <text class="stat-value">{{ stats.max }}</text>
        <text class="stat-unit">{{ metricUnit }}</text>
      </view>
    </view>

    <view class="stats-card bp-stats" v-else>
      <view class="stat-item"><text class="stat-label">{{ tr("Min") }}</text><text class="stat-value bp-value"><text class="sys">{{ stats.systolic.min }}</text>/<text class="dia">{{ stats.diastolic.min }}</text></text><text class="stat-unit">{{ tr("SYS / DIA") }}</text></view>
      <view class="stat-item main"><text class="stat-label">{{ tr("Avg") }}</text><text class="stat-value bp-value"><text class="sys">{{ stats.systolic.avg }}</text>/<text class="dia">{{ stats.diastolic.avg }}</text></text><text class="stat-unit">{{ tr("SYS / DIA") }}</text></view>
      <view class="stat-item"><text class="stat-label">{{ tr("Max") }}</text><text class="stat-value bp-value"><text class="sys">{{ stats.systolic.max }}</text>/<text class="dia">{{ stats.diastolic.max }}</text></text><text class="stat-unit">{{ tr("SYS / DIA") }}</text></view>
    </view>

    <view class="chart-card" >
      <view class="chart-header">
        <text class="chart-title">{{ metricName }} {{ tr('Trend') }}</text>
        <view class="chart-badge" :style="{ background: metricColor }">
          <text class="badge-text">{{ metricUnit }}</text>
        </view>
      </view>
      <MeasurementChart v-if="chartData.length" chart-id="trendChart" :labels="displayChartData.map(item => formatBarLabel(item.date))" :series="lineSeries" :x-values="displayChartData.map(item => dateValue(item.date))" :axis-ticks="chartAxisTicks" :minimum-span="currentMetric === 'spo2' ? 2 : 10" />
      <view class="chart-note" v-else>{{ tr('No data available') }}</view>
    </view>

    <view class="calendar-section" v-if="currentRange === 'month' && currentMetric !== 'blood_pressure'">
      <view class="section-header">
        <text class="section-title">{{ tr('Daily Averages') }} · {{ metricName }}</text>
        <view class="chart-badge" :style="{ background: metricColor }">
          <text class="badge-text">{{ metricUnit }}</text>
        </view>
      </view>
      <view class="calendar-grid">
        <view class="calendar-weekdays">
          <text v-for="(day,index) in [tr('Sun'),tr('Mon'),tr('Tue'),tr('Wed'),tr('Thu'),tr('Fri'),tr('Sat')]" :key="index" class="weekday">{{ day }}</text>
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
        <text class="legend-label">{{ tr('Low') }} {{ heatMin }}</text>
        <view class="legend-gradient" :style="{ background: metricGradient }"></view>
        <text class="legend-label">{{ tr('High') }} {{ heatMax }}</text>
      </view>
    </view>

    <view class="records-card">
      <view class="card-header">
        <text class="card-title">{{ tr("Recent Records") }}</text>
        <text class="card-count">{{ recordsList.length }} {{ tr('measurements') }}</text>
      </view>
      <view class="records-list">
        <view v-if="recordsList.length === 0" class="empty-records">
          <view class="empty-icon">📝</view>
          <text>{{ tr("No recent records") }}</text>
        </view>
        <view
          v-for="(item, index) in recordsList"
          :key="index"
          class="record-item"
        >
          <view class="record-date">{{ formatDate(item.measure_time) }}</view>
          <view class="record-values">
            <text class="record-value">❤️ {{ item.heart_rate || '--' }} {{ tr('bpm') }}</text>
            <text class="record-value">🩸 {{ item.systolic_bp || '--' }}/{{ item.diastolic_bp || '--' }} {{ tr('mmHg') }}</text>
            <text class="record-value">🫁 {{ item.spo2 || '--' }}%</text>
          </view>
        </view>
      </view>
    </view>
  </view>
</template>

<script>
import MeasurementChart from '@/components/MeasurementChart.vue';
import {validNumber} from '@/utils/measurements.js';
import {getDataUrl,getAuthUrl} from '@/config/api.js';
const blank=()=>({min:'--',avg:'--',max:'--'});
const stats=values=>{values=values.filter(validNumber).map(Number);return values.length?{min:Number(Math.min(...values).toFixed(1)),avg:Number((values.reduce((a,b)=>a+b,0)/values.length).toFixed(1)),max:Number(Math.max(...values).toFixed(1))}:blank();};
export default {
  components:{MeasurementChart},
  data:()=>({selectedDeviceId:'',currentRange:'week',currentMetric:'heart_rate',chartData:[],aggregatedData:[],calendarDays:[],recordsList:[],loadVersion:0,stats:{...blank(),systolic:blank(),diastolic:blank()}}),
  computed:{
    metricName(){return this.tr(this.currentMetric==='heart_rate'?'Heart Rate':this.currentMetric==='blood_pressure'?'Blood Pressure':'SpO2');},
    metricUnit(){return this.tr(this.currentMetric==='heart_rate'?'bpm':this.currentMetric==='blood_pressure'?'mmHg':'%');},
    metricColor(){return this.currentMetric==='spo2'?'#06b6d4':'#ef4444';},
    metricGradient(){return this.currentMetric==='spo2'?'linear-gradient(90deg,#ecfeff,#0891b2)':'linear-gradient(90deg,#fef2f2,#dc2626)';},
    displayChartData(){return this.currentRange==='day'?this.chartData:this.aggregatedData;},
    lineSeries(){const rows=this.displayChartData;return this.currentMetric==='blood_pressure'?[{name:this.tr('Systolic'),color:'#ef4444',data:rows.map(r=>r.systolic)},{name:this.tr('Diastolic'),color:'#14b8a6',data:rows.map(r=>r.diastolic)}]:[{name:this.metricName,color:this.metricColor,data:rows.map(r=>r.value)}];},
    heatValues(){return this.aggregatedData.map(r=>r.value).filter(validNumber).map(Number);},
    heatMin(){return this.heatValues.length?Math.min(...this.heatValues).toFixed(1):'--';},
    heatMax(){return this.heatValues.length?Math.max(...this.heatValues).toFixed(1):'--';},
    chartAxisTicks(){
      if(this.currentRange!=='week')return [];
      return this.aggregatedData.map(row=>({
        value:this.dateValue(row.date),
        label:String(Number(String(row.date).slice(8,10)))
      }));
    }
  },
  async onShow(){await this.loadSelectedDevice();this.loadData();},
  onHide(){this.loadVersion++;},
  methods:{
    goBack(){uni.switchTab({url:'/pages/index/index'});},
    switchRange(range){this.currentRange=range;this.loadData();},
    switchMetric(metric){this.currentMetric=metric;this.loadData();},
    formatYMD(d){return `${d.getFullYear()}-${String(d.getMonth()+1).padStart(2,'0')}-${String(d.getDate()).padStart(2,'0')}`;},
    dateValue(text){return new Date(String(text).replace(' ','T')).getTime();},
    formatBarLabel(text){if(this.currentRange==='day')return String(text).slice(11,16);return String(text).slice(5,10);},
    formatDate(text){return text?String(text).replace('T',' ').slice(5,19):'--';},
    request(url,data={}){return new Promise((resolve,reject)=>uni.request({url,data,success:r=>r.data.errcode===0?resolve(r.data.data||[]):reject(Error(r.data.msg)),fail:reject}));},
    async loadSelectedDevice(){
      try{const devices=await this.request(getAuthUrl('devices'));const stored=uni.getStorageSync('selectedDeviceId');const device=devices.find(d=>(d.deviceId||d.device_id)===stored)||devices[0];this.selectedDeviceId=device?.deviceId||device?.device_id||'';if(this.selectedDeviceId)uni.setStorageSync('selectedDeviceId',this.selectedDeviceId);else uni.removeStorageSync('selectedDeviceId');}catch(e){this.selectedDeviceId='';}
    },
    async loadData(){
      const version=++this.loadVersion,range=this.currentRange,metric=this.currentMetric;
      this.chartData=[];this.aggregatedData=[];this.calendarDays=[];this.recordsList=[];this.stats={...blank(),systolic:blank(),diastolic:blank()};
      if(!this.selectedDeviceId)return;
      const now=new Date(),start=new Date(now.getFullYear(),now.getMonth(),range==='month'?1:now.getDate()-(range==='week'?6:0));
      try{
        const records=await this.request(getDataUrl('rangeRecords'),{deviceId:this.selectedDeviceId,startDate:this.formatYMD(start),endDate:this.formatYMD(now)});
        if(version!==this.loadVersion)return;
        this.recordsList=records.slice(0,50);
        const rows=records.filter(r=>metric==='blood_pressure'?validNumber(r.systolic_bp)&&validNumber(r.diastolic_bp):validNumber(r[metric])).map(r=>metric==='blood_pressure'?{date:r.measure_time,systolic:Number(r.systolic_bp),diastolic:Number(r.diastolic_bp)}:{date:r.measure_time,value:Number(r[metric])}).sort((a,b)=>this.dateValue(a.date)-this.dateValue(b.date));
        this.chartData=rows;
        this.stats=metric==='blood_pressure'?{...blank(),systolic:stats(rows.map(r=>r.systolic)),diastolic:stats(rows.map(r=>r.diastolic))}:stats(rows.map(r=>r.value));
        const groups={};for(const row of rows)(groups[row.date.slice(0,10)] ||= []).push(row);
        const averages=[];
        for(const day=new Date(start);day<=now;day.setDate(day.getDate()+1)){
          const key=this.formatYMD(day),group=groups[key]||[],mean=k=>group.length?group.reduce((sum,r)=>sum+r[k],0)/group.length:null;
          averages.push(metric==='blood_pressure'?{date:key+' 12:00:00',systolic:mean('systolic'),diastolic:mean('diastolic')}:{date:key+' 12:00:00',value:mean('value')});
        }
        this.aggregatedData=rows.length?averages:[];
        const calendar=Array.from({length:new Date(now.getFullYear(),now.getMonth(),1).getDay()},()=>({day:'',value:null}));
        const map=new Map(averages.map(r=>[Number(r.date.slice(8,10)),r.value]));
        for(let d=1;d<=new Date(now.getFullYear(),now.getMonth()+1,0).getDate();d++)calendar.push({day:d,value:map.get(d)??null});
        this.calendarDays=calendar;
      }catch(e){if(version===this.loadVersion)uni.showToast({title:this.tr('Load failed'),icon:'none'});}
    },
    getHeatmapColor(value){const values=this.heatValues;if(!values.length)return '#f1f5f9';const lo=Math.min(...values),hi=Math.max(...values),fraction=hi===lo?0.5:(value-lo)/(hi-lo);const end=this.currentMetric==='spo2'?[8,145,178]:[220,38,38];return `rgb(${end.map(v=>Math.round(255+(v-255)*fraction)).join(',')})`;}
  }
};
</script>

<style lang="scss" scoped>
.trend-container {
  min-height: 100vh;
  background: #f8fafc;
  padding: 0 24rpx calc(140rpx + env(safe-area-inset-bottom));
  box-sizing: border-box;
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

.bp-value {
  font-size: 34rpx;
  white-space: nowrap;
}

.sys { color: #ef4444; }
.dia { color: #14b8a6; }

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
  width: 100%;
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
