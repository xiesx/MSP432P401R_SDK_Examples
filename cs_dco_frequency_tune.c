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
 * MSP432 Clock System - ����DCOƵ��Ϊ�Ǳ�׼Ƶ��
 *
 * ������
 * ����������У�����չʾ�˽�DCOƵ������Ϊ�Ǳ�׼Ƶ��ֵ
 * ����MSP432�ܹ���DCOƵ�ʾ�ϸ�������Ǳ�׼Ƶ�ʣ���˿���ʹ��CS_setDCOFrequencyƵ��������ɡ�
 * ��Ӧ�ó���DCOƵ������Ϊ8.33MHz��Ȼ��ʹ��SysTickģ����P1.0����˸��LED��
 *
 *              MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST         P1.0  |---> P1.0 LED
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
#include <stdbool.h>

int main(void)
{
    //ͣ�ÿ��Ź�
    MAP_WDT_A_holdTimer();

    //��������P1.0Ϊ���,��LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    
    //ʹ��FPU,����DCOƵ�ʼ���
    MAP_FPU_enableModule();
    
    //����DCOΪ�Ǳ�׼Ƶ��8.33MHz
    MAP_CS_setDCOFrequency(8330000);
    
    //����SysTick����ֵΪ4165000 (MCLK=8.33MHz,ȡ��ʱ�� ~0.5s)
    MAP_SysTick_enableModule();
    MAP_SysTick_setPeriod(4165000);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_SysTick_enableInterrupt();
    
    //ʹ��ȫ���ж�
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

