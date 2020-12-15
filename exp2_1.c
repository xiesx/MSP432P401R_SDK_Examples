//*****************************************************************************
// 基础练习2.1: UART的基本收发实验
//
// 波特率计算可以使用以下网页小工具:
// http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
//
// -------------MSP-EXP432P401R硬件描述 -----------------
// (1) 顶部 1个复位按键,S3
// (2) 左右两侧各1个用户按键,S1(P1.1),S2(P1.4)
// (3) 1个单色发光管LED1(P1.0红)
// (4) 1个三色发光管LED2(P2.0红/P2.1绿/P2.2蓝)
//
//         MSP-EXP432P401R
//         +-------------+
//     /|\ |             |
//      |  |             |
//  S3  +--|RST          |
//         |         P1.0|---> LED1(红)
//  S1 --->|P1.1         |
//         |         P2.0|---> LED2(红)
//  S2 --->|P1.4     P2.1|---> LED2(绿)
//         |         P2.2|---> LED2(蓝)
//         |             |
//         | P1.3/UCA0TXD|----> PC
//         | P1.2/UCA0RXD|<---- PC
//         |             |
//         |             |
// 按键引脚设置为输入,且使能内部上拉电阻.按键未按下时,读取引脚是高电平;按键按下时,接地,读取引脚是低电平;
// LED引脚设置为输出。输出高电平时，点亮LED; 输出低电平时，熄灭LED。
//
//-------------------------------------------------------
//
//
//*****************************************************************************

//常见标准头文件包含
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

//外设驱动库DriverLib 头文件包含
//--如果编译此处出错,请查看工程的设置, 看下是否把SDK路径包含进来
//--Properties / Build / ARM Compiler / Include Options
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//uart的基本参数配置
//9600,无校验位,停止位1
//在网页小工具中, 选择eUSCI, Clock = 12M, 9600, 得出:
//clockPrescalar:78
//firstModReg:   2
//secondModReg:  0
//overSampleing: 1
const eUSCI_UART_Config uartConfig =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK, //时钟源使用SMCLK
    78,                             //BRDIV = 78, 这3个参数可以通过小工具计算
    2,                              //UCxBRF = 2
    0,                              //UCxBRS = 0
    EUSCI_A_UART_NO_PARITY,         //无校验位
    EUSCI_A_UART_LSB_FIRST,         //LSB First
    EUSCI_A_UART_ONE_STOP_BIT,      //1位停止位
    EUSCI_A_UART_MODE,              //UART模式
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  //参考小工具最后一项
};

#define   RX_BUFF_MAX  5

volatile uint8_t rx_buf[RX_BUFF_MAX];
volatile uint8_t rx_flag;

//发送字符串的函数
//输入字符串,字符串结尾为'\0',调用串口发送函数,从第一个字节开始发送
void uart_send_string(unsigned char *pt)
{
    while(*pt!= '\0')
    {
        UART_transmitData(EUSCI_A0_BASE, *pt);
        pt++;
    }
}

void main(void)
{
    volatile uint32_t ui32_delay;

    //关闭看门狗。实验例程, 我们一般不需要看门狗, 直接关闭
    WDT_A_holdTimer();

    //时钟配置, DCO=12MHz
    CS_setDCOFrequency(CS_12MHZ);

    //SMCLK = DCO = 12MHz,
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //引脚P1.0连接LED1,设置引脚P1.0为输出
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //配置P1.2和P1.3工作在UART模式
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    //配置UART
    UART_initModule(EUSCI_A0_BASE, &uartConfig);

    //使能外设UART
    UART_enableModule(EUSCI_A0_BASE);

    //中断相关配置
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);
    Interrupt_enableMaster();

    uart_send_string("Lab2.1 uart experiment!\r\n");

    //UART发送"I am AI!",一次只能发送一个字符.
    //此处可单步调试,PC端依次收到各个字符. 每次都这么调用,太麻烦了,可以写一个字符串发送函数
    UART_transmitData(EUSCI_A0_BASE, 'I');
    UART_transmitData(EUSCI_A0_BASE, ' ');
    UART_transmitData(EUSCI_A0_BASE, 'a');
    UART_transmitData(EUSCI_A0_BASE, 'm');
    UART_transmitData(EUSCI_A0_BASE, ' ');
    UART_transmitData(EUSCI_A0_BASE, 'A');
    UART_transmitData(EUSCI_A0_BASE, 'I');
    UART_transmitData(EUSCI_A0_BASE, '!');

    //直接调用字符串函数
    uart_send_string("I am AI!\r\n");

    while(1)
    {
        //led灯闪烁,表示程序在运行

        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

        //延时
        for(ui32_delay=0;ui32_delay<1000;ui32_delay++)
        {
            //放led的延时循环里边,减少延时对串口数据的影响
            if(1 == rx_flag)
            {
                rx_flag = 0;
                uart_send_string("hello\r\n");

            }
            else if(2 == rx_flag)
            {
                rx_flag = 0;
                uart_send_string("04045073\r\n");

            }
            else if(3 == rx_flag)
            {
                rx_flag = 0;
                uart_send_string("byebye\r\n");

            }
        }
    }
}

//中断处理函数EUSCI A0 UART ISR
void EUSCIA0_IRQHandler(void)
{
    uint32_t status;

    //读取中断标志
    status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);
    //清除中断标志
    UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    //根据前边读取的中断标志, 判断中断类型, 此处是 接收中断
    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        //一般定义个数组,只是改变下标的方式.
        //本次例程比较简单,最长字符为5个字符. 手动构造一个队列,先进先出
        //0-1-2-3-4 ; 每次接收时都往左移动一个.
        rx_buf[0]=rx_buf[1];
        rx_buf[1]=rx_buf[2];
        rx_buf[2]=rx_buf[3];
        rx_buf[3]=rx_buf[4];
        rx_buf[4]=UART_receiveData(EUSCI_A0_BASE);

        //对接收到的数据进行判断
        //hello, who?, what?
        if((rx_buf[0] =='h')&&
                (rx_buf[1] == 'e') &&
                (rx_buf[2] == 'l') &&
                (rx_buf[3] == 'l') &&
                (rx_buf[4] == 'o'))
        {
            rx_flag = 1;
        }
        else if((rx_buf[1] == 'w') &&
                (rx_buf[2] == 'h') &&
                (rx_buf[3] == 'o') &&
                (rx_buf[4] == '?'))
        {
            //要注意下下标哦,判断后边的4个
            rx_flag = 2;
        }
        else if((rx_buf[0] == 'w') &&
                (rx_buf[1] == 'h') &&
                (rx_buf[2] == 'a') &&
                (rx_buf[3] == 't') &&
                (rx_buf[4] == '?'))
        {
            rx_flag = 3;
        }
    }

}

