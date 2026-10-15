# 你现在的运行方式

前端继续放在电脑上，用HBuilderX打开、修改和运行；运行到手机后，App就在手机上执行。它通过网络访问后端，不需要与后端放在同一台电脑，更不需要把前端源码塞进后端文件夹。

## 日常只做这些

1. HBuilderX打开 `D:\project\PPG\App\appprogram`，继续按原来的方式运行到浏览器或手机。
2. 已有node_modules保留，直接使用。无需先照长手册运行npm ci、doctor或切换Node。
3. 原服务器地址没有改，仍为 `http://192.144.185.64:3002`。地址集中在src/config/api.js顶部：

```js
export const API_BASE_URL = 'http://192.144.185.64:3002';
```

以后换IP只改这一行。不用.env.local，不用填写环境变量，也不用再改另外几套配置文件。

## 电脑、手机和服务器分别放什么

| 位置 | 内容 |
| --- | --- |
| 你的电脑 / HBuilderX | App前端源码；日常开发运行 |
| 手机 | HBuilderX运行的App或安装的APK |
| 服务器 | health-backend、MySQL、raw-receiver、Python/MATLAB算法 |

H5前端以后如果要让别人打开网址访问，才需要单独发布网页；目前用HBuilderX开发不需要做这一步。

## 这次新增功能为什么可能还不能用

GitHub和本地文件已更新，但没有自动覆盖或重启服务器。前端新增页面调用的接口，也需要把后端的新业务文件复制到服务器。

仍用你习惯的复制方式即可：把当前health-backend的业务源码、package.json和package-lock.json复制到原后端项目，保留服务器原有.env、node_modules和data。因为这次新增头像上传依赖multer，复制业务源码后需要在服务器后端目录执行一次npm install，再按你原来的方式重启后端。没有新建数据库表这一步，也不用复制App前端到后端。

接收算法那边同样需要安装新增SciPy依赖后才使用新解析器，步骤在解析手册；仅HBuilderX运行App不会替你更新服务器算法。

## 长手册什么时候才看

换电脑、依赖损坏、要做命令行打包时，再看BUILD_AND_PAGE_GUIDE.md。此前锁定版本是为了以后能复现，不是要求你每天重新配置环境。
