# TI-MSPM0-Contest-Learning

全国大学生电子设计竞赛（电赛）TI 基础学习仓库，基于 **MSPM0G3507 LaunchPad**（LP-MSPM0G3507）。

- IDE：CCS Theia（Code Composer Studio）
- SDK：MSPM0 SDK 2.11（DriverLib + SysConfig）
- 编译器：TI Clang

## 项目索引

每个项目对应一个独立 CCS 工程，按学习顺序编号命名（`NN_主题`）。

| 项目 | 内容 | 涉及外设 |
| --- | --- | --- |
| [01_LED](./01_LED) | GPIO 输出：LED 点灯、流水灯 | GPIO |
| [02_CLOCK_GPIO](./02_CLOCK_GPIO) | 时钟树配置（HFXT + PLL）与 GPIO | SYSCTL, GPIO |
| [03_Debug_Block](./03_Debug_Block) | 调试方法与延时函数 | DEBUGSS, GPIO |
| [05_OLED](./05_OLED) | 硬件 I2C 驱动 SSD1306 OLED（字符/数字/汉字/图形/图片），学习笔记见项目内 [notes.md](./05_OLED/notes.md) | I2C, GPIO |
| [06_uart](./06_uart) | UART 串口收发（含 OLED 显示） | UART, I2C, GPIO |

## 仓库结构约定

```
TI-MSPM0-Contest-Learning/
├── README.md            # 本文件：项目索引与约定
├── .gitignore           # 排除 Debug/ 等编译产物
├── 01_LED/              # 每个 CCS 项目一个文件夹
├── 02_CLOCK_GPIO/
├── ...
└── NN_主题/             # 新项目按序号新建，如 07_ADC
```

### 新增项目规范

1. 在 CCS Theia 中把工程创建/导入到本仓库根目录，命名格式 `NN_主题`（如 `07_ADC`、`08_PWM`）。
2. 项目内主要源文件使用中文注释说明学习要点。
3. 每个项目可附一个学习笔记（如 `notes.md`），记录核心知识点与踩坑记录。
4. 提交信息格式建议：`NN_主题: 简要描述`（如 `05_OLED: 添加显示图片功能`）。

## 环境说明

- 硬件：LP-MSPM0G3507 LaunchPad + 4 针 I2C OLED（0x3C）等外设模块
- 调试器：XDS-110（板载，注意 J101 跳线帽设置）
- 每个 `*.syscfg` 文件为 SysConfig 图形化配置，可在 CCS Theia 中双击打开编辑
