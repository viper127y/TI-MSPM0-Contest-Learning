/**
 *
 *
 * @file       project_config.h
 * @author     [Ns单片机实验室--yzq]
 * @version    v1.0.01
 * @date       2026-7-24
 * @brief      全局配置头文件：各设备数量 / 设备号别名 / OLED 屏幕参数
 *
 * 组织规则：
 *   - 设备总数（XXX_DEV_NUM）→ 决定驱动里"设备池"数组长度
 *   - 设备号别名（如 OLED → emOLedDevNum0）→ 应用层只认别名，
 *     换设备号/换屏幕只改这里，应用层代码一行不动
 *   - OLED 屏幕参数 → 显存大小由它推导，换屏只改这 2 个宏
 *
 *  <h2><center>&copy;文件版权归属【Ns单片机实验室】所有.</center></h2>
 */


#ifndef _PROJECT_CONFIG_H_
#define _PROJECT_CONFIG_H_

#define TRUE                                            1
#define FALSE                                           0

#define TI_IC_DL                                   "ti_msp_dl_config.h"

///LED
#define LED_DEV_NUM                                     9           /* LED 设备池长度：板载灯 + 8 路 */
#define LED_BORAD                                    emLedDevNum0   /* 别名：LED_BORAD 即 0 号设备（板载 PC13） */
#define LED1                                         emLedDevNum1
#define LED2                                         emLedDevNum2
#define LED3                                         emLedDevNum3
#define LED4                                         emLedDevNum4
#define LED5                                         emLedDevNum5
#define LED6                                         emLedDevNum6
#define LED7                                         emLedDevNum7
#define LED8                                         emLedDevNum8

///DHT11温湿度传感器
#define DHT11_DEV_NUM                                		1       /* DHT11 设备池长度：只挂 1 个 */
#define DHT11                                        emDht11DeviceNum0

///按键
#define BTN_DEV_NUM                                     4           /* 按键设备池长度 */

///环形缓冲区
#define	RING_BUFFER_DEV_NUM															1       /* 环形缓冲区设备池长度 */
#define	UART_RX_BUFFER															emRingBufferDevNum0   /* 别名：串口接收缓冲区即 0 号设备 */

///命令解释器
#define INTERPRETER_DEV_NUM															1						/* 命令解释器设备池数量 */
#define CMD_INTERPRETER_DELIVERY_NUM_MAX                                            5    /* 一条命令最多分词数（对象+命令+参数...） */
#define CMD_INTERPRETER_DELIVERY_STRING                                             " "  /* 分词分隔符：空格 */


//OLED
#define OLED_DEV_NUM																		1       /* OLED 设备池长度：主屏 1 块 */
#define OLED																		        emOLedDevNum0   /* 别名：OLED 即 0 号设备（主屏） */

/* 屏幕物理像素：换屏只改这 2 个宏（如换 0.91 寸屏改为 128 和 32） */
#define OLED_POINT_WIDTH                                                                    128   /* 屏幕横向像素数（列） */
#define OLED_POINT_HEIGHT                                                                   64    /* 屏幕纵向像素数（行） */

/* 显存大小推导：
 *   WIDTH  = 像素宽 128（每列 1 字节/页）
 *   HEIGHT = 像素高 64 ÷ 8 = 8 页（单色屏 1 像素=1 位，竖 8 行打包 1 字节 = 1 页）
 *   显存总大小 = 128 × 8 = 1024 字节 = 1KB
 * 改 POINT 宏后显存自动跟着变，业务代码不用动 */
#define OLED_BUFFER_WIDTH                                                                   OLED_POINT_WIDTH   //一帧数据的行宽度
#define OLED_BUFFER_HEIGHT                                                                  OLED_POINT_HEIGHT/8 //一帧数据的页数（高度÷8）




#endif