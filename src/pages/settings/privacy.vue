<template><view class="page"><text class="back" @click="back">‹ 返回</text><text class="title">数据与隐私</text><view v-for="item in items" :key="item.title" class="card"><text class="heading">{{ item.title }}</text><text>{{ item.body }}</text></view></view></template>
<script>
export default {data:()=>({items:[
 {title:'账号与测量数据',body:'手机号、个人资料、设备绑定和测量记录保存在你配置的业务服务器；登录令牌及部分展示缓存保存在当前设备。'},
 {title:'原始信号与袖带真值',body:'原始干涉信号及其标注保存在服务器文件目录。标注通过匿名受试者编号关联原始帧，不会自动变成训练完成的模型。'},
 {title:'比赛演示数据',body:'本版本按项目要求保留算法失败后的演示兜底及血氧占位值。展示结果可能包含演示值，不能用于诊断或作为真实测量训练标签。'},
 {title:'AI 健康建议',body:'请求 AI 建议时，后端会把必要的资料和测量摘要发送到配置的 AI 服务；使用前请确认服务器和服务商配置。'},
 {title:'删除与传输',body:'退出登录不会删除服务器记录。需要清除服务器数据时请联系项目维护者。本项目默认地址使用 HTTP，正式部署请配置 HTTPS 后重新构建。'}
]}),methods:{back(){uni.navigateBack();}}};
</script>
<style scoped>.page{padding:36rpx;background:#f5f6fb;min-height:100vh}.back{color:#7763df}.title{display:block;font-size:40rpx;margin:30rpx 0}.card{background:white;padding:30rpx;border-radius:20rpx;margin-bottom:24rpx;line-height:1.8;font-size:26rpx}.heading{display:block;font-weight:600;margin-bottom:14rpx}</style>
