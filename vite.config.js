// 如果还没导入,需要先导入
import { defineConfig } from 'vite'
import uni from '@dcloudio/vite-plugin-uni'
import path from 'path'  // 用于路径处理

export default defineConfig({  
  // 插件配置
  plugins: [uni()],
  
  // CSS 配置
  css: {
    preprocessorOptions: {
      scss: {
        additionalData: `@import "uview-plus/theme.scss";`,
        includePaths: [path.resolve(__dirname, 'styles')]
      }
    }
  }
})
