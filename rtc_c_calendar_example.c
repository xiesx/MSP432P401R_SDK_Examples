/*******************************************************************************
 * MSP432 实时钟练习
 *
 * 描述：
 * 例程演示实时钟的基本操作。将系统的当前时间通过串口送到PC端显示出来。
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
 *            |                  |
 *            |      P1.3/UCA0TXD|----> PC (echo)
 *            |      P1.2/UCA0RXD|<---- PC
 *            |                  |
 ******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include <stdint.h>
#include <stdbool.h>

//定义变量
 static volatile RTC_C_Calendar newTime;

 //增加变量
 unsigned char flag_disp;

 //时间是2019年12月17日,周四, 23:58:00 PM
RTC_C_Calendar currentTime =
 {
         0x00,
         0x58,
         0x23,
         0x04,
         0x17,
         0x12,
         0x2019
 };

 //115200, 12M
 const eUSCI_UART_ConfigV1 uartConfig =
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
    UART_transmitData(EUSCI_A0_BASE,'    ');
    UART_transmitData(EUSCI_A0_BASE,((tempTime.hours>>4) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.hours) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,':');
    UART_transmitData(EUSCI_A0_BASE,((tempTime.minutes>>4) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.minutes) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,':');
    UART_transmitData(EUSCI_A0_BASE,((tempTime.seconds>>4) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,((tempTime.seconds) & 0x0F) +0x30);
    UART_transmitData(EUSCI_A0_BASE,'\r');
    UART_transmitData(EUSCI_A0_BASE,'\n');
}

int main(void)
{
    //关闭看门狗
   WDT_A_holdTimer();

   //使能FPU,用于DCO频率计算
   FPU_enableModule();

    //配置外设功能引脚,连接低频晶振
   GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    //配置GPIO为输出
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //设置外部的时钟频率,非必须.  如果用户使用了getMCLK/getACLK等API, 则必须先调用该函数
    CS_setExternalClockSourceFrequency(32000,48000000);

    //启动LFXT, 非有源晶振模式
    CS_startLFXT(CS_LFXT_DRIVE3);

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    //设置DCO为非标准频率12MHz
    CS_setDCOFrequency(CS_12MHZ);

    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //配置UART
    UART_initModule(EUSCI_A0_BASE, &uartConfig);
    //使能UART
    UART_enableModule(EUSCI_A0_BASE);

    uart_send_string("Init RTC.\r\n");
    //初始化RTC，current time
    RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BCD);

    //使能RTC Ready Status中断
    RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT);
    RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT);

    newTime = RTC_C_getCalendarTime();
    Display_Time(newTime);

    //使能中断,进入休眠模式
    Interrupt_enableInterrupt(INT_RTC_C);
    Interrupt_enableMaster();

    flag_disp = 0;
    //开启RTC时钟
    RTC_C_startClock();

    while(1)
    {
        if(1 == flag_disp)
        {
            flag_disp = 0;
            Display_Time(newTime);
        }
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
        newTime = RTC_C_getCalendarTime();
        flag_disp =1;
    }
}
