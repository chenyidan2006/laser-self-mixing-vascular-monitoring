export function backToTab(tab='index') {
  const url=tab==='profile'?'/pages/profile/profile':'/pages/index/index';
  const fallback=()=>uni.switchTab({url,fail:()=>uni.reLaunch({url})});
  if(typeof getCurrentPages==='function' && getCurrentPages().length>1)uni.navigateBack({delta:1,fail:fallback});
  else fallback();
}
