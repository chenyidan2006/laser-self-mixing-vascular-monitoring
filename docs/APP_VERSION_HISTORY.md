# App 版本关系核查

## 关于提交 `a0f6eb1`

提交标题为“第一次提交，光电比赛视频版本，有真实和模拟两种，真实没有实验，模拟目前正好”。它不是当前 App 原封不动沿用至今的版本。

证据：`a0f6eb1` 到当前 `HEAD` 的关键 App 文件对象哈希均已变化，包括：

- `src/App.vue`
- `src/config/api.js`
- `src/pages/index/index.vue`
- `src/pages/health/data.vue`
- `src/pages/health/input.vue`
- `src/pages/health/trend.vue`
- `src/pages/login/login.vue`

后续版本关系：

| 提交 | 作用 |
| --- | --- |
| `a0f6eb1` | 光电比赛视频版首次完整提交；建立 `src/` App，含真实/模拟入口 |
| `3d04318` | 修登录逻辑和模拟数据，新增目标设置、重置密码等页面 |
| `ec79547` | 修复模拟数据时间问题 |
| `2d9b6ed` | 大规模整理现役 App；修改 30 个 `src` 文件，新增测量图、波形、参考血压等模块，并清理旧构建产物/后端重复内容 |
| `06ac846` | 同步服务器算法并记录全链路审计 |
| `8b0d94a` | 记录最终原始波形接收服务和服务器调试工具 |

因此准确说法是：**当前 App 以比赛视频版为祖先，后续持续修改，2026-10-13 又经过一次现役版本整理；不是从未覆盖。** Git 的提交页仍显示 `a0f6eb1` 是因为 GitHub 永久保留历史记录，这正是版本控制的作用，并不表示它仍是当前文件。

## 为什么看起来像“旧版还在”

1. GitHub 的 Commits 页面列出历史提交，不会在新版本覆盖后删除旧提交。
2. App 的源码根目录一直是 `src/`，所以新旧页面路径相同，看目录名无法判断内容是否更新。
3. 多次提交是在旧版上增量修改，不是删除仓库后重新上传；因此提交历史会形成连续链。
4. `2d9b6ed` 清理了大量构建产物和重复目录，但保留了比赛版的可追溯历史。

查看某个文件当前版与比赛版的差异：

```powershell
git diff a0f6eb1 HEAD -- src/pages/index/index.vue
```

查看比赛版原文件而不切换分支：

```powershell
git show a0f6eb1:src/pages/index/index.vue
```
