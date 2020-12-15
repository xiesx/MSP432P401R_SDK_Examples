//*****************************************************************************
// 基础练习2.3: 通过定时器实现LED闪烁
//
// MSP432有多种定时器,例如Timer32, TimerA, WDT, SystemTick.
// 在该例程中,我们选用Timer32定时器.
// 可以在SDK中找个例程timer32_periodic_mode_led_toggle,在此基础上修改.
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



void main(void)
{

    //关闭看门狗。实验例程, 我们一般不需要看门狗, 直接关闭
    WDT_A_holdTimer();

    //时钟配置, DCO=1MHz
    CS_setDCOFrequency(CS_1MHZ);

    //MCLK = DCO/1 = 1MHz,
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //引脚P1.0连接LED1,设置引脚P1.0为输出
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //led的初始状态为熄灭
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);


    //配置Timer32在周期模式. 时钟源为MCLK,不分频,1MHz
    Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,TIMER32_PERIODIC_MODE);

    //1MHz, 1->1us, 1000->1ms, 500 000 -->500ms=0.5s
    Timer32_setCount(TIMER32_BASE,500000);
    //使能定时器中断
    Timer32_enableInterrupt(TIMER32_BASE);

    Interrupt_enableInterrupt(INT_T32_INT1);

    //开总中断
    Interrupt_enableMaster();

    //启用定时器
    Timer32_startTimer(TIMER32_BASE, false);

    while (1)
    {
        
    }

}

