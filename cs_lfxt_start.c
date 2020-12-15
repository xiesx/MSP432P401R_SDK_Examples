/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
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
 * --/COPYRIGHT--*/
/*******************************************************************************
 * MSP432 Clock System - LFXT例程
 *
 * 描述：
 * 这是一个简单的代码示例，它启动连接到LFXTIN / LFXTOUT的32khz晶体，从晶体获取MCLK，
 * 然后使用SysTick（源自MCLK）使LED闪烁。
 * 这是一个非常基本的代码示例，它显示了如何从LFXT启动loq频率晶体和源时钟信号。
 * 由于我们将MCLK设置为从32khz示例运行，因此我们能够以LPM模式运行。
 *
 * 该程序无限运行，直到用户手动暂停。
 *
 *              MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST         P1.0  |---> P1.0 LED
 *            |     PJ.0 LFXIN   |---------
 *            |                  |         |
 *            |                  |     < 32khz xTal >
 *            |                  |         |
 *            |     PJ.1 LFXOUT  |---------
 *
  *******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

int main(void)
{
    //停用看门狗
    MAP_WDT_A_holdTimer();

    //配置引脚PJ.0和PJ.1为功能引脚,外接晶振
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    //配置引脚P1.0为输出,接LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //设置外部时钟频率,这个步骤是可选的，如果后续使用了getMCLK/getACLK等函数，则必须配置
    CS_setExternalClockSourceFrequency(32000,48000000);

    //启动LFXT,不带超时判断
    CS_startLFXT(CS_LFXT_DRIVE3);

    //配置时钟MCLK=LFXT, LFXT=32khz
    MAP_CS_initClockSignal(CS_MCLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //系统工作在32khz,可配置为LF模式
    MAP_PCM_setPowerMode(PCM_LF_MODE);

    //配置SysTick的计数值为16000 (MCLK=32khz),取反时间为 0.5s
    MAP_SysTick_enableModule();
    MAP_SysTick_setPeriod(16000);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_SysTick_enableInterrupt();

    //使能全局中断
    MAP_Interrupt_enableMaster();

    while (1)
    {
        MAP_PCM_gotoLPM0();
    }
}

void SysTick_Handler(void)
{
    MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
}
