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
 * 描述：
 * 该程序通过每分钟触发一次中断来演示RTC模式。
 * 日期在程序开始时进行初始化，并且还设置特定时间的附加警报以演示RTC_C模块的各种警报/事件模式。
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

//定义变量
 static volatile RTC_C_Calendar newTime;



 //增加变量
 unsigned char flag_disp;

 //时间是1955年,11月12日,周六, 10:03:00 PM
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
          UART_transmitData(EUSCI_A0_BASE,*s++);  //
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
    UART_transmitData(EUSCI_A0_BASE,((tempTime.year>>12)&0x0F) + 0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.year>>8 )&0x0F) + 0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.year>>4 )&0x0F) + 0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.year   )&0x0F) + 0x30);
    UART_transmitData(EUSCI_A0_BASE,'.');
    UART_transmitData(EUSCI_A0_BASE,((tempTime.month>>4) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.month) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,'.');
    UART_transmitData(EUSCI_A0_BASE,((tempTime.dayOfmonth>>4) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.dayOfmonth) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,'.');
    UART_transmitData(EUSCI_A0_BASE,((tempTime.hours>>4) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.hours) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,':');
    UART_transmitData(EUSCI_A0_BASE,((tempTime.minutes>>4) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.minutes) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,':');
    UART_transmitData(EUSCI_A0_BASE,((tempTime.seconds>>4) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.seconds) & 0x0F) +0x30);
   // UART_transmitData(EUSCI_A0_BASE,'\r');
    UART_transmitData(EUSCI_A0_BASE,'\n');
//    UART_transmitData(EUSCI_A0_BASE,tempTime.dayOfWeek +0x30);
//    UART_transmitData(EUSCI_A0_BASE,'.');
}

int main(void)
{

    unsigned char rtc_index;

    //关闭看门狗
    WDT_A_holdTimer();

    //配置外设功能引脚,连接低频晶振
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    //配置GPIO为输出
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //设置外部的时钟频率,非必须.  如果用户使用了getMCLK/getACLK等API, 则必须先调用该函数
    CS_setExternalClockSourceFrequency(32000,48000000);

    //启动LFXT, 非有源晶振模式
    CS_startLFXT(CS_LFXT_DRIVE3);

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    //使能FPU,用于DCO频率计算
    FPU_enableModule();

    //设置DCO为非标准频率12MHz
    CS_setDCOFrequency(12000000);

    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //配置UART
    UART_initModule(EUSCI_A0_BASE, &uartConfig);
    //使能UART
    UART_enableModule(EUSCI_A0_BASE);

    /* Enabling interrupts */
//    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//    Interrupt_enableInterrupt(INT_EUSCIA0);
//    Interrupt_enableSleepOnIsrExit();
//    Interrupt_enableMaster();

    uart_send_string("Init RTC.\r\n");
    //初始化RTC，current time
    RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);

    //测试RTC的初始化读数;
#if 1
    uart_send_string("RTC Register Write and Read TEST.\r\n");
    rtc_index = 0;
    while(1)
    {
        currentTime.minutes = TestTab[rtc_index];
        rtc_index ++;
        if(rtc_index == 60)rtc_index=0;
        RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);
        delay();
        newTime = RTC_C_getCalendarTime();
        Display_Time(newTime);
        delay();
    }
#endif

    //设置日历告警22:04 即10:04pm
   // RTC_C_setCalendarAlarm(0x04, 0x22, RTC_C_ALARMCONDITION_OFF,RTC_C_ALARMCONDITION_OFF);

    //设置事件，每分钟中断1次
   // RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);

    //使能RTC Ready Status中断，用来指示RTC日历寄存器可读. 同样,使能日历告警中断,和事件中断
   // RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT| RTC_C_CLOCK_ALARM_INTERRUPT);
   // RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT| RTC_C_CLOCK_ALARM_INTERRUPT);
    RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT);
    RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT);

    newTime = RTC_C_getCalendarTime();
    Display_Time(newTime);

    //开启RTC时钟
    RTC_C_startClock();

    //使能中断,进入休眠模式
    Interrupt_enableInterrupt(INT_RTC_C);
//    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableMaster();

//    newTime = RTC_C_getCalendarTime();
//    RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);

    flag_disp = 0;



    while(1)
    {
        //PCM_gotoLPM0();
        if(1 == flag_disp)
        {
            newTime = RTC_C_getCalendarTime();
            flag_disp=0;
            Display_Time(newTime);

        }
        __no_operation();
    }

}

//中断服务程序
void RTC_C_IRQHandler(void)
{
    uint32_t status;

    status = RTC_C_getEnabledInterruptStatus();
    RTC_C_clearInterruptFlag(status);

    if (status & RTC_C_CLOCK_READ_READY_INTERRUPT)
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
        flag_disp =1;
    }

    if (status & RTC_C_TIME_EVENT_INTERRUPT)
    {
        //此处设置断点, 每分钟中断一次
        __no_operation();
//        newTime = RTC_C_getCalendarTime();
    }

    if (status & RTC_C_CLOCK_ALARM_INTERRUPT)
    {
        //10:04pm 中断
        __no_operation();
    }

}


