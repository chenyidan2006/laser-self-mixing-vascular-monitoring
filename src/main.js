import {tr, getLang, syncNavigation} from '@/utils/i18n.js';
// main.js
import App from './App.vue'
import { createSSRApp } from 'vue'
import config from '../config/config.js'

// 引入 uni-app 的样式
import './uni.scss'
import uviewPlus from 'uview-plus'
import 'uview-plus/index.scss'; // 引入全部样式(包含变量定义)

export function createApp() {
  const app = createSSRApp(App)
  app.mixin({data:()=>({uiLang:getLang()}),onShow(){this.uiLang=getLang();syncNavigation();},methods:{tr(text){return tr(text,this.uiLang);},switchLanguage(){this.uiLang=this.uiLang==='zh'?'en':'zh';uni.setStorageSync('language',this.uiLang);uni.reLaunch({url:'/pages/login/login'});}}});
  
  // 全局配置
  app.config.productionTip = false
  uni.config = config
  
  // 全局错误处理
  app.config.errorHandler = (err, vm, info) => {
    console.error('[Vue 错误]', err, info)
    // 可以在这里上报错误
  }
  
  // 全局警告处理
  app.config.warnHandler = (msg, vm, trace) => {
    console.warn('[Vue 警告]', msg, trace)
  }
  
  return { app }
}

export default {
  createApp
}
