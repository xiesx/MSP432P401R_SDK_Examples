/*******************************************************************************
 * MSP432 RTC实时钟例程
 *
 * 描述：
 * 例程演示了RTC时钟的基本功能。
 * 在程序开始时初始化日期，并设置了闹钟和事件。开启了3个中断:
 * (1)允许读数中断即每秒中断, 将LED灯的输出取反;
 * (2)事件中断,每分钟,可以中断中设置断点,查看当前的时间;
 * (3)闹钟,发生1次;
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
 * by: xie_sx@126.com
 ******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//定义变量
static volatile RTC_C_Calendar newTime;

//时间是1955年,11月12日,周六, 10:03:00 PM
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
    //关闭看门狗
    WDT_A_holdTimer();

    //配置外设功能引脚,连接低频晶振
   GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    //配置GPIO为输出
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //设置外部的时钟频率,非必须.  如果用户使用了getMCLK/getACLK等API, 则必须先调用该函数
    CS_setExternalClockSourceFrequency(32000,48000000);

    //启动LFXT, 非有源晶振模式
    CS_startLFXT(CS_LFXT_DRIVE3);

    //初始化RTC，current time
    RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);

    //设置日历闹钟22:04 即10:04pm
    RTC_C_setCalendarAlarm(0x04, 0x22, RTC_C_ALARMCONDITION_OFF,RTC_C_ALARMCONDITION_OFF);

    //设置事件，每分钟中断1次
    RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);

    //使能RTC Ready Status中断，用来指示RTC日历寄存器可读. 同样,使能日历闹钟中断,和事件中断
    RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT | RTC_C_CLOCK_ALARM_INTERRUPT);
    RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT| RTC_C_CLOCK_ALARM_INTERRUPT);

//    newTime = RTC_C_getCalendarTime();
    //开启RTC时钟
    RTC_C_startClock();

    //使能中断,进入休眠模式
    Interrupt_enableInterrupt(INT_RTC_C);
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableMaster();

    while(1)
    {
        PCM_gotoLPM0();
//        __no_operation();
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
    }

    if (status & RTC_C_TIME_EVENT_INTERRUPT)
    {
        //此处设置断点, 每分钟中断一次
        newTime =RTC_C_getCalendarTime();
        __no_operation();
    }

    if (status & RTC_C_CLOCK_ALARM_INTERRUPT)
    {
        //10:04pm 中断
        __no_operation();
    }

}
