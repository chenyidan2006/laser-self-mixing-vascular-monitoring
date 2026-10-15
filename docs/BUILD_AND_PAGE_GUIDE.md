# App 构建与页面接通手册（2026-10-15）

**日常开发请先看 [HBuilderX简单运行说明](HBUILDERX_QUICK_START.md)。** 本页是换电脑、重装依赖和排错参考；已有依赖时直接用HBuilderX运行，不必每次执行这些命令。前端不需要放到后端或服务器上。

## 固定的构建组合

| 项目 | 版本 |
| --- | --- |
| Node.js | 22.14.0 |
| npm | 10.9.2 |
| DCloud 全套包 | 3.0.0-alpha-5000120260205001 |
| Vite | 5.2.8 |
| Vue / DCloud 内置编译器 | 3.4.21 |
| Sass | 1.77.8 |

这些版本已经过独立空目录 `npm ci` 和 H5 构建验证。`.nvmrc`、`.node-version`、`engines` 和 `packageManager` 都记录同一组合。`.npmrc` 开启严格版本检查；不要直接运行 `npm update` 或 `npx @dcloudio/uvm@latest`。DCloud 包必须整体同步升级，不能只升级一个插件。

本次还通过App资源和微信小程序资源编译；未进行APK签名、iOS打包或小程序平台发布。其他机器可从 [Node官方22.14.0 Windows x64压缩包](https://nodejs.org/dist/v22.14.0/node-v22.14.0-win-x64.zip) 安装同版；SHA-256为 `55b639295920b219bb2acbcfa00f90393a2789095b7323f79475c9f34795f217`。

## 本机逐条构建

打开普通 PowerShell。项目专用便携 Node 位于本地工具目录，不会覆盖系统 Node。

```powershell
cd D:\project\PPG\App\appprogram
$env:PATH = 'D:\project\PPG\工具\运行环境\node-v22.14.0-win-x64;' + $env:PATH
node --version
npm --version
npm run doctor
```

预期版本为 `v22.14.0`、`10.9.2`，doctor 中每项都显示 OK。

现有 `node_modules` 已保留，可直接构建：

```powershell
npm run build:h5
```

在一份新的检出目录安装时，使用锁文件完整安装：

```powershell
npm ci
npm run doctor
npm run build:h5
```

`npm ci` 会重建当前目录的依赖，所以不要在需要原样保留依赖的旧目录随意运行；本次洁净验证在独立目录完成。

```powershell
npm run dev
npm run build:app-plus
npm run build:mp-weixin
```

H5 成品在 `dist/build/h5/`。App 命令生成 uni-app App 资源，签名 APK/IPA 仍需要 HBuilderX 或相应打包服务；小程序正式发布还需要平台 AppID、合法 HTTPS 域名和真机验收。原来四个小程序脚本错误调用 Vue CLI，现已统一改用当前 Vite/uni CLI。

## Sass 已安装但提示找不到

本机复现到的直接原因是 `fs.realpathSync.native(.../node_modules/sass/package.json)` 返回 `EPERM`。Vite 5 的包解析会捕获这一错误并把它显示为“Preprocessor dependency sass not found”。`npm ls sass` 只检查依赖元数据，无法证明 realpath 权限正常。

```powershell
node -e "const fs=require('node:fs'); console.log(fs.realpathSync.native(require.resolve('sass')))"
npm run doctor
```

如果出现 EPERM，应检查隔离执行环境、目录权限、受控文件夹访问或安全软件。此次正常文件权限下原有依赖即可构建成功，无需把 Sass 安装到项目上层目录，也无需修改第三方 node_modules 源码或添加全局 NODE_PATH 补丁。仍有第三方组件的 Sass 弃用提示及 Browserslist 数据提示，它们不会导致当前构建失败；本次没有为消除提示而升级整套工具链。

## 统一 API 地址

全部页面和旧配置统一读取 `src/config/api.js`，开发阶段直接保留原IP：

```js
export const API_BASE_URL = 'http://192.144.185.64:3002';
```

换服务器只改这一行，再用HBuilderX重新运行。不需要.env.local或VITE_API_BASE_URL。将来启用HTTPS时再换成正式域名。

## 已接通页面

| 页面 | App 入口 | 后端/保存位置 |
| --- | --- | --- |
| 测量记录 | 我的 → 测量记录与标注 | 原有 daily-summary 接口 |
| 信号分析 | 我的 → 信号分析，或测量记录 → 信号分析 | GET /api/data/waveform；读取当前绑定设备最新原始帧 |
| 袖带真值 | 我的 → 袖带血压真值 | GET /api/training/sessions/pending；PUT /api/training/sessions/:id/reference |
| 密码重置 | 登录 → 忘记密码 | POST /api/user/reset-password |
| 头像 | 我的 → 编辑资料 → Change avatar | POST /api/user/avatar；服务器 data/avatars/ |
| 隐私 | 设置 → 数据与隐私 | 本地页面 |
| 应用日志 | 设置 → 应用日志 | 当前手机 appLogs，最多100条，无密码或令牌 |

袖带真值保存在对应 `raw_*.bin.reference.json`，由原始帧 SHA-256 关联，保存匿名受试者编号和人工参考值。无需建新表，不修改现有预测记录，不执行特征提取或训练。已标注帧从待标注列表中移除。页上时间为服务器接收时间；标注文件的 referenceTime 是录入时间，并非保证与采集同步的袖带测量时间。

波形显示完整连续 40 kHz 单通道干涉信号，并进行显示用滤波和峰值包络压缩。波形页不是血压曲线，也不使用降采样后的模型输入代替原始波形。

## 后端需同步更新

只更新前端无法启用新接口。后端按下面顺序手工操作：

```powershell
cd D:\project\PPG\App\appprogram\health-backend
npm ci
```

在后端已有 `.env` 中配置共享原始帧目录，必须与 raw-receiver 实际目录相同：

```text
RAW_DATA_DIR=D:\project\PPG\算法\服务器部署\raw-receiver\received
```

然后启动或重启原有后端进程：

```powershell
npm start
```

本次不需要执行数据库迁移。现有登录、设备绑定和测量表继续使用。头像目录需要可写；标注目录需要可写。不要把这些运行数据提交 Git。

后端默认CORS已加入localhost/127.0.0.1的8081和5173开发来源；若.env自行设置了CORS_ORIGINS，应手动加入实际浏览器来源。`App/health-backend.zip` 是原始后端快照，早于本次接口修复；本次部署应使用当前health-backend仓库，不能再用旧ZIP覆盖。

## 验收与边界

```powershell
cd D:\project\PPG\App\appprogram\health-backend
node --test test/repair-integration.test.cjs
```

测试使用模拟 SQL 层、临时合成帧和测试令牌，覆盖未登录/非本人设备拒绝、波形时间轴、标注落盘、验证码一次性消费、头像格式和个人资料缺省字段，完全不写真实数据库。

当前短信发送仍是开发模拟：验证码由后端控制台提供，未接短信服务商。重置流程已接通，但正式用户短信登录/找回仍需要真实服务。微信和 QQ 的外部平台能力不属于此次可独立完成的接通项。模型精度、训练脚本和数据库数据治理仍按约定保留原状。
