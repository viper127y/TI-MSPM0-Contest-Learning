#include "ti_msp_dl_config.h"
#include "delay.h"



void Vdelay_ms(uint32_t ms)
{
    //逻辑：CPU 跑 CPUCLK_FREQ 个周期 = 1 秒 → 跑 CPUCLK_FREQ/1000 个周期 ≈ 1 毫秒 → 乘以 ms 就是延时 ms 毫秒。
    uint32_t cycles = (CPUCLK_FREQ / 1000) * ms;
    delay_cycles(cycles);

}
