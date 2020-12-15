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
 * MSP432 Clock System - Reference Oscillator
 *
 * ������
 * ����һ���򵥵Ĵ���ʾ���������ڲ��ο�����REFOCLK��ȡMCLK,��ʹ��SysTickģ��ʹLED��˸��
 * P1.1�����أ������жϣ��⽫�ı�����ģʽ��32khz��128 khz��֮��ο�������Ƶ�ʡ�
 *
 *              MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST         P1.0  |---> P1.0 LED
 *            |                  |
 *            |            P1.1  |<--Toggle Switch
 *            |                  |
 *            |                  |
 *
 ******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Statics */
static volatile bool is32khz;

int main(void)
{
    //ͣ�ÿ��Ź�
    MAP_WDT_A_holdTimer();
    is32khz = true;

    //��������P1.0Ϊ���,���LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    //��������P1.1Ϊ����,ʹ���ڲ�����;
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

    //����MCLK=REFO,  REFOCLKĬ��Ϊ32768
    MAP_CS_initClockSignal(CS_MCLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //ʱ��Ϊ32khz,���Թ�����ģʽLF
    MAP_PCM_setPowerMode(PCM_LF_MODE);

    //����SysTick����ֵΪ16000.
    //��32khz��,LEDȡ����Ƶ��ԼΪ0.5s, ���ʱ��Ϊ128K,��4������
    MAP_SysTick_enableModule();
    MAP_SysTick_setPeriod(16000);
    MAP_SysTick_enableInterrupt();
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    MAP_Interrupt_enableInterrupt(INT_PORT1);

    //ʹ��ȫ���ж�
    MAP_Interrupt_enableMaster();

    while (1)
    {
        MAP_PCM_gotoLPM0();
    }
}

/* SysTick ISR */
void SysTick_Handler(void)
{
    MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

/* Port 1 ISR */
void PORT1_IRQHandler(void)
{
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    if(status & GPIO_PIN1)
    {

        if(is32khz)
            MAP_CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
        else
            MAP_CS_setReferenceOscillatorFrequency(CS_REFO_32KHZ);

        is32khz = !is32khz;

    }

}

