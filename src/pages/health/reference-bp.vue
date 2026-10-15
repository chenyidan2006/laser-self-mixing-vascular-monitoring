<template>
  <view class="page">
    <view class="nav">
      <text class="back" @click="goBack">‹</text>
      <text class="title">录入袖带血压真值</text>
      <view class="placeholder" />
    </view>

    <view class="tip">选择当前设备已上传的原始帧，录入同一时段袖带测量的真值。标注独立保存，不会修改模型预测，也不会自动训练。</view>
    <view v-if="loading" class="empty">正在加载…</view>
    <view v-else-if="!sessions.length" class="empty">当前设备没有待标注的测量记录。</view>

    <view v-for="session in sessions" :key="session.id" class="card">
      <view class="time">{{ session.measureTime }}</view>
      <view class="meta">{{ session.deviceId }} · {{ session.file }}</view>
      <view class="inputs">
        <input v-model="form[session.id].subjectId" placeholder="受试者编号，如 P001" />
      </view>
      <view class="inputs">
        <input v-model="form[session.id].sbp" type="number" placeholder="收缩压 SBP" />
        <input v-model="form[session.id].dbp" type="number" placeholder="舒张压 DBP" />
        <input v-model="form[session.id].hr" type="number" placeholder="袖带心率 HR" />
      </view>
      <button class="save" :loading="savingId === session.id" @click="save(session)">保存真值</button>
    </view>
  </view>
</template>

<script>
import { getTrainingUrl } from '@/config/api.js';

export default {
  data: () => ({ sessions: [], form: {}, loading: false, savingId: null }),
  onShow() { this.load(); },
  methods: {
    goBack() { uni.navigateBack(); },
    deviceId() { return uni.getStorageSync('selectedDeviceId') || ''; },
    headers() { return { Authorization: `Bearer ${uni.getStorageSync('token')}`, 'Content-Type': 'application/json' }; },
    async load() {
      const deviceId = this.deviceId();
      this.sessions = [];
      if (!deviceId) { uni.showToast({ title: '请先选择设备', icon: 'none' }); return; }
      this.loading = true;
      try {
        const response = await new Promise((resolve, reject) => uni.request({
          url: getTrainingUrl('pendingSessions'), data: { deviceId }, header: this.headers(),
          success: (result) => resolve(result.data), fail: reject,
        }));
        if (response.errcode !== 0) throw new Error(response.msg || '读取失败');
        this.sessions = response.data || [];
        const savedSubjectId = uni.getStorageSync('trainingSubjectId') || '';
        this.form = Object.fromEntries(this.sessions.map((row) => [row.id, { subjectId: savedSubjectId, sbp: '', dbp: '', hr: '' }]));
      } catch (error) {
        uni.showToast({ title: error.message || '读取待标注测量失败', icon: 'none' });
      } finally { this.loading = false; }
    },
    async save(session) {
      const values = this.form[session.id];
      const sbp = Number(values.sbp), dbp = Number(values.dbp), hr = Number(values.hr);
      if (!Number.isInteger(sbp) || !Number.isInteger(dbp) || !Number.isInteger(hr) ||
          sbp < 60 || sbp > 260 || dbp < 30 || dbp > 160 || sbp <= dbp || hr < 25 || hr > 250 ||
          !/^[A-Za-z0-9_-]{1,64}$/.test(String(values.subjectId || ''))) {
        uni.showToast({ title: '请填写受试者编号、有效的 SBP、DBP 和心率', icon: 'none' }); return;
      }
      this.savingId = session.id;
      try {
        const response = await new Promise((resolve, reject) => uni.request({
          url: `${getTrainingUrl('referenceBase')}/sessions/${session.id}/reference`, method: 'PUT', header: this.headers(),
          data: { deviceId: session.deviceId, subjectId: String(values.subjectId).trim(), referenceSbp: sbp, referenceDbp: dbp, referenceHr: hr },
          success: (result) => resolve(result.data), fail: reject,
        }));
        if (response.errcode !== 0) throw new Error(response.msg || '保存失败');
        uni.setStorageSync('trainingSubjectId', String(values.subjectId).trim());
        uni.showToast({ title: '真值已保存', icon: 'success' });
        await this.load();
      } catch (error) {
        uni.showToast({ title: error.message || '保存失败', icon: 'none' });
      } finally { this.savingId = null; }
    },
  },
};
</script>

<style lang="scss" scoped>
.page { min-height: 100vh; background: #f5f7fa; padding: 0 24rpx 60rpx; }
.nav { height: 120rpx; display: flex; align-items: center; justify-content: space-between; }
.back { font-size: 56rpx; width: 80rpx; color: #1e293b; }.title { font-size: 32rpx; font-weight: 600; }.placeholder { width: 80rpx; }
.tip, .empty, .card { background: #fff; border-radius: 18rpx; padding: 24rpx; margin-bottom: 20rpx; }
.tip { font-size: 26rpx; color: #64748b; line-height: 1.55; }.empty { color: #94a3b8; text-align: center; }
.time { font-size: 30rpx; font-weight: 600; color: #1e293b; }.meta { margin-top: 8rpx; font-size: 24rpx; color: #64748b; }
.inputs { display: flex; gap: 12rpx; margin: 22rpx 0; }.inputs input { flex: 1; min-width: 0; border: 2rpx solid #e2e8f0; border-radius: 10rpx; padding: 16rpx 10rpx; font-size: 26rpx; }
.save { background: #2b85e4; color: #fff; font-size: 28rpx; }
</style>
