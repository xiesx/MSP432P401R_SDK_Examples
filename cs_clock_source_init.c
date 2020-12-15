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
 * MSP432时钟系统- 时钟源初始化
 *
 * 描述：
 * 该例程中，演示函数CS_initClockSignal()的使用，将每个时钟都初始化为非默认值。
 * CS_initClockSignal函数是最方便的CS函数之一，因为它为用户提供简便的方法，给时钟信号配置特定时钟源。
 * 一旦每个时钟被初始化，可以通过函数获得时钟频率，将频率赋值为对应的变量。
 * 用户可以暂停调试器以观察已为变量分配了正确的频率。
 * MCLK = MODOSC/4 = 6MHz
 * ACLK = REFO/2 = 16kHz
 * HSMCLK = DCO/2 = 1.5Mhz
 * SMCLK = DCO/4 = 750kHz
 * BCLK  = REFO = 32kHz
 *
 *              MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *
 ******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
/* Statics */
static volatile uint32_t aclk, mclk, smclk, hsmclk, bclk;

int main(void)
{
    //停用看门狗
    MAP_WDT_A_holdTimer();

    //系统时钟的初始化配置如下:
    //系统时钟源默认值：MODOSC =24MHz, REFOCLK=32768, DCO= 3MHz
    //MCLK = MODOSC/4 = 6MHz
    //ACLK = REFO/2 = 16kHz
    //HSMCLK = DCO/2 = 1.5Mhz
    //SMCLK = DCO/4 = 750kHz
    //BCLK  = REFO = 32kHz
    MAP_CS_initClockSignal(CS_MCLK, CS_MODOSC_SELECT, CS_CLOCK_DIVIDER_4);
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_2);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4);
    MAP_CS_initClockSignal(CS_BCLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //使用函数读取当前的时钟值
    aclk = CS_getACLK();
    mclk = CS_getMCLK();
    smclk = CS_getSMCLK();
    hsmclk = CS_getHSMCLK();
    bclk = CS_getBCLK();

    //保持系统活动状态,然后通过调试器,查看这些变量的值
    while (1)
    {

    }
}

