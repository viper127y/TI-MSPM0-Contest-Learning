# 05_OLED 学习总结

基于 **TI MSPM0G3507**（LP-MSPM0G3507 LaunchPad）+ **MSPM0 SDK 2.11**，通过 **硬件 I2C** 驱动 0.96 寸 **SSD1306 OLED（128×64）** 显示字符、数字、汉字、图形与图片。

## 项目核心学习点

### 1. SysConfig 图形化配置（empty.syscfg）
- **时钟树**：外部晶振 HFXT 40MHz → SYSPLL（QDIV=5）→ UDIV=2，最终得到高频系统时钟；`SYSCTL.clockTreeEn` 开启时钟树编辑。
- **I2C 外设**：实例名 `OLED`，映射到硬件 I2C1（SCL=PB2，SDA=PB3），使能 Controller 模式，配置上拉、毛刺滤波、TX/RX FIFO 触发水位。
- **GPIO**：LED0（PA14）、LED1（PA15）用于主循环中翻转指示。
- 配置完成后由 `SYSCFG_DL_init()`（ti_msp_dl_config.c）自动生成外设初始化代码 —— **“配置与逻辑分离”是 TI 新一代 SDK 的核心思想**。

### 2. 硬件 I2C 控制器操作（OLED_WR_Byte）
驱动 SSD1306 的关键通信流程，使用 DriverLib API 操作 FIFO 与状态机：
1. `DL_I2C_getControllerStatus()` 轮询等待总线 **IDLE**；
2. `DL_I2C_fillControllerTXFIFO()` 填入 2 字节：**控制字节 + 数据字节**；
3. `DL_I2C_startControllerTransfer()` 以从机地址 `0x3C` 启动传输；
4. 等待 **BUSY**（确保状态机真正启动）→ 再等回 **IDLE**（传输完成）。
- **控制字节规则**：`0x00` 表示后面是命令，`0x40` 表示后面是数据。

### 3. SSD1306 初始化与时序要点
- 4 针 OLED 无 RST 引脚，上电后需 `Vdelay_ms(100)` 等内部 RC 复位完成。
- 初始化序列涵盖：关显示 `0xAE` → 对比度 `0x81/0xCF` → 扫描方向 `0xA1/0xC8` → 占空比 `0xA8/0x3F` → 时钟/预充电/COM 引脚/VCOMH → **页寻址模式 `0x20/0x02`** → 电荷泵 `0x8D/0x14`（无 VCC 引脚，屏幕靠内部电荷泵升压）→ 开显示 `0xAF`。

### 4. 显存（GRAM）架构 —— 最重要的软件设计思想
- 定义 `OLED_GRAM[144][8]`：**128 列 × 8 页**，每页竖直 8 个像素（1 字节）。
- 所有绘图函数（`DrawPoint / DrawLine / DrawCircle / ShowChar / ShowString / ShowNum / ShowChinese`）只修改显存，**最后必须调用 `OLED_Refresh()` 统一刷屏**。
- `Refresh` 按"页寻址"逐页写：设页地址 `0xB0+i` → 列低 4 位 `0x00` → 列高 4 位 `0x10` → 连续写 128 字节。
- 画点本质是位运算：`OLED_GRAM[x][y/8] |= 1<<(y%8)`。

### 5. 字模与取模原理
- ASCII 字符：`asc2_1206 / asc2_1608 / asc2_2412` 字模库，索引 = `chr - ' '`，逐位判断最高位（`temp & 0x80` 后左移）来点亮/熄灭像素。
- 汉字：`Hzk1~Hzk4` 字模库（16/24/32/64 号），按**数组编号**索引，需要用取模软件（如 PCtoLCD2002）预先取模。
- 图片：`OLED_ShowPicture()` 按页寻址直接整块写入位图数组。

### 6. 延时实现（delay.c）
- `Vdelay_ms()`：基于 `delay_cycles()`，公式 `cycles = CPUCLK_FREQ/1000 × ms`，即 CPU 主频换算软件循环延时（忙等，非定时器）。

### 7. main() 应用层套路
```
SYSCFG_DL_init();      // SysConfig 生成的外设初始化
OLED_Init();           // 屏幕初始化
OLED_ColorTurn(0);     // 正常/反色
OLED_DisplayTurn(0);   // 是否 180° 翻转
OLED_Clear();
while(1) {
    sprintf(buf, "Integer %d", 50);   // 格式化字符串再显示
    OLED_ShowString(...);
    OLED_Refresh();                    // 改完显存统一刷新
    Vdelay_ms(500);
}
```

## 关键经验与易错点
| 问题 | 说明 |
| --- | --- |
| 屏幕 I2C 地址 | SSD1306 默认 `0x3C`（SA0 接低），`0x3D` 为备选 |
| 显示不亮 | 检查电荷泵命令 `0x8D+0x14` 是否发出；4 针模块必须开电荷泵 |
| 内容不更新 | 修改显存后忘记调用 `OLED_Refresh()` |
| 上电花屏/不亮 | 上电后需延时 ≥100ms 再初始化 |
| 命令/数据混乱 | I2C 模式下控制字节 `0x00`=命令、`0x40`=数据，不能省略 |
| SDA/SCL 引脚 | 本工程固定 PB3=SDA、PB2=SCL，需外部 4.7k 上拉（SysConfig 已配内部上拉） |

## 可扩展方向
- 用定时器/SysTick 替换忙等延时；
- I2C 改用 DMA 或中断提高刷屏效率；
- 加入滚动显示（SSD1306 内置滚动命令 0x26/0x27）、帧率优化（局部刷新）。
