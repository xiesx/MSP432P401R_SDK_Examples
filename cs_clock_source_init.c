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
 * MSP432ʱ��ϵͳ- ʱ��Դ��ʼ��
 *
 * ������
 * �������У���ʾ����CS_initClockSignal()��ʹ�ã���ÿ��ʱ�Ӷ���ʼ��Ϊ��Ĭ��ֵ��
 * CS_initClockSignal����������CS����֮һ����Ϊ��Ϊ�û��ṩ���ķ�������ʱ���ź������ض�ʱ��Դ��
 * һ��ÿ��ʱ�ӱ���ʼ��������ͨ���������ʱ��Ƶ�ʣ���Ƶ�ʸ�ֵΪ��Ӧ�ı�����
 * �û�������ͣ�������Թ۲���Ϊ������������ȷ��Ƶ�ʡ�
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
    //ͣ�ÿ��Ź�
    MAP_WDT_A_holdTimer();

    //ϵͳʱ�ӵĳ�ʼ����������:
    //ϵͳʱ��ԴĬ��ֵ��MODOSC =24MHz, REFOCLK=32768, DCO= 3MHz
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

    //ʹ�ú�����ȡ��ǰ��ʱ��ֵ
    aclk = CS_getACLK();
    mclk = CS_getMCLK();
    smclk = CS_getSMCLK();
    hsmclk = CS_getHSMCLK();
    bclk = CS_getBCLK();

    //����ϵͳ�״̬,Ȼ��ͨ��������,�鿴��Щ������ֵ
    while (1)
    {

    }
}

