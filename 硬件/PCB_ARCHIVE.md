# PCB 与原理图归档说明

本目录保存的是 Altium Designer 源文件，不是可以直接交给工厂的完整生产包。仓库中没有经过确认的 Gerber、钻孔文件、BOM、贴片坐标、阻抗要求或投板版本标签，因此不能仅凭 `.PcbDoc` 文件宣称“可直接生产”。

## 两套设计资料

### `AIA/stm32f4/`：较早版，文件时间 2026-01-15

```text
ADC/
  PCB1.pcbdoc
  Schematic2/1_P1.schdoc
Board1/
  PCB2.pcbdoc
  Schematic1/1_主体.schdoc
  Schematic1/2_存储.schdoc
  Schematic1/3_蓝牙.schdoc
```

### `Altium_stm32f41_2026-02-06/stm32f41/`：时间更晚、资料更完整

```text
ADC/
  PCB1.pcbdoc
  Schematic2/P1.schdoc
Board1/
  PCB2.pcbdoc
  Schematic1/主体.schdoc
  Schematic1/存储.schdoc
  Schematic1/蓝牙.schdoc
TP5400电源板-v1.1/
  TP5400电源板-v1.1.pcbdoc
  TP5400电源板-v1.1/P1.schdoc
```

从文件名只能判断存在 ADC 板、主板、存储/蓝牙原理图片段和 TP5400 电源板。不能仅根据名称确定实际焊接器件、板间接口或照片中每块 PCB 的准确版本。

## 建议的查看顺序

1. 使用 Altium Designer 打开时间较晚的 `.SchDoc`，先阅读 `主体`，再看 `存储`、`蓝牙` 和 ADC 原理图。
2. 用 Design → Update PCB 检查原理图与 PCB 的元件/网络差异，但不要直接保存覆盖原文件。
3. 在 PCB 中检查 Board Shape、层叠、规则、未布线网络和 DRC。
4. 核对接口丝印、电源方向、连接器 Pin 1、SMA/排线方向和测试点。
5. 将较早的 `AIA` 版本作为差异参考，不要混用两版原理图与 PCB。

## 投板前必须补齐

- 明确哪一套目录是最终版本，并在目录名或 README 中写版本号、日期和负责人；
- 导出并人工复核 Gerber/ODB++、NC Drill；
- 导出 BOM 和 Pick & Place，并确认封装、替代料和不可贴装器件；
- 标记电源输入范围、3.3 V/5 V 域、模拟地/数字地处理；
- 核对 ADS8688 模拟输入保护、参考电压和量程；
- 核对 STM32 与 ESP32 UART 都是 3.3 V TTL；
- 对激光器供电、MOS 极性和上电默认关闭状态做安全检查；
- 首板先限流供电，分电源板、数字板、ADC/模拟前端逐块验证。

## 建议以后增加的公开材料

为方便面试展示而又不暴露生产细节，可以补充：

- 每块板正反面照片和一句功能说明；
- 一张经过脱敏的系统接线图；
- 原理图关键页面截图（电源、STM32、ADS8688、UART/ESP32）；
- 板卡版本号与固件 commit 的对应表；
- 首板问题、修改原因和最终验证结果。

这些材料比直接上传整个 Altium 工程更容易让面试官理解你的工程贡献。
