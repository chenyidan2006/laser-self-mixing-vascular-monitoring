import { defineConfig } from 'vite'
import uni from '@dcloudio/vite-plugin-uni'
import path from 'path'

export default defineConfig({
  plugins: [uni()],
  css: {
    preprocessorOptions: {
      scss: {
        // 将 styles 目录添加到 Sass 的查找路径中
        includePaths: [path.resolve(__dirname, 'styles')]
      }
    }
  }
})