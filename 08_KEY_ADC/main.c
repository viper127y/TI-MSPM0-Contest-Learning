/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 
#include "ti_msp_dl_config.h"
#include "delay.h"
#include "oled.h"
#include "uart.h"
#include "key.h"
/* =============================================== C库函数（SysTick，1ms 心跳） =================================================================== */
#include <stdio.h>
 
 
/* ============ 滴答定时器（SysTick，1ms 心跳） ============ */
static volatile uint32_t s_ulSysTick = 0;   // 系统毫秒计数，中断里自增
static uint8_t s_ucScanDiv = 0;
 

/// @brief      配置 SysTick 每 1ms 产生一次中断
static void vSysTickInit(void)
{
    SysTick_Config(CPUCLK_FREQ / 1000);     // 80MHz / 1000 = 每 80000 个时钟节拍 1ms
}

/// @brief      SysTick 中断服务函数（函数名必须和向量表一致，不能改）
void SysTick_Handler(void)
{
    s_ulSysTick++;      // 只数心跳，不干活，保持中断极短
    if(++s_ucScanDiv >= 10)
    {
        //扫描10分频率
        s_ucScanDiv = 0 ;
        //每10ms周期执行
        vBtnExecute(emBtnDevNum0);     
    }

}



void vKeyInit()
{
    stBtnStaticParamTdf stKeyStaticInit;
    stKeyStaticInit.pstGpioBase                 = KEY_PORT;
    stKeyStaticInit.usGpioPin                   = KEY_KEY9_PIN;
    stKeyStaticInit.emActiveLevel                = emBtnActiveLevel_Low;   // 实测按下=0，改 Low
    stKeyStaticInit.ulDebounceCountThreshold    = 1;
    stKeyStaticInit.ulLongPressCountThreshold   =100;
    stKeyStaticInit.ulDoubleClickInterval       = 20;
    emBtnDeviceInit(&stKeyStaticInit, emBtnDevNum0);
}

 

//中断版本呢
// void GROUP1_IRQHandler()
// {

//     //1.判断中断源
//     switch (DL_GPIO_GetPendingInterrut(KEY_PORT))
//     {
//         case KEY_KEY9_IIDX:
//         //status = (status + 1) % 3;
//         break;

//         case KEY_KEY10_IIDX:
//         //status = (status + 3 -1) % 3;
//         break;
//     }


// }





int main(void)
{
   //*********************************************SYSTEM INIT*************************************************************************** */

    SYSCFG_DL_init();

    //*********************************************USER INIT*************************************************************************** */
 
    vKeyInit();
    OLED_Init();
    vSysTickInit();

    //*********************************************USER CODE*************************************************************************** */  
    OLED_ColorTurn(0);//0正常显示，1 反色显示
    OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
    OLED_Clear();
    //NVIC_EnableIRQ(PRINT_INST_INT_IRQN);


    int iFlag = 0;
    int iLastFlag = -1;         // 上次刷屏的 status，-1 强制首帧刷新

    static uint16_t usLastMv = 0xFFFF ; // 上次显示的电压(mV), 0xFFFF 强制首刷
    static uint16_t usShowMv = 0 ; // 当前显示用的电压(mV)


    uint8_t bNeedRefresh = 0;


 
    while (1)
     {
 
        //通知ADC开始采样
        DL_ADC12_startConversion(ADC_Input_1_INST);
        uint16_t ulResult = DL_ADC12_getMemResult(ADC_Input_1_INST,ADC_Input_1_ADCMEM_0);

        char acOLed_Buffer[30];
        uint16_t usMv = ( uint16_t )(( uint32_t )ulResult * 2500UL / 4095UL ); // 当前电压
        int32_t lDiff = ( int32_t )usMv - ( int32_t )usLastMv;


            // 电压行 (y=2): 电压变了才写显存
            if (usLastMv == 1 || (lDiff >300 ) || (lDiff < -300 )) 
            {          // 0.3V 量化值, 变了才算
                usShowMv = usMv;
                usLastMv = usMv;
                snprintf(acOLed_Buffer, sizeof(acOLed_Buffer), "ADC: %d.%dV", usShowMv / 1000, (usShowMv % 1000) / 100 );
                OLED_ShowString(0, 30, (uint8_t*)acOLed_Buffer, 16);
                bNeedRefresh = 1;
            }
 
            // 只有内容变化才刷屏：OLED_Refresh 约 250ms，
             // 每圈都刷会把主循环拖慢，导致按键扫描检查 250ms 才执行一次
            if (iFlag != iLastFlag)
            {
                iLastFlag = iFlag;
               
            if (iFlag == 0)
            {
                OLED_ShowString(0, 0, (uint8_t*)"status: 0", 16);
            }
            else if (iFlag == 1)
            {
                OLED_ShowString(0, 0, (uint8_t*)"status: 1", 16);
            }
            else
            {
                OLED_ShowString(0, 0, (uint8_t*)"status: 2", 16);
            }
     
                bNeedRefresh = 1;
            }
 
            // 唯一的刷新出口
            if (bNeedRefresh) 
            {
                bNeedRefresh = 0;
                OLED_Refresh();
            }
  
 
        switch (emBtnGetEvent(emBtnDevNum0))
        {
            case emBtnEvent_Click:
                iFlag = (iFlag + 1) % 3;
                vBtnClearEvent(emBtnDevNum0);
                break;

            default:

                break;
        } ; 

 

      


    }
}
