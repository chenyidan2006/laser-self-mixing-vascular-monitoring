%% ========================================================================
%  ADS8688 串口采集数据处理: 帧头解析 + 电压换算 + 带通滤波(滤掉工频) + 绘图
%  ------------------------------------------------------------------------
%  【一、文件帧头(3005/3010.txt 实测)】
%    文件是"整条串口流"的十六进制转储: 每个字节写成 2 位十六进制 + 空格,
%    所以 ASCII 帧头/帧尾也被写成了十六进制。开头是:
%      23 41 44 53 ... = "#ADS8688 v1 rate=40000 secs=8 col=10000 blk=2000
%                          nblk=160 ch=2 range=0x02 code=u16le\r\n"
%    结尾是:
%      "#END blk=160 skip=0 over=0 dma=0 ecode=0 to=0 spi=0 ack=0\r\n"
%    中间 640000 字节是数据区: 320000 个采样值。
%       rate=40000  -> 采样率 40 kHz (Fs, 设计滤波器必需)
%       range=0x02  -> ADS8688 量程 ±2.56 V (满量程跨度 5.12 V)
%       code=u16le  -> 16 位、低字节在前
%       ch=2, blk=2000 -> 2 个通道, 按 2000 点一块【交替成块】存放:
%                         第1块=通道A、第2块=通道B、第3块=通道A ...
%                         (实测: 相邻两块直流电平差 40~80 uV, 即两通道失调不同;
%                          同奇偶的块之间没有这个差, 见下面 blockSel 选项)
%    数据区第 1 个采样值是 0xFFFF(65535, 满量程轨)的无效值, 必须丢掉。
%
%  【二、换算公式】(与 untitled03.m 一致)
%       V = (u / 65535) * Vspan - Vspan/2    双极性 ±2.56 V => Vspan = 5.12
%    等价 V = (u - 32768) * 5.12/65536 = (u - 32768) * 78.125 uV
%
%  【三、滤波: 默认用带通把工频范围整段滤掉】
%    零相位(前后向各滤一次)巴特沃斯带通, 由 RBJ 双二阶级联实现, 不依赖工具箱:
%        下限 bpLow  = 100 Hz  -> 滤掉 直流 + 50 Hz 工频 + 工频低次谐波 + 10 Hz 块交替
%        上限 bpHigh = 0.45*Fs -> 限制带宽(抗混叠/降噪)
%    比"只陷 50 Hz"更干净: 陷波只挖掉 50 Hz 一个点, 直流、50 Hz 的漂移、
%    以及 2 通道成块存放带来的 ~10 Hz 方波(约 40~80 uV)都还在; 带通一起解决。
%    若有用信号本身就在 50 Hz 附近, 请把 bpLow 调到信号频率以下(并关掉工频)。
%    端点延拓长度取 20*tau: 实测 5*tau 时记录开头会残留 2.5 mV 假尖峰。
%  ------------------------------------------------------------------------
%  本文件按 UTF-8 保存; R2019b 及更早版本请另存为 GBK, 或先执行
%  feature('DefaultCharacterSet','UTF-8')
% =========================================================================
clear; clc; close all;
warning off;

%% ---------------- 1. 参数设置 ----------------
filePath   = fullfile(pwd, 'input_frame.txt');          % 改成待分析的十六进制文本路径

autoFrame  = true;      % true = 自动识别帧头长度/字节序/无效首样本(推荐)
nSkipByte  = 2;         % autoFrame = false 时手工指定: 跳过前几个字节
byteOrder  = 'little';  % autoFrame = false 时手工指定: 'little'=低字节在前

FsOverride = NaN;       % 非 NaN 则覆盖帧头里的 rate(采样率, Hz)
scaleFactor = 5.12;     % 满量程跨度(V); 帧头 range=0x02 时自动识别
zeroOffset  = 2.56;     % 零点偏移(V)

% ---- 滤波方式 ----
filtMode = 'bandpass';  % 'bandpass' = 带通(默认, 滤掉工频整段)
                        % 'notch'    = 只陷工频(50 Hz 单点)
                        % 'both'     = 先带通再陷波(最狠)
                        % 'none'     = 不滤波
bpLow    = 100;         % 带通下限(Hz): 要滤掉直流+50Hz 工频, 取 >50 的合适值
bpHigh   = [];          % 带通上限(Hz): 留空 = 0.45*Fs
bpOrder  = 4;           % 巴特沃斯阶数(4 阶 = 每侧 2 个二阶节, 零相位后等效 8 阶)
notchHz  = 50;          % filtMode 含 notch 时使用
notchQ   = 20;          % 陷波品质因数(带宽 ≈ f0/Q)

blockSel = 0;           % 0 = 整条流(默认); 1 = 第1/3/5...块, 2 = 第2/4/6...块
                        % 帧头 ch=2 且 blk 已知时有效, 去掉 ~10 Hz 块交替伪迹

expectMin = NaN;        % 已知信号下限(V), 如正弦测量填 0.1; 不检查填 NaN
expectMax = NaN;        % 已知信号上限(V), 如 1.1(注意: 自检用【滤波前】的原始值)
zoomPts   = 3200;       % 局部放大点数(3005/3010 的工频周期 = 800 点, 3200 = 4 个周期)
saveTxt   = false;      % true = 输出滤波前后两个结果文件到源文件同目录

% 使用方法和 8 个主图的解释见同目录 README.md。
% 注意：默认 100 Hz 高通用于诊断高频 SMI 成分和去除块交替/工频，
% 不是服务器血压模型的预处理流程；研究低频脉搏波时必须降低 bpLow 或关闭滤波。

%% ---------------- 2. 读取十六进制文本 ----------------
fid = fopen(filePath, 'r');
if fid < 0
    error('文件打开失败, 请检查路径: %s', filePath);
end
txt = fread(fid, Inf, '*char').';
fclose(fid);
txt = strrep(txt, '<![CDATA[', '');
txt = strrep(txt, '<! [CDATA [', '');
txt = strrep(txt, ']]', '');

bytes = sscanf(txt, '%2x');                  % 每 2 位十六进制 -> 1 个字节
if isempty(bytes)
    bytes = hex2dec(regexp(strtrim(txt), '[0-9A-Fa-f]{2}', 'match'));
end
bytes = double(bytes(:));
fprintf('文件字节数: %d\n', numel(bytes));

%% ---------------- 3. 解析 ASCII 帧头 / 帧尾 ----------------
hdrBytes = 0; tailStart = 0; headText = '';
Fs = 1; blkLen = 0; chNum = 1;
if numel(bytes) > 16 && bytes(1) == 35                   % '#'
    k = find(bytes(1:min(512, numel(bytes))) == 13, 1);  % 找 CR
    if ~isempty(k)
        hdrBytes = k + 1;                                % 连 CRLF 一起跳过
        headText = char(bytes(1:hdrBytes)');
    end
end
if hdrBytes > 0
    p = find(bytes == 35);                               % 数据区之后最后的 '#'
    p = p(p > hdrBytes + 3);
    if ~isempty(p) && (numel(bytes) - p(end)) < 512
        tailStart = p(end);
    end
end
dataEnd = numel(bytes);
if tailStart > 0
    dataEnd = tailStart - 1;
end

if ~isempty(headText)
    fprintf('\n================ 帧头 ================\n');
    fprintf('%s', headText);
    rateStr = hdrGet(headText, 'rate', '');
    codeStr = hdrGet(headText, 'code', '');
    rangStr = hdrGet(headText, 'range', '');
    chStr   = hdrGet(headText, 'ch',   '');
    blkStr  = hdrGet(headText, 'blk',  '');
    nblkStr = hdrGet(headText, 'nblk', '');
    secsStr = hdrGet(headText, 'secs', '');
    fprintf('  -> 帧头 %d 字节', hdrBytes);
    if tailStart > 0
        fprintf(', 帧尾 %d 字节', numel(bytes) - tailStart + 1);
    end
    fprintf('\n');

    if ~isempty(rateStr)                                 % 采样率
        Fs = str2double(rateStr);
        if isnan(Fs), Fs = 1; end
        fprintf('  -> 采样率 rate = %g Hz\n', Fs);
    end
    if ~isempty(blkStr)
        blkLen = str2double(blkStr);
    end
    if ~isempty(chStr)
        chNum = str2double(chStr);
        if isnan(chNum), chNum = 1; end
        fprintf('  -> 通道数 ch = %s', chStr);
        if blkLen > 0
            fprintf(' (按 %d 点一块交替成块存放)', blkLen);
        end
        fprintf('\n');
    end
    if ~isempty(codeStr)                                 % 字节序
        lc = lower(codeStr);
        if ~isempty(strfind(lc, 'le'))                   %#ok<STREMP>
            byteOrder = 'little';
        elseif ~isempty(strfind(lc, 'be'))               %#ok<STREMP>
            byteOrder = 'big';
        end
        fprintf('  -> 编码 %s => 字节顺序 %s\n', codeStr, byteOrder);
    end
    if ~isempty(rangStr)                                 % 量程 -> 满量程跨度
        rangeCode = hex2dec(regexprep(rangStr, '^0[xX]', ''));
        bip = true;
        switch rangeCode
            case 0, spanHdr = 20.48;                     % ±10.24 V
            case 1, spanHdr = 10.24;                     % ±5.12  V
            case 2, spanHdr =  5.12;                     % ±2.56  V
            case 3, spanHdr =  2.56;                     % ±1.28  V
            case 4, spanHdr = 10.24; bip = false;        % 0~10.24 V
            case 5, spanHdr =  5.12; bip = false;        % 0~5.12  V
            otherwise, spanHdr = NaN;
        end
        if ~isnan(spanHdr)
            scaleFactor = spanHdr;
            if bip
                zeroOffset = spanHdr / 2;
            else
                zeroOffset = 0;
            end
            fprintf('  -> 量程 range=0x%X => 满量程 %.2f V, 偏移 %.2f V%s\n', ...
                    rangeCode, scaleFactor, zeroOffset, ...
                    ternary(bip, '', ' (单极性)'));
        end
    end
    if ~isempty(blkStr) && ~isempty(nblkStr) && ~isempty(secsStr)
        fprintf('  -> 分块: %s 点/块 x %s 块 = %g 点, 时长 %s s\n', ...
                blkStr, nblkStr, str2double(blkStr)*str2double(nblkStr), secsStr);
    end
    fprintf('======================================\n');
else
    fprintf('\n未发现 "#..." 帧头(旧格式或无头文件)。\n');
end
if ~isnan(FsOverride)
    Fs = FsOverride;
    fprintf('采样率被 FsOverride 覆盖为 %g Hz\n', Fs);
end

%% ---------------- 4. 帧头长度/字节序自动确认 ----------------
dataAll = bytes(hdrBytes+1 : dataEnd);
if autoFrame
    frames = struct('off', {}, 'order', {}, 'cost', {}, 'n', {});
    orders = {'little', 'big'};
    if ~isempty(headText)
        orders = {byteOrder};          % 帧头已声明字节序时只扫这一种
    end
    for off = 0:16
        rest = dataAll(off+1:end);
        n2   = floor(numel(rest)/2);
        if n2 < 2000
            continue;
        end
        pr = reshape(rest(1:2*n2), 2, []).';
        for k = 1:numel(orders)
            if strcmpi(orders{k}, 'little')
                u = pr(:,2)*256 + pr(:,1);
            else
                u = pr(:,1)*256 + pr(:,2);
            end
            c   = double(u);
            neg = (c >= 32768);
            c(neg) = c(neg) - 65536;
            d2 = abs(diff(diff(c)));
            frames(end+1) = struct('off', off, 'order', orders{k}, ...
                                   'cost', max(d2), 'n', n2);   %#ok<SAGROW>
        end
    end
    costs = [frames.cost];
    if isempty(costs)
        error('数据太短, 无法自动识别帧头。');
    end
    best = min(costs);
    ok   = find(costs <= max(best*1.5, best + 50));
    [~, kk] = min([frames(ok).off]);   % 并列时取跳字节最少的那个
    sel = ok(kk);

    fprintf('\n============ 帧头/对齐自动识别 ============\n');
    [~, ordIdx] = sort(costs);
    fprintf('  %-8s %-8s %-14s %s\n', '跳字节', '字节序', 'max|二阶差分|', '点数');
    for i = ordIdx(1:min(6, numel(ordIdx)))
        fprintf('  %-8d %-8s %-14d %d\n', frames(i).off, frames(i).order, ...
                frames(i).cost, frames(i).n);
    end
    nSkipByte = frames(sel).off;
    byteOrder = frames(sel).order;
    fprintf('  => 跳过 %d 个字节, 字节序 %s', nSkipByte, byteOrder);
    if nSkipByte > 0
        fprintf(' (跳过的字节: %s', ...
                strjoin(cellstr(dec2hex(dataAll(1:nSkipByte), 2)).', ' '));
        if nSkipByte == 2 && all(dataAll(1:2) == 255)
            fprintf(', 即数据区开头的无效首样本 0xFFFF');
        end
        fprintf(')');
    end
    fprintf('\n==========================================\n');
else
    fprintf('\n手工设置: 跳过 %d 字节, 字节序 %s\n', nSkipByte, byteOrder);
end

%% ---------------- 5. 拼接 + 电压换算 ----------------
data  = dataAll(nSkipByte+1:end);
if mod(numel(data), 2) ~= 0
    fprintf('提示: 剩余 %d 字节为奇数, 末尾 1 字节已丢弃。\n', numel(data));
end
nPairRaw = floor(numel(data)/2);
data = data(1:2*nPairRaw);

if strcmpi(byteOrder, 'little')
    loByte = data(1:2:end);  hiByte = data(2:2:end);     % 低字节在前
else
    hiByte = data(1:2:end);  loByte = data(2:2:end);     % 高字节在前
end
u = hiByte * 256 + loByte;                              % 无符号码 0~65535

% ---- 可选: 按块抽取某个通道(ch=2 时数据是"每 2000 点一块"交替存放) ----
FsEff = Fs;
if blockSel >= 1
    if blkLen < 1
        warning('帧头里没有 blk, 无法按块抽取通道, 已按整条流处理。');
    else
        nb = floor(numel(u)/blkLen);
        ub = reshape(u(1:nb*blkLen), blkLen, []);        % blkLen x nb
        u  = reshape(ub(:, blockSel:2:nb), [], 1);       % 取第1组或第2组交错块
        FsEff = Fs / 2;                                  % 每通道名义采样率
        fprintf('已按块抽取通道 %d: %d 块 x %d 点 = %d 点, 名义采样率 %g Hz\n', ...
                blockSel, numel(blockSel:2:nb), blkLen, numel(u), FsEff);
    end
end

voltRaw = (u / 65535) * scaleFactor - zeroOffset;        % 与 untitled03.m 相同
nPair   = numel(voltRaw);
t       = (0:nPair-1).' / FsEff;

DClevel = mean(voltRaw);
rawAC   = voltRaw - DClevel;                             % 去直流, 便于看交流

fprintf('\n采样点数            : %d\n', nPair);
if FsEff > 1
    fprintf('采样率              : %g Hz (时长 %.3f s)\n', FsEff, nPair/FsEff);
end
fprintf('无符号码 最小/最大值: %d / %d (0x%04X / 0x%04X)\n', ...
        min(u), max(u), min(u), max(u));
fprintf('电压     最小/最大值: %.6f V / %.6f V\n', min(voltRaw), max(voltRaw));
fprintf('电压     直流/交流  : %.6f V / %.6f V(rms)\n', DClevel, std(voltRaw));
fprintf('电压     峰峰值     : %.6f V\n', max(voltRaw) - min(voltRaw));

%% ---------------- 6. 滤波(默认带通, 滤掉工频整段) ----------------
volt    = voltRaw;
filtSec = [];                                            % [b(3) a(3) f0 Q type]
if isempty(bpHigh)
    bpHigh = 0.45 * FsEff;
end
if bpHigh <= bpLow
    warning('带通上限 %g Hz 不大于下限 %g Hz, 已自动调整。', bpHigh, bpLow);
    bpHigh = min(0.49 * FsEff, max(bpLow*10, bpLow + 100));
end
bpHigh = min(bpHigh, 0.49 * FsEff);

doBP    = any(strcmpi(filtMode, {'bandpass', 'both'}));
doNotch = any(strcmpi(filtMode, {'notch', 'both'}));

if (doBP || doNotch) && FsEff <= 1
    warning(['没有采样率(帧头无 rate 且 FsOverride 未设), 无法设计滤波器。' ...
             '请设置 FsOverride, 例如 FsOverride = 40000;']);
    doBP = false; doNotch = false;
end

if doBP
    Qs = butterQ(bpOrder);
    for k = 1:numel(Qs)                                  % 高通: 干掉直流+工频
        [b, a] = biquadHP(bpLow, FsEff, Qs(k));
        volt = zerophaseIIR(b, a, volt, FsEff, bpLow, Qs(k));
        filtSec = [filtSec; b, a, bpLow, Qs(k), 1];      %#ok<AGROW>
    end
    for k = 1:numel(Qs)                                  % 低通: 限制带宽
        [b, a] = biquadLP(bpHigh, FsEff, Qs(k));
        volt = zerophaseIIR(b, a, volt, FsEff, bpHigh, Qs(k));
        filtSec = [filtSec; b, a, bpHigh, Qs(k), 2];     %#ok<AGROW>
    end
    fprintf(['\n带通滤波: %.4g ~ %.4g Hz, %d 阶巴特沃斯(每侧 %d 个二阶节), ' ...
             '零相位(前后向各一次)\n'], bpLow, bpHigh, bpOrder, numel(Qs));
    fprintf('  等效 -3dB 通带 %.4g~%.4g Hz, 通带外衰减 ≥ %d dB/oct 每侧\n', ...
            bpLow, bpHigh, 6*bpOrder);
end
if doNotch
    for k = 1:numel(notchHz)
        f0 = notchHz(k);
        if f0 >= FsEff/2
            warning('陷波频率 %g Hz 超过 Nyquist(%g Hz), 已跳过。', f0, FsEff/2);
            continue;
        end
        w0    = 2*pi*f0/FsEff;
        alpha = sin(w0)/(2*notchQ);
        b = [1, -2*cos(w0), 1] / (1 + alpha);
        a = [1, -2*cos(w0)/(1+alpha), (1-alpha)/(1+alpha)];
        volt = zerophaseIIR(b, a, volt, FsEff, f0, notchQ);
        filtSec = [filtSec; b, a, f0, notchQ, 3];        %#ok<AGROW>
        fprintf('陷波: %.4g Hz, Q = %g (单次 -3dB 带宽 %.3f Hz), 零相位\n', ...
                f0, notchQ, f0/notchQ);
    end
end

if ~isempty(filtSec)
    fprintf('滤波后 交流 rms    : %.6f V  (滤波前 %.6f V, 降到 %.1f%%)\n', ...
            std(volt), std(voltRaw), 100*std(volt)/max(std(voltRaw), eps));
    for k = 1:numel(notchHz)
        a1 = ampAt(rawAC, notchHz(k), FsEff);
        a2 = ampAt(volt,   notchHz(k), FsEff);
        fprintf('%g Hz 幅值: %.6f V -> %.9f V (衰减 %.1f dB)\n', ...
                notchHz(k), a1, a2, 20*log10(max(a1, eps)/max(a2, eps)));
    end
end
voltAC = volt;                                           % 滤波后(已无直流)

%% ---------------- 7. 自检 ----------------
if ~isnan(expectMin) && ~isnan(expectMax)
    fprintf('\n================ 自检(用滤波前原始值) ================\n');
    fprintf('期望范围: %.3f ~ %.3f V\n', expectMin, expectMax);
    eMin = min(voltRaw) - expectMin;
    eMax = max(voltRaw) - expectMax;
    fprintf('实测偏差: 下限 %+0.4f V, 上限 %+0.4f V\n', eMin, eMax);
    spanReq = (expectMax - expectMin) * 65535 / double(max(u) - min(u));
    fprintf('按已知峰值反推满量程 = %.3f V (当前 %.3f V)\n', spanReq, scaleFactor);
    if max(abs([eMin eMax])) < 0.05
        fprintf('结论: 帧头/字节序/量程均已对齐。\n');
    else
        fprintf('警告: 偏差偏大, 请核对量程或 ADC 极性设置。\n');
    end
    fprintf('======================================================\n');
end

[fAx, Yraw] = ampSpectrum(rawAC);
[~,    Yflt] = ampSpectrum(voltAC);
fDisp = fAx * max(FsEff, 1);        % Fs 已知时为 Hz, 否则为 cycle/sample

%% ---------------- 8. 绘图 ----------------
% ---- 图1: 滤波后数据总览(交流分量, mV)
figure('Name', '滤波后数据总览', 'Color', 'w', 'Position', [50 30 1180 780]);
ymaxV = max(abs(voltAC)) * 1e3 * 1.1;
if ymaxV <= 0, ymaxV = 1; end

subplot(2,2,1);
plot(t, voltAC*1e3, 'Color', [0.12 0.47 0.71], 'LineWidth', 0.5);
grid on; box on;
xlabel(xLabel(FsEff)); ylabel('交流分量 (mV)');
title(sprintf('全段波形(滤波后, 直流 %.4f V 已扣除)', DClevel));

subplot(2,2,2);
nz = min(zoomPts, nPair);
plot(t(1:nz)*1e3, voltAC(1:nz)*1e3, 'r.-', 'LineWidth', 1, 'MarkerSize', 6);
grid on; box on;
xlabel('时间 (ms)'); ylabel('交流分量 (mV)');
title(sprintf('局部放大(前 %d 点 = %.1f ms)', nz, t(nz)*1e3));

subplot(2,2,3);
histogram(voltAC*1e3, 80, 'FaceColor', [0.64 0.08 0.18], 'EdgeColor', 'none');
grid on; box on; xlabel('交流分量 (mV)'); ylabel('点数');
title('幅值分布(滤波后)');

subplot(2,2,4);
fplotMax = FsEff/2;
if FsEff <= 1, fplotMax = 0.1; end
m = fDisp <= fplotMax;
plot(fDisp(m), 20*log10(max(Yraw(m), eps)), 'Color', [0.75 0.75 0.75], 'LineWidth', 0.8);
hold on;
plot(fDisp(m), 20*log10(max(Yflt(m), eps)), 'k', 'LineWidth', 0.9);
grid on; box on;
xlabel(ternary(FsEff<=1, '归一化频率 (cycle/sample)', '频率 (Hz)'));
ylabel('幅值 (dBV)');
title('幅值谱: 灰 = 滤波前, 黑 = 滤波后');
legend({'滤波前', '滤波后'}, 'Location', 'best');

% ---- 图2: 滤波细节(低频段)
if ~isempty(filtSec)
    figure('Name', '带通滤波细节', 'Color', 'w', 'Position', [80 60 1180 780]);
    nshow = min(nPair, max(round(4*FsEff/min(50, bpLow)), 2000));

    % (1) 时域对比(去直流)
    subplot(2,2,1);
    plot(t(1:nshow)*1e3, rawAC(1:nshow)*1e3, 'Color', [0.78 0.78 0.78], 'LineWidth', 1.2);
    hold on;
    plot(t(1:nshow)*1e3, voltAC(1:nshow)*1e3, 'r', 'LineWidth', 1.0);
    grid on; box on;
    xlabel('时间 (ms)'); ylabel('交流分量 (mV)');
    title(sprintf('去直流后时域对比(前 %.1f ms)', t(nshow)*1e3));
    legend({'滤波前', '滤波后'}, 'Location', 'best');

    % (2) 低频段频谱对比
    subplot(2,2,2);
    fhi = min(FsEff/2, max(500, 5*bpLow));
    m = fDisp <= fhi;
    h1 = plot(fDisp(m), 20*log10(max(Yraw(m), eps)), ...
              'Color', [0.78 0.78 0.78], 'LineWidth', 1.0);
    hold on;
    h2 = plot(fDisp(m), 20*log10(max(Yflt(m), eps)), 'r', 'LineWidth', 1.0);
    grid on; box on;
    xlabel('频率 (Hz)'); ylabel('幅值 (dBV)');
    title(sprintf('0~%g Hz 频谱对比', fhi));
    legend([h1 h2], {'滤波前', '滤波后'}, 'Location', 'best');
    yl2 = ylim;
    plot([bpLow bpLow], yl2, ':', 'Color', [0.4 0.4 0.9]);
    plot([50 50], yl2, ':', 'Color', [0.9 0.6 0.2]);

    % (3) 滤波器幅频响应
    subplot(2,2,3);
    fz = linspace(0, min(FsEff/2, max(2000, 8*bpLow)), 4000);
    H = ones(size(fz));
    for k = 1:size(filtSec, 1)
        bb = filtSec(k, 1:3);  aa = filtSec(k, 4:6);
        z  = exp(-1i*2*pi*fz/FsEff);
        H  = H .* ((bb(1) + bb(2)*z + bb(3)*z.^2) ./ ...
                   (aa(1) + aa(2)*z + aa(3)*z.^2));
    end
    plot(fz, 20*log10(max(abs(H), eps)), 'b', 'LineWidth', 1.1); hold on;
    plot(fz, 20*log10(max(abs(H).^2, eps)), 'r--', 'LineWidth', 1.1);
    grid on; box on;
    xlabel('频率 (Hz)'); ylabel('|H| (dB)');
    title('滤波器幅频响应');
    legend({'单次经过', '零相位(前后各一次)'}, 'Location', 'best');
    h50 = 20*log10(max(abs(interp1(fz, abs(H), min(50, max(fz)))), eps));
    fprintf('滤波器在 50 Hz 处衰减: 单次 %.1f dB, 零相位 %.1f dB\n', h50, 2*h50);

    % (4) 被滤除的低频成分(工频+直流漂移)
    subplot(2,2,4);
    plot(t(1:nshow)*1e3, (rawAC(1:nshow) - voltAC(1:nshow))*1e3, ...
         'Color', [0.2 0.5 0.2]);
    grid on; box on;
    xlabel('时间 (ms)'); ylabel('mV');
    title('滤波前 - 滤波后 (被滤掉的工频/低频成分)');
end

% ---- 图3: 帧头/错位对比
if autoFrame && nSkipByte > 0
    badOff   = nSkipByte + 1;
    badOrder = 'big';
    if strcmpi(byteOrder, 'big')
        badOrder = 'little';
    end
    bd = dataAll(badOff+1:end);
    bd = bd(1:2*floor(numel(bd)/2));
    if strcmpi(badOrder, 'little')
        bu = bd(1:2:end) + bd(2:2:end)*256;
    else
        bu = bd(1:2:end)*256 + bd(2:2:end);
    end
    bv = (bu / 65535) * scaleFactor - zeroOffset;

    figure('Name', '帧头处理对比', 'Color', 'w', 'Position', [110 110 1100 420]);
    nz2 = min([120, nPair, numel(bv)]);
    yl  = [min(voltRaw(1:nz2)) max(voltRaw(1:nz2))];
    pd  = 0.08 * max(yl(2) - yl(1), 1e-9);

    subplot(1,2,1);
    plot(t(1:nz2), voltRaw(1:nz2), '.-', 'Color', [0.12 0.47 0.71], 'LineWidth', 1);
    grid on; box on; ylim(yl + [-pd pd]);
    xlabel(xLabel(FsEff)); ylabel('电压 (V)');
    title(sprintf('正确: 跳过 %d 字节 + %s 在前', nSkipByte, byteOrder));

    subplot(1,2,2);
    plot(t(1:nz2), bv(1:nz2), '.-', 'Color', [0.79 0.10 0.16], 'LineWidth', 1);
    grid on; box on; ylim(yl + [-pd pd]);
    xlabel(xLabel(FsEff)); ylabel('电压 (V)');
    title(sprintf('错误: 跳过 %d 字节 + %s 在前 (低字节错位)', badOff, badOrder));
end

%% ---------------- 9. 保存结果 ----------------
if saveTxt
    [p, name, ~] = fileparts(filePath);
    o1 = fullfile(p, [name '_raw.txt']);
    o2 = fullfile(p, [name '_filtered.txt']);
    f1 = fopen(o1, 'w');  fprintf(f1, '%.8f\n', voltRaw);  fclose(f1);
    f2 = fopen(o2, 'w');  fprintf(f2, '%.8f\n', voltAC);   fclose(f2);
    fprintf('\n已保存: %s , %s (%d 行)\n', o1, o2, nPair);
end
warning on;

%% ---------------- 局部函数 ----------------
function v = hdrGet(txt, key, def)
    t = regexp(txt, [key '=([^\s]+)'], 'tokens', 'once');
    if isempty(t), v = def; else, v = t{1}; end
end

function Qs = butterQ(n)
    % n 阶(偶数)巴特沃斯分解成 n/2 个二阶节的 Q 值
    k  = (1:n/2)';
    Qs = 1 ./ (2*cos((2*k-1)*pi/(2*n)));
end

function [b, a] = biquadLP(fc, Fs, Q)
    % RBJ 二阶巴特沃斯低通
    w0 = 2*pi*fc/Fs;  cw = cos(w0);  al = sin(w0)/(2*Q);
    b0 = (1-cw)/2;  b1 = 1-cw;  b2 = (1-cw)/2;
    a0 = 1+al;      a1 = -2*cw; a2 = 1-al;
    b = [b0 b1 b2]/a0;  a = [a0 a1 a2]/a0;
end

function [b, a] = biquadHP(fc, Fs, Q)
    % RBJ 二阶巴特沃斯高通
    w0 = 2*pi*fc/Fs;  cw = cos(w0);  al = sin(w0)/(2*Q);
    b0 = (1+cw)/2;  b1 = -(1+cw);  b2 = (1+cw)/2;
    a0 = 1+al;      a1 = -2*cw;    a2 = 1-al;
    b = [b0 b1 b2]/a0;  a = [a0 a1 a2]/a0;
end

function y = zerophaseIIR(b, a, x, Fs, f0, Q)
    % 零相位(前后向各滤一次)IIR 滤波, 手动实现 filtfilt, 不依赖工具箱。
    % 端点用奇镜像延拓: 延拓长度必须远大于振铃时间常数 tau,
    % 否则记录开头会留下假尖峰(实测 5*tau 时残留 2.5 mV, 20*tau 时降到噪声级)。
    x = x(:);
    n = numel(x);
    tau  = 2*Q / (2*pi*f0/Fs);                        % 振铃时间常数(采样点)
    npad = min(floor(n/2), max(50, ceil(20*tau)));
    pre  = 2*x(1)   - x(npad+1:-1:2);
    post = 2*x(end) - x(end-1:-1:end-npad);
    xx = [pre(:); x; post(:)];
    y  = filter(b, a, xx);
    y  = flipud(filter(b, a, flipud(y)));
    y  = y(npad+1 : npad+n);
end

function A = ampAt(x, f0, Fs)
    % 直接 DFT 投影: 精确取出 f0 处的幅值(不受频谱栅格限制)
    x = x(:) - mean(x);
    n = numel(x);
    k = exp(-1i*2*pi*f0*(0:n-1)'/Fs);
    A = 2*abs(sum(x .* k)) / n;
end

function [f, Y] = ampSpectrum(x)
    x = x(:);
    n = numel(x);
    w = 0.5 - 0.5*cos(2*pi*(0:n-1)'/(n-1));           % Hann 窗
    Y = abs(fft((x - mean(x)).*w)) / sum(w) * 2;
    Y = Y(1:floor(n/2)+1);
    f = (0:numel(Y)-1).' / n;                         % 归一化频率(乘 Fs 即 Hz)
end

function s = xLabel(Fs)
    if Fs <= 1
        s = '采样点序号';
    else
        s = '时间 (s)';
    end
end

function s = ternary(cond, a, b)
    if cond, s = a; else, s = b; end
end
