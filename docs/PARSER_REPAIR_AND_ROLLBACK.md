# 现役解析修复、演示兜底与回退（2026-10-15）

## 修改范围

当前数据按学长确认：单物理通道、40 kHz、8秒、320,000个小端 uint16 采样。`ch=2` 只保存在元数据 `legacy_header_ch` 中，不再用于拆块。解析器只接受现役 `rate=40000 blk=2000 nblk=160` 配置，显式标为 block-interleaved 的旧文件会拒绝，避免静默误解析。

处理顺序：完整连续流 → 去掉首个无效 0xFFFF（如果存在）→ `scipy.signal.resample_poly(up=1,down=2,window=('kaiser',5.0),padtype='line')` → 约160,000点、20 kHz → 前100,000点、5秒 → 原有 EXE。

FIR 是多相抗混叠低通，`padtype=line` 减少边界补零造成的直流边缘突变。保留原有 ADC 码幅值单位；本次不重写 MATLAB 特征提取、归一化和模型。此实现修正时间轴及抗混叠，不代表已复现旧训练阶段未知的滤波细节，也不代表模型临床精度已经验证。

## 安装与只解析检查

使用独立 Python 3.11 或 3.12 环境。这里锁定的依赖不适用于 Python 3.14；服务器如果仍用3.14，请并存安装3.12并建立虚拟环境，不覆盖旧环境。

```powershell
cd D:\project\PPG\算法\服务器部署\prodict_algorithm_infuwuqi\project
py -3.12 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
.\.venv\Scripts\python.exe frame_pipeline.py "你的原始帧.bin" --parse-only
```

只解析模式不启动 EXE、不上传后端。检查日志中的 `layout=continuous`、`physical_channels=1`、`raw_rate_hz=40000`、`effective_fs_hz=20000`、`available_points≈160000`。

启动接收器前把 PYTHON_BIN 指向该虚拟环境：

```powershell
$env:PYTHON_BIN = 'D:\project\PPG\算法\服务器部署\prodict_algorithm_infuwuqi\project\.venv\Scripts\python.exe'
$env:ENABLE_FAILURE_FALLBACK = '1'
cd D:\project\PPG\算法\服务器部署\raw-receiver
npm start
```

演示兜底按用户要求保留，默认仍是1；模型失败继续生成确定性的演示血压/心率并写库，SpO2占位行为未改。帧格式不合法会直接失败，不会用兜底掩盖损坏的帧。失败兜底不能当袖带真值；标注页面只保存人工输入的参考值。

## 解析回归

公开接口目录提供合成测试，不需要私有模型：

```powershell
cd D:\project\PPG\App\appprogram\prodict_algorithm_infuwuqi\project
D:\project\PPG\工具\运行环境\smi-parser-venv\Scripts\python.exe -m unittest -v test_frame_pipeline
```

覆盖连续斜坡的时间轴、ch=1/2兼容、15 kHz 抗混叠抑制、损坏帧拒绝，以及模型失败时演示兜底仍可走通。测试通过不代表已经跑通私有 MATLAB Runtime 或实机采集。

## 改前存档

本机存档：`D:\project\PPG\private-backups\before-app-parser-repair-20261015\`。

- `source-and-assets.tar`：修改前 App、后端、作品集、算法源码/模型/EXE及根README，包括本地未跟踪资产和环境文件；排除node_modules、.git、unpackage。
- `app.bundle`、`backend.bundle`、`algorithms.bundle`、`portfolio.bundle`：四个仓库完整Git历史。
- App、后端及私有算法的回退标签：`archive/before-app-parser-repair-20261015`。
- 原工作目录 node_modules 继续保留。新下载的Node和隔离验证依赖只在 `工具/运行环境`。
- 此次未修改数据目录、MySQL结构或现存数据库数据；存档不是服务器数据库备份。

本地备份含密钥和保密资产，不上传GitHub。

回退时先解压到一个新的目录核对，避免覆盖后续工作：

```powershell
New-Item -ItemType Directory D:\project\PPG\private-backups\restore-review-20261015
tar -xf D:\project\PPG\private-backups\before-app-parser-repair-20261015\source-and-assets.tar -C D:\project\PPG\private-backups\restore-review-20261015
```

对比新目录中的改前文件与工作目录，再恢复选定文件。整仓库历史也可从bundle克隆到新目录。不要对当前仓库直接执行 `git reset --hard`。
