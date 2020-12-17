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
 * MSP432 RTC_C - Calendar Mode
 *
 * ������
 * �ó���ͨ��ÿ���Ӵ���һ���ж�����ʾRTCģʽ��
 * �����ڳ���ʼʱ���г�ʼ�������һ������ض�ʱ�����������ʾRTC_Cģ��ĸ�������/�¼�ģʽ��
 *
 *                MSP432P401
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
 ******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//�������
static volatile RTC_C_Calendar newTime;

//ʱ����1955��,11��12��,����, 10:03:00 PM
const RTC_C_Calendar currentTime =
{
     0x00,
     0x03,
     0x22,
     0x06,
     0x12,
     0x11,
     0x1955
};

int main(void)
{
    //�رտ��Ź�
    MAP_WDT_A_holdTimer();

    //�������蹦������,���ӵ�Ƶ����
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    //����GPIOΪ���
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //�����ⲿ��ʱ��Ƶ��,�Ǳ���.  ����û�ʹ����getMCLK/getACLK��API, ������ȵ��øú���
    CS_setExternalClockSourceFrequency(32000,48000000);

    //����LFXT, ����Դ����ģʽ
    CS_startLFXT(CS_LFXT_DRIVE3);

    //��ʼ��RTC��current time
    MAP_RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);
//    RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);
    //������������22:04 ��10:04pm
    MAP_RTC_C_setCalendarAlarm(0x04, 0x22, RTC_C_ALARMCONDITION_OFF,RTC_C_ALARMCONDITION_OFF);

    //�����¼���ÿ�����ж�1��
    MAP_RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);

    //ʹ��RTC Ready Status�жϣ�����ָʾRTC�����Ĵ����ɶ�. ͬ��,ʹ�����������ж�,���¼��ж�
    MAP_RTC_C_clearInterruptFlag(
            RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT
                    | RTC_C_CLOCK_ALARM_INTERRUPT);
    MAP_RTC_C_enableInterrupt(
            RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT
                    | RTC_C_CLOCK_ALARM_INTERRUPT);


//    newTime = RTC_C_getCalendarTime();
    //����RTCʱ��
    MAP_RTC_C_startClock();

    //ʹ���ж�,��������ģʽ
    MAP_Interrupt_enableInterrupt(INT_RTC_C);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableMaster();

    while(1)
    {
        MAP_PCM_gotoLPM0();
//        __no_operation();
    }

}

//�жϷ������
void RTC_C_IRQHandler(void)
{
    uint32_t status;

    status = MAP_RTC_C_getEnabledInterruptStatus();
    MAP_RTC_C_clearInterruptFlag(status);

    if (status & RTC_C_CLOCK_READ_READY_INTERRUPT)
    {
        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }

    if (status & RTC_C_TIME_EVENT_INTERRUPT)
    {
        //�˴����öϵ�, ÿ�����ж�һ��
        __no_operation();
//        newTime = MAP_RTC_C_getCalendarTime();
    }

    if (status & RTC_C_CLOCK_ALARM_INTERRUPT)
    {
        //10:04pm �ж�
        __no_operation();
    }

}


