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
 * �����ڳ���ʼʱ���г�ʼ�������һ������ض�ʱ��ĸ��Ӿ�������ʾRTC_Cģ��ĸ��־���/�¼�ģʽ��
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
 * *               MSP432P401
 *             -----------------
 *            |                 |
 *            |                 |
 *            |                 |
 *       RST -|     P1.3/UCA0TXD|----> PC (echo)
 *            |                 |
 *            |                 |
 *            |     P1.2/UCA0RXD|<---- PC
 *            |                 |
 ******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>


#include <stdint.h>
#include <stdbool.h>

//�������
 static volatile RTC_C_Calendar newTime;



 //���ӱ���
 unsigned char flag_disp;

 //ʱ����1955��,11��12��,����, 10:03:00 PM
RTC_C_Calendar currentTime =
 {
         0x00,
         0x39,
         0x22,
         0x06,
         0x21,
         0x09,
         0x2019
 };

 const unsigned char TestTab[]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
                                0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
                                0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,
                                0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
                                0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
                                0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,};

 //115200, 12M
 const eUSCI_UART_Config uartConfig =
 {
         EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
         6,                                     // BRDIV = 78
         8,                                       // UCxBRF = 2
         32,                                       // UCxBRS = 0
         EUSCI_A_UART_NO_PARITY,                  // No Parity
         EUSCI_A_UART_LSB_FIRST,                  // LSB First
         EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
         EUSCI_A_UART_MODE,                       // UART mode
         EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
 };

void uart_send_string(char *s)
{
      while (*s)
          MAP_UART_transmitData(EUSCI_A0_BASE,*s++);  //
}

void delay(void)
{
    unsigned int i,j;
    for(i=30000;i>0;i--)
    {
        for(j=10;j>0;j--);
    }

}

//#define BCD_TO_ASCCII(val)    (((((val) >> 8) + 0x30) << 8) +  (((val) & 0xFF) + 0x30))


void Display_Time(RTC_C_Calendar tempTime)
{
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.year>>12)&0x0F) + 0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.year>>8 )&0x0F) + 0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.year>>4 )&0x0F) + 0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.year   )&0x0F) + 0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,'.');
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.month>>4) & 0x0F) +0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.month) & 0x0F) +0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,'.');
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.dayOfmonth>>4) & 0x0F) +0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.dayOfmonth) & 0x0F) +0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,'.');
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.hours>>4) & 0x0F) +0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.hours) & 0x0F) +0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,':');
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.minutes>>4) & 0x0F) +0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.minutes) & 0x0F) +0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,':');
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.seconds>>4) & 0x0F) +0x30);
    MAP_UART_transmitData(EUSCI_A0_BASE,((tempTime.seconds) & 0x0F) +0x30);
   // MAP_UART_transmitData(EUSCI_A0_BASE,'\r');
    MAP_UART_transmitData(EUSCI_A0_BASE,'\n');
//    MAP_UART_transmitData(EUSCI_A0_BASE,tempTime.dayOfWeek +0x30);
//    MAP_UART_transmitData(EUSCI_A0_BASE,'.');
}

int main(void)
{

    unsigned char rtc_index;

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

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    //ʹ��FPU,����DCOƵ�ʼ���
    MAP_FPU_enableModule();

    //����DCOΪ�Ǳ�׼Ƶ��12MHz
    MAP_CS_setDCOFrequency(12000000);

    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //����UART
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
    //ʹ��UART
    MAP_UART_enableModule(EUSCI_A0_BASE);

    /* Enabling interrupts */
//    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
//    MAP_Interrupt_enableSleepOnIsrExit();
//    MAP_Interrupt_enableMaster();

    uart_send_string("Init RTC.\r\n");
    //��ʼ��RTC��current time
    MAP_RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);

    //����RTC�ĳ�ʼ������;
#if 1
    uart_send_string("RTC Register Write and Read TEST.\r\n");
    rtc_index = 0;
    while(1)
    {
        currentTime.minutes = TestTab[rtc_index];
        rtc_index ++;
        if(rtc_index == 60)rtc_index=0;
        MAP_RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);
        delay();
        newTime = RTC_C_getCalendarTime();
        Display_Time(newTime);
        delay();
    }
#endif

    //���������澯22:04 ��10:04pm
   // MAP_RTC_C_setCalendarAlarm(0x04, 0x22, RTC_C_ALARMCONDITION_OFF,RTC_C_ALARMCONDITION_OFF);

    //�����¼���ÿ�����ж�1��
   // MAP_RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);

    //ʹ��RTC Ready Status�жϣ�����ָʾRTC�����Ĵ����ɶ�. ͬ��,ʹ�������澯�ж�,���¼��ж�
   // MAP_RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT| RTC_C_CLOCK_ALARM_INTERRUPT);
   // MAP_RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT| RTC_C_CLOCK_ALARM_INTERRUPT);
    MAP_RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT);
    MAP_RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT);

    newTime = RTC_C_getCalendarTime();
    Display_Time(newTime);

    //����RTCʱ��
    MAP_RTC_C_startClock();

    //ʹ���ж�,��������ģʽ
    MAP_Interrupt_enableInterrupt(INT_RTC_C);
//    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableMaster();

//    newTime = RTC_C_getCalendarTime();
//    RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);

    flag_disp = 0;



    while(1)
    {
        //MAP_PCM_gotoLPM0();
        if(1 == flag_disp)
        {
            newTime = RTC_C_getCalendarTime();
            flag_disp=0;
            Display_Time(newTime);

        }
        __no_operation();
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
        flag_disp =1;
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


