export function appendAppLog(message) {
  try {
    const stored = uni.getStorageSync('appLogs');
    const rows = Array.isArray(stored) ? stored : [];
    rows.unshift({time:new Date().toLocaleString(),message:String(message).slice(0,300)});
    uni.setStorageSync('appLogs',rows.slice(0,100));
  } catch { /* Logging must not prevent normal requests. */ }
}
