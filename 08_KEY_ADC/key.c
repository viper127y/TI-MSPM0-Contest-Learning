/**
  * @file       button_device.c
  * @author     拉咯比哩
  * @version    V1.0.1
  * @date       20251203
  * @brief      独立按键驱动，基于 STM32 HAL 库
  *
  * <h2><center>&copy;此文件版权归【拉咯比哩】所有.</center></h2>
  */

#include "key.h"
#include "string.h"

/// @brief      设备实例
///
/// @note
static stBtnDeviceParamTdf s_astBtnDevice[BTN_DEV_NUM];

/// @brief      静态参数拷贝
///
/// @param      pstInit     : 初始化参数
/// @param      emDevNum    : 设备号
///
/// @note
static void s_vBtnStaticParamCopy(stBtnStaticParamTdf *pstInit, emBtnDevNumTdf emDevNum)
{
    memcpy(&s_astBtnDevice[emDevNum].stStaticParam, pstInit, sizeof(stBtnStaticParamTdf) / sizeof(uint8_t));
}

/// @brief      获取运行参数指针
///
/// @param      emDevNum    : 设备号
///
/// @note       注意，获取的指针，指向的内容是只读的
const stBtnRunningParamTdf *c_pstGetBtnRunningParam(emBtnDevNumTdf emDevNum)
{
    return &s_astBtnDevice[emDevNum].stRunningParam;
}

/// @brief      运行参数设置
///
/// @param      pstInit     : 要设置的运行参数
/// @param      emDevNum    : 设备号
///
/// @note       参数为 0xFFFF 或 0xFFFFFFFF 时，表示不做更改
void vBtnSetRunningParam(stBtnRunningParamTdf *pstInit, emBtnDevNumTdf emDevNum)
{
    if(pstInit->emCurrentStatus != emBtnStatus_NotModify)
    {
        s_astBtnDevice[emDevNum].stRunningParam.emCurrentStatus = pstInit->emCurrentStatus;
    }
    
    if(pstInit->emLastStatus != emBtnStatus_NotModify)
    {
        s_astBtnDevice[emDevNum].stRunningParam.emLastStatus = pstInit->emLastStatus;
    }
    
    if(pstInit->ulDebounceCount != 0xFFFFFFFF)
    {
        s_astBtnDevice[emDevNum].stRunningParam.ulDebounceCount = pstInit->ulDebounceCount;
    }
    
    if(pstInit->ulPressCount != 0xFFFFFFFF)
    {
        s_astBtnDevice[emDevNum].stRunningParam.ulPressCount = pstInit->ulPressCount;
    }
    
    if(pstInit->ulClickCount != 0xFFFFFFFF)
    {
        s_astBtnDevice[emDevNum].stRunningParam.ulClickCount = pstInit->ulClickCount;
    }
    
    if(pstInit->ulDoubleClickCount != 0xFFFFFFFF)
    {
        s_astBtnDevice[emDevNum].stRunningParam.ulDoubleClickCount = pstInit->ulDoubleClickCount;
    }
}

/// @brief      获取按键事件
///
/// @param      emDevNum    : 设备号
///
/// @retval     当前按键事件
emBtnEventTypeTdf emBtnGetEvent(emBtnDevNumTdf emDevNum)
{
    return s_astBtnDevice[emDevNum].stRunningParam.emCurrentEvent;
}

/// @brief      清除按键事件
///
/// @param      emDevNum    : 设备号
void vBtnClearEvent(emBtnDevNumTdf emDevNum)
{
    s_astBtnDevice[emDevNum].stRunningParam.emCurrentEvent = emBtnEvent_None;
}


typedef enum
{
  GPIO_PIN_RESET = 0u,
  GPIO_PIN_SET
} GPIO_PinState;

 


/// @brief      按键状态读取与消抖处理
///
/// @param      emDevNum    : 设备号
///
/// @retval     经过消抖后的按键状态
static emBtnStatusTdf s_emBtnReadStatus(emBtnDevNumTdf emDevNum)
{
        uint8_t uchPinLevel;
        emBtnStatusTdf emCurrentLevel;

        // 读取引脚电平（DL 库返回位掩码，非 0 即高电平，转为 0/1）
        uchPinLevel = (DL_GPIO_readPins(s_astBtnDevice[emDevNum].stStaticParam.pstGpioBase,
                                        s_astBtnDevice[emDevNum].stStaticParam.usGpioPin) != 0) ? 1u : 0u;

        // 转换为按键状态（按下/释放）
        emCurrentLevel = (uchPinLevel == s_astBtnDevice[emDevNum].stStaticParam.emActiveLevel) ? 
                        emBtnStatus_Pressed : emBtnStatus_Released;
    
    // 消抖处理
    if(emCurrentLevel != s_astBtnDevice[emDevNum].stRunningParam.emCurrentStatus)
    {
        s_astBtnDevice[emDevNum].stRunningParam.ulDebounceCount++;
        if(s_astBtnDevice[emDevNum].stRunningParam.ulDebounceCount >= 
           s_astBtnDevice[emDevNum].stStaticParam.ulDebounceCountThreshold)
        {
            s_astBtnDevice[emDevNum].stRunningParam.emCurrentStatus = emCurrentLevel;
            s_astBtnDevice[emDevNum].stRunningParam.ulDebounceCount = 0;
        }
    }
    else
    {
        s_astBtnDevice[emDevNum].stRunningParam.ulDebounceCount = 0;
    }
    
    return s_astBtnDevice[emDevNum].stRunningParam.emCurrentStatus;
}

/// @brief      按键事件检测
///
/// @param      emDevNum    : 设备号
///
/// @note       需配合周期调用的 vBtnExecute 使用
static void s_vBtnDetectEvent(emBtnDevNumTdf emDevNum)
{
    emBtnStatusTdf emCurrentStatus = s_astBtnDevice[emDevNum].stRunningParam.emCurrentStatus;
    emBtnStatusTdf emLastStatus = s_astBtnDevice[emDevNum].stRunningParam.emLastStatus;
    
    // 状态变化处理
    if(emCurrentStatus != emLastStatus)
    {
        // 按下状态处理
        if(emCurrentStatus == emBtnStatus_Pressed)
        {
            s_astBtnDevice[emDevNum].stRunningParam.ulPressCount = 0;
        }
        // 释放状态处理
        else
        {
            // 判断是否为长按
            if(s_astBtnDevice[emDevNum].stRunningParam.ulPressCount >= 
               s_astBtnDevice[emDevNum].stStaticParam.ulLongPressCountThreshold)
            {
                s_astBtnDevice[emDevNum].stRunningParam.emCurrentEvent = emBtnEvent_LongPress;
                s_astBtnDevice[emDevNum].stRunningParam.ulClickCount = 0;
            }
            // 短按处理
            else
            {
                s_astBtnDevice[emDevNum].stRunningParam.ulClickCount++;
                s_astBtnDevice[emDevNum].stRunningParam.ulDoubleClickCount = 0;
            }
        }
    }
    // 状态未变化处理
    else
    {
        // 按下状态持续计数
        if(emCurrentStatus == emBtnStatus_Pressed)
        {
            s_astBtnDevice[emDevNum].stRunningParam.ulPressCount++;
        }
        // 释放状态处理双击判断
        else
        {
            if(s_astBtnDevice[emDevNum].stRunningParam.ulClickCount == 1)
            {
                s_astBtnDevice[emDevNum].stRunningParam.ulDoubleClickCount++;
                if(s_astBtnDevice[emDevNum].stRunningParam.ulDoubleClickCount >= 
                   s_astBtnDevice[emDevNum].stStaticParam.ulDoubleClickInterval)
                {
                    s_astBtnDevice[emDevNum].stRunningParam.emCurrentEvent = emBtnEvent_Click;
                    s_astBtnDevice[emDevNum].stRunningParam.ulClickCount = 0;
                }
            }
            else if(s_astBtnDevice[emDevNum].stRunningParam.ulClickCount == 2)
            {
                s_astBtnDevice[emDevNum].stRunningParam.emCurrentEvent = emBtnEvent_DoubleClick;
                s_astBtnDevice[emDevNum].stRunningParam.ulClickCount = 0;
            }
        }
    }
    
    // 更新上一次状态
    s_astBtnDevice[emDevNum].stRunningParam.emLastStatus = emCurrentStatus;
}

/// @brief      按键执行
///
/// @param      emDevNum    : 设备号
///
/// @note       需要周期调用执行，建议调用周期10ms
void vBtnExecute(emBtnDevNumTdf emDevNum)
{
    // 1. 读取按键状态（含消抖）
    s_emBtnReadStatus(emDevNum);
    
    // 2. 检测按键事件
    s_vBtnDetectEvent(emDevNum);
}

/// @brief      设备初始化
///
/// @param      pstInit     : 初始化结构体实例指针
/// @param      emDevNum    : 设备号
///
/// @retval     调用是否成功，详见 emBtnFuncErrorCodeTdf 定义
///
/// @note
emBtnFuncErrorCodeTdf emBtnDeviceInit(stBtnStaticParamTdf *pstInit, emBtnDevNumTdf emDevNum)
{
    // 1. 拷贝静态参数
    s_vBtnStaticParamCopy(pstInit, emDevNum);
    
    // 2. 初始化运行参数
    s_astBtnDevice[emDevNum].stRunningParam.emCurrentStatus = emBtnStatus_Released;
    s_astBtnDevice[emDevNum].stRunningParam.emLastStatus = emBtnStatus_Released;
    s_astBtnDevice[emDevNum].stRunningParam.emCurrentEvent = emBtnEvent_None;
    s_astBtnDevice[emDevNum].stRunningParam.ulDebounceCount = 0;
    s_astBtnDevice[emDevNum].stRunningParam.ulPressCount = 0;
    s_astBtnDevice[emDevNum].stRunningParam.ulClickCount = 0;
    s_astBtnDevice[emDevNum].stRunningParam.ulDoubleClickCount = 0;
    
    // 3. 返回错误码
    return emBtnFuncErrorCode_None;
}