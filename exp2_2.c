//*****************************************************************************
// 基础练习2.2: 通过UART控制LED的亮和灭
//
// 涉及到控制的,可以先定义双方通信的协议.
// 假设协议为4个字节: AA BB CC DD
// 第1个字节为 AA表示 帧头, 固定为0x5A
// 第2个字节为 BB表示 命令:
//         0x00查询LED状态; 0x01打开LED; 0x02,关闭LED
// 第3个字节为 CC表示 数据,表示LED的序号:
//         0x01,第1个;0x02,第2个;0x03,第3个, 0x04,第4个,0xFF,所有的LED。
// 第4个字节为 DD表示 帧尾, 固定为0xA5
// 所以,如果上位机发送 5A 01 01 A5 -- 打开LED1
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

#define  RX_BUFF_MAX  4

//用来接收临时存放的数据
volatile uint8_t rx_buf[RX_BUFF_MAX];
//如果串口接收到一帧数据,该标志位置1,在主函数里处理
volatile uint8_t rx_flag;
//用来存放串口接收到的数据
volatile uint8_t app_cmd, app_data;

//用来存放串口接收到的数据
volatile uint8_t led1,led2,led3,led4;

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

    //SMCLK = DCO/1 = 12MHz,
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //引脚P1.0连接LED1,设置引脚P1.0为输出
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    //LED2,设置引脚P2.0/P2.1/P2.2为输出
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 +GPIO_PIN1 +GPIO_PIN2);

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

    uart_send_string("Lab2.2 uart control led!\r\n");
    uart_send_string("--------------------------\r\n");
    uart_send_string("USE: 0x5A byte2 byte3 0xA5\r\n");
    uart_send_string("byte2 is cmd:\r\n");
    uart_send_string("----0x00:query led status\r\n");
    uart_send_string("----0x01:turn on led     \r\n");
    uart_send_string("----0x02-turn off led    \r\n");
    uart_send_string("byte3 is data, led number:\r\n");
    uart_send_string("----0x01:led1      \r\n");
    uart_send_string("----0x02:led2 red  \r\n");
    uart_send_string("----0x03:led2 green\r\n");
    uart_send_string("----0x04:led2 blue \r\n");
    uart_send_string("----0xff:all leds  \r\n");
    uart_send_string("--------------------------\r\n");

    //led的初始状态都为熄灭
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    led1=0;
    led2=0;
    led3=0;
    led4=0;

    rx_flag =0;

    while(1)
    {
        for( ui32_delay=0; ui32_delay <200; ui32_delay++);

        if(rx_flag == 1)
        {
            //串口接收到一帧数据,需要处理
            rx_flag = 0;
            uart_send_string("uart get user data!\r\n");
            if(app_cmd == 0x00)
            {
                uart_send_string("cmd: query led status!\r\n");

                switch(app_data)
                {
                    case 0x01:
                        if(led1 == 1) uart_send_string("led1 is on!\r\n");
                        else uart_send_string("led1 is off!\r\n");
                        break;
                    case 0x02:
                        if(led2 == 1)uart_send_string("led2 red is on!\r\n");
                        else uart_send_string("led2 red is off!\r\n");
                        break;
                    case 0x03:
                        if(led3 == 1) uart_send_string("led2 green is on!\r\n");
                        else uart_send_string("led2 green is off!\r\n");
                        break;
                    case 0x04:
                        if(led4 == 1) uart_send_string("led2 blue is on!\r\n");
                        else uart_send_string("led2 blue is off!\r\n");
                        break;
                    case 0xFF:
                        //查询所有的LED状态
                        if(led1 == 1) uart_send_string("led1 is on!\r\n");
                        else uart_send_string("led1 is off!\r\n");
                        //led2-1
                        if(led2 == 1)uart_send_string("led2 red is on!\r\n");
                        else uart_send_string("led2 red is off!\r\n");
                        //led2-2
                        if(led3 == 1) uart_send_string("led2 green is on!\r\n");
                        else uart_send_string("led2 green is off!\r\n");
                        //led2-3
                        if(led4 == 1) uart_send_string("led2 blue is on!\r\n");
                        else uart_send_string("led2 blue is off!\r\n");
                        break;
                    default:
                        uart_send_string("unknown data!\r\n");
                        break;
                }


            }
            else if(app_cmd == 0x01)
            {
                uart_send_string("cmd: turn on led!\r\n");
                switch(app_data)
                {
                    case 0x01:
                        uart_send_string("turn on led1!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        led1=1;
                        break;
                    case 0x02:
                        uart_send_string("turn on led2 red!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                        led2=1;
                        break;
                    case 0x03:
                        uart_send_string("turn on led2 green!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
                        led3=1;
                        break;
                    case 0x04:
                        uart_send_string("turn on led2 blue!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
                        led4=1;
                        break;
                    case 0xFF:
                        uart_send_string("turn on all the leds!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        led1=0;
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                        led2=0;
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
                        led3=0;
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
                        led4=0;
                        break;
                    default:
                        uart_send_string("unknown data!\r\n");
                        break;
                }

            }
            else if(app_cmd == 0x02)
            {
                switch(app_data)
                {
                    case 0x01:
                        uart_send_string("turn off led1!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        led1=0;
                        break;
                    case 0x02:
                        uart_send_string("turn off led2 red!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                        led2=0;
                        break;
                    case 0x03:
                        uart_send_string("turn off led2 green!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
                        led3=0;
                        break;
                    case 0x04:
                        uart_send_string("turn off led2 blue!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
                        led4=0;
                        break;
                    case 0xFF:
                        uart_send_string("turn off all the leds!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        led1=0;
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                        led2=0;
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
                        led3=0;
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
                        led4=0;
                        break;
                    default:
                        uart_send_string("unknown data!\r\n");
                        break;
                }
            }
            else
            {
                //未定义的命令
                uart_send_string("unknown command!!!\r\n");
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
        //协议数据较短,直接参考前边的例程
        //手动构造一个队列,先进先出
        //0-1-2-3; 每次接收时都往左移动一个.
        rx_buf[0]=rx_buf[1];
        rx_buf[1]=rx_buf[2];
        rx_buf[2]=rx_buf[3];
        rx_buf[3]=UART_receiveData(EUSCI_A0_BASE);

        //对接收到的数据进行判断,判断帧头和帧尾,如果满足要求,则认为接收到有效数据
        //实际应用中,一般会加校验位
        if((rx_buf[0] == 0x5A) && (rx_buf[3] == 0xA5))
        {
            //中断里边只设置标志位,剩下的到主函数里边处理
            if(rx_flag == 0)
            {
                app_cmd = rx_buf[1];
                app_data =rx_buf[2];
                //最后设置数据接收有效标志位为1
                rx_flag = 1;
            }
            else
            {
                //数据溢出,说明前一次的数据还未处理,可添加错误处理
            }
        }
    }

}


