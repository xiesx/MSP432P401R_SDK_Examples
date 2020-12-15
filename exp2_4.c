//*****************************************************************************
// 基础练习2.4: 基于定时器的按键检测
//
// MSP432有多种定时器,例如Timer32, TimerA, WDT, SystemTick.
// 在该例程中,我们选用Timer32定时器. 产生一个20ms的定时器
// 每20ms去读取一次引脚值
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

//定义变量
//key_flag,检测到按键松开,认为是一次完整的按键
//key_time,用来记录按键按下的时长
volatile uint8_t key_flag, key_time;

//用来临时存放按键的键值
volatile uint8_t pre_key, now_key;

void main(void)
{
    volatile uint32_t ui32_delay;
    volatile uint32_t ui32_delay_config;
    volatile uint8_t index;

    //关闭看门狗。实验例程, 我们一般不需要看门狗, 直接关闭
    WDT_A_holdTimer();

    //-------系统时钟设置---------
    //时钟配置, DCO=1MHz
    CS_setDCOFrequency(CS_1MHZ);
    //MCLK = DCO/1 = 1MHz,
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //-------GPIO口设置---------
    //引脚P1.0连接LED1,设置引脚P1.0为输出
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    //led的初始状态为熄灭
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    //p2.0
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    //按键引脚设置为输入,而且要使能内部上拉电阻,p1.1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

    //-------定时器Timer32设置---------
    //配置Timer32在周期模式. 时钟源为MCLK,不分频,1MHz
    Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,TIMER32_PERIODIC_MODE);

    //1MHz, 1->1us, 1000->1ms, 20 000 -->20ms
    Timer32_setCount(TIMER32_BASE,20000);
    //使能定时器中断
    Timer32_enableInterrupt(TIMER32_BASE);

    Interrupt_enableInterrupt(INT_T32_INT1);

    //开总中断
    Interrupt_enableMaster();

    //启用定时器
    Timer32_startTimer(TIMER32_BASE, false);

    pre_key = 1;
    now_key = 1;

    //uart相关的代码自行添加

    while(1)
    {
        //有按键动作,需要处理
        if(key_flag==1)
        {
            //可以设置断点,看下自己短按键的时间

            //判断长按键还是短按键
            if(key_time > 12)
            {
                //长按键
                GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
            }
            else
            {
                //短按键
                GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            }

            key_flag =0;
            key_time =0;
        }

    }

}

// Timer32中断处理函数
void T32_INT1_IRQHandler(void)
{
    //清除中断标志位
    Timer32_clearInterruptFlag(TIMER32_BASE);

    //读取按键值
    now_key= GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1);

    //按键未按下时未高电平,按下时为低电平.对按键值进行组合判断
    //pre_key, now_key
    //   1       1      没有按键动作
    //   1       0      按键按下
    //   0       0      一直按住按键,按键未松开 --> key_time ++;
    //   0       1      按键松开  -->设置标志位 key_flag=1
    if((pre_key == 1) && (now_key == 0))
    {
        key_time = 1;
    }
    else if((pre_key == 0) && (now_key == 0))
    {
        key_time ++;
    }
    else if((pre_key == 0) && (now_key == 1))
    {
        //一次按键动作结束
        key_flag=1;
    }

    //前一次按键值为当前值
    pre_key = now_key;
}

