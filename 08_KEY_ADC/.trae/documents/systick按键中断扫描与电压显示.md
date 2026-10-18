# 计划: SysTick 中断按键扫描 + 电压显示解耦

## Context (背景)

当前 main.c 的电压/状态显示都挂在 `if (iFlag != iLastFlag)` 判断里, 且主循环靠 `delay_ms(9)` 凑按键 10ms 调用周期。OLED_Refresh 全屏传输约 250ms, 一旦在循环里频繁刷屏就会把按键扫描拖慢 (项目硬约束: 按键驱动计数阈值单位是调用次数, 必须固定周期调用)。

目标: 电压值持续显示在 OLED 上 (1 位小数, 变了才刷), 但按键响应完全不受刷屏阻塞影响。

方案 (用户已确认): 把 `vBtnExecute` 挪进 SysTick 中断, 固定 10ms 调用; 主循环只负责 ADC 采样、换算电压、内容变化才刷屏。按键时序与主循环彻底解耦。

已确认的事实:
- `vBtnExecute` ([key.c](../../key.c) L112-L225) 只做 GPIO 读 + 计数器, 无浮点无阻塞, 适合进中断
- 事件保持到 `vBtnClearEvent` 才清除 ([key.c](../../key.c) L84-L95), 主循环慢不会丢事件
- main.c L40-L53 已有注释好的 SysTick 框架可直接启用
- OLED_ShowString 只写 RAM 显存, OLED_Refresh 才走 I2C 全屏传输

## 修改文件: 仅 main.c

### 1. 启用 SysTick 框架并加入按键扫描

取消 L40-L53 注释, 改为:

```c
static volatile uint32_t s_ulSysTick = 0;   // 系统毫秒计数
static uint8_t s_ucScanDiv = 0;             // 只在中断里访问, 不需要 volatile

void SysTick_Handler(void)
{
    s_ulSysTick++;
    if (++s_ucScanDiv >= 10) {          // 1ms x 10 = 10ms 固定周期
        s_ucScanDiv = 0;
        vBtnExecute(emBtnDevNum0);      // 中断里扫描, 主循环刷屏再久也不影响
    }
}
```

`vSysTickInit()` 在 `SYSCFG_DL_init()` 之后调用: `SysTick_Config(CPUCLK_FREQ / 1000)`。

### 2. 主循环重构

- 删除 `delay_ms(9)` 和循环内的 `vBtnExecute(emBtnDevNum0)`
- 按键事件处理 (switch emBtnGetEvent) 保留在主循环, 逻辑不变
- ADC 采样用时间戳调度, 每 100ms 一次 (无符号减法判溢出, 复用 s_ulSysTick):

```c
if ((uint32_t)(s_ulSysTick - ulLastAdcTick) >= 100) {
    ulLastAdcTick = s_ulSysTick;
    DL_ADC12_startConversion(ADC_Input_1_INST);
    while (!DL_ADC12_getStatus(ADC_Input_1_INST, DL_ADC12_STATUS_MEM_RES0_LOADED));
    ulResult = DL_ADC12_getMemResult(ADC_Input_1_INST, ADC_Input_1_ADCMEM_0);
    // 纯整数换算, 避开浮点 printf 依赖: 单位 0.01V
    usCentiVolt = (uint16_t)((uint32_t)ulResult * 2500UL / 4095UL / 10UL);
}
```

### 3. 显示逻辑: 任一行内容变化才 OLED_Refresh

- 第 0 行 (y=0): `status: X`, iFlag 变化时更新该行字符串
- 电压行 (y=2): `U: 0.6V`, 由 1 位小数量化值变化时更新:

```c
uint8_t ucTenth = usCentiVolt / 100;          // 0.1V 精度量化
if (ucTenth != ucLastTenth) {
    ucLastTenth = ucTenth;
    snprintf(ucVoltLine, sizeof(ucVoltLine), "U: %d.%dV", ucTenth / 10, ucTenth % 10);
    OLED_ShowString(0, 2, ucVoltLine, 16);
    bNeedRefresh = 1;
}
```

- 循环末尾: `if (bNeedRefresh) { bNeedRefresh = 0; OLED_Refresh(); }`
- 首帧强制刷新: `ucLastTenth` 初始化为 0xFF, `iLastFlag` 保持 -1

## 已知小风险 (记录, 不阻塞)

- `emBtnGetEvent` 与中断里的 `vBtnExecute` 共享事件字段, 理论上存在"读走后新事件被 Clear 覆盖"的窗口, 但 10ms 周期 + 人工按键频率下概率可忽略
- 中断里调用驱动函数要求驱动不可重入冲突, 当前只有 SysTick 一个中断源调用它, 安全

## 验证步骤

1. 编译烧录 (CCS 里 Debug 构建)
2. 调节 ADC 输入电压 (电位器/电源), OLED 电压行按 0.1V 步进更新
3. 在刷屏瞬间连续按按键: 单击切 status 立即跟手, 无明显延迟
4. 长按/双击行为与之前一致
5. 用万用表对比 OLED 显示值与实测引脚电压, 误差应在 0.1V 量化范围内
