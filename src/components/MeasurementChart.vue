<template>
  <view>
    <view class="legend"><text v-for="line in series" :key="line.name" :style="{color: line.color}">● {{ line.name }}</text></view>
    <canvas :id="chartId" :canvas-id="chartId" class="plot" @touchstart="touch" />
    <text class="selection" v-if="selected">{{ selected }}</text>
  </view>
</template>
<script>
import { axisBounds } from '@/utils/measurements.js';
export default {
  props: { chartId: {type: String, required: true}, labels: {type: Array, default: () => []}, xValues: {type: Array, default: () => []}, xDomain: {type: Array, default: () => []}, axisTicks: {type: Array, default: () => []}, showMarkers: {type:Boolean,default:true}, series: {type: Array, default: () => []}, minimumSpan: {type: Number, default: 10} },
  data: () => ({ selected: '', width: 300 }),
  watch: { labels: {handler() {this.selected='';this.$nextTick(this.draw);}, deep: true}, series: {handler() {this.$nextTick(this.draw);}, deep: true}, xDomain: {handler() {this.$nextTick(this.draw);}, deep: true}, axisTicks: {handler() {this.$nextTick(this.draw);}, deep: true} },
  mounted() { this.$nextTick(this.draw); },
  methods: {
    draw() {
      uni.createSelectorQuery().in(this).select('#' + this.chartId).boundingClientRect(rect => {
        if (!rect) return;
        this.width = rect.width;
        const ctx = uni.createCanvasContext(this.chartId, this);
        const w = rect.width, h = rect.height, left = 48, right = w - 14, top = 14, bottom = h - 30;
        const bounds = axisBounds(this.series.flatMap(line => line.data), this.minimumSpan);
        ctx.clearRect(0, 0, w, h); ctx.setFontSize(10);
        for (let i = 0; i <= 4; i++) {
          const y = bottom - (bottom - top) * i / 4;
          ctx.setStrokeStyle('#e2e8f0'); ctx.beginPath(); ctx.moveTo(left,y); ctx.lineTo(right,y); ctx.stroke();
          const decimals = Math.min(5, Math.max(0, 2 - Math.floor(Math.log10(bounds.max - bounds.min))));
          ctx.setFillStyle('#64748b'); ctx.fillText((bounds.min + (bounds.max-bounds.min)*i/4).toFixed(decimals), 0,y+3);
        }
        const x = i => left + (right-left) * this.position(i);
        this.series.forEach(line => {
          ctx.setStrokeStyle(line.color); ctx.setFillStyle(line.color); ctx.setLineWidth(2); ctx.beginPath(); let connected = false;
          line.data.forEach((value,i) => {
            if (value == null || !Number.isFinite(Number(value))) {connected = false; return;}
            const y = bottom - (Number(value)-bounds.min)/(bounds.max-bounds.min)*(bottom-top);
            if (connected) ctx.lineTo(x(i),y); else ctx.moveTo(x(i),y);
            connected = true;
          }); ctx.stroke();
          if (this.showMarkers) line.data.forEach((value,i) => {
            if (value == null || !Number.isFinite(Number(value))) return;
            const y = bottom - (Number(value)-bounds.min)/(bounds.max-bounds.min)*(bottom-top);
            ctx.beginPath(); ctx.arc(x(i),y,3,0,Math.PI*2); ctx.fill();
          });
        });
        const step = Math.max(1, Math.ceil(this.labels.length/4));
        const ticks=this.axisTicks.length?this.axisTicks.map(t=>({label:t.label,x:left+(right-left)*this.valuePosition(t.value)})):this.labels.map((label,i)=>({label,x:x(i),index:i})).filter(t=>t.index%step===0||t.index===this.labels.length-1);
        let previousEnd=-Infinity;
        for(const tick of ticks) {
          const text=String(tick.label),width=text.length*6;
          const start=Math.min(w-width,Math.max(0,tick.x-width/2));
          if(start<previousEnd+6)continue;
          ctx.setFillStyle('#64748b');ctx.fillText(text,start,h-8);previousEnd=start+width;
        }
        ctx.draw();
      }).exec();
    },
    position(index) {
      const first=this.xValues[0],last=this.xValues[this.xValues.length-1];
      if(this.xValues.length===this.labels.length && Number.isFinite(first) && Number.isFinite(last))return this.valuePosition(this.xValues[index]);
      return this.labels.length===1?0.5:index/Math.max(1,this.labels.length-1);
    },
    valuePosition(value) {
      const first=this.xDomain.length===2?this.xDomain[0]:this.xValues[0],last=this.xDomain.length===2?this.xDomain[1]:this.xValues[this.xValues.length-1];
      return Number.isFinite(first)&&Number.isFinite(last)&&last>first?(value-first)/(last-first):0.5;
    },
    touch(event) {
      if (!this.labels.length) return;
      const point = event.touches?.[0]; if (!point) return;
      const fraction=((point.x ?? 48)-48)/(this.width-62);
      let index=0;for(let i=1;i<this.labels.length;i++)if(Math.abs(this.position(i)-fraction)<Math.abs(this.position(index)-fraction))index=i;
      this.selected = this.labels[index] + ' · ' + this.series.map(line => line.name + ': ' + (line.data[index] == null ? '--' : Number(line.data[index]).toFixed(2))).join(' / ');
    }
  }
};
</script>
<style scoped>
.plot {width:100%;height:240px;} .legend {display:flex;flex-wrap:wrap;gap:16px;font-size:13px;margin-bottom:10px;} .selection {font-size:12px;color:#475569;}
</style>
