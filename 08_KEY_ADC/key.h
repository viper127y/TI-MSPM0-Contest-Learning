#ifndef KEY_H
#define KEY_H

 

#include "project_config.h"
#include TI_IC_DL

/// @brief      设备号枚举定义
///
/// @note
typedef enum
{
    emBtnDevNum0                                       = 0,                            // 0  设备 0
    emBtnDevNum1,
    emBtnDevNum2,
    emBtnDevNum3,
    emBtnDevNum4,
    emBtnDevNum5,
    emBtnDevNum6,
    emBtnDevNum7,
    emBtnDevNum8,
    emBtnDevNum9,
    emBtnDevNum10,
} emBtnDevNumTdf;

/// @brief      按键有效电平枚举定义
///
/// @note       按键按下时的电平状态
typedef enum
{
    emBtnActiveLevel_Low                               = 0,                            // 0  低电平有效
    emBtnActiveLevel_High                              = 1,                            // 1  高电平有效
} emBtnActiveLevelTdf;

/// @brief      按键状态枚举定义
///
/// @note
typedef enum
{
    emBtnStatus_Released                               = 0,                            // 0        释放
    emBtnStatus_Pressed                                = 1,                            // 1        按下
    emBtnStatus_NotModify                              = 0xFFFF,                       // 0xFFFF   不做更改
} emBtnStatusTdf;

/// @brief      按键事件枚举定义
///
/// @note
typedef enum
{
    emBtnEvent_None                                    = 0,                            // 0        无事件
    emBtnEvent_Click                                   = 1,                            // 1        单击事件
    emBtnEvent_LongPress                               = 2,                            // 2        长按事件
    emBtnEvent_DoubleClick                             = 3,                            // 3        双击事件
} emBtnEventTypeTdf;

/// @brief      函数调用错误码枚举定义
///
/// @note       用于指示函数调用是否异常
typedef enum
{
    emBtnFuncErrorCode_None                            = 0,                            // 0  无错误
} emBtnFuncErrorCodeTdf;

/// @brief      设备静态参数结构定义
///
/// @note       通常情况下也可以理解为硬件参数
typedef struct
{
    GPIO_Regs*                                         pstGpioBase;                  // 使用的 GPIOx
    uint16_t                                           usGpioPin;                     // 使用的 PINx
    emBtnActiveLevelTdf                                emActiveLevel;                 // 按键有效电平
    uint32_t                                           ulDebounceCountThreshold;      // 消抖计数阈值
    uint32_t                                           ulLongPressCountThreshold;     // 长按计数阈值
    uint32_t                                           ulDoubleClickInterval;         // 双击间隔计数阈值
} stBtnStaticParamTdf;

/// @brief      设备动态参数结构定义
///
/// @note       通常情况下也可以理解为运行参数
typedef struct
{
    emBtnStatusTdf                                     emCurrentStatus;               // 当前状态
    emBtnStatusTdf                                     emLastStatus;                  // 上一次状态
    emBtnEventTypeTdf                                  emCurrentEvent;                // 当前事件
    uint32_t                                           ulDebounceCount;               // 消抖计数器
    uint32_t                                           ulPressCount;                  // 按下持续计数器
    uint32_t                                           ulClickCount;                  // 点击次数计数器
    uint32_t                                           ulDoubleClickCount;            // 双击间隔计数器
} stBtnRunningParamTdf;

/// @brief      设备参数结构定义
///
/// @note       含静态参数和动态参数
typedef struct
{
    stBtnStaticParamTdf                                stStaticParam;                 // 静态参数
    stBtnRunningParamTdf                               stRunningParam;                // 运行参数
} stBtnDeviceParamTdf;

const stBtnRunningParamTdf *c_pstGetBtnRunningParam(emBtnDevNumTdf emDevNum);
emBtnEventTypeTdf emBtnGetEvent(emBtnDevNumTdf emDevNum);
void vBtnClearEvent(emBtnDevNumTdf emDevNum);
void vBtnSetRunningParam(stBtnRunningParamTdf *pstInit, emBtnDevNumTdf emDevNum);
void vBtnExecute(emBtnDevNumTdf emDevNum);
emBtnFuncErrorCodeTdf emBtnDeviceInit(stBtnStaticParamTdf *pstInit, emBtnDevNumTdf emDevNum);



#endif /* UART_H */