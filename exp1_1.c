//*****************************************************************************
// 基础练习1.1: 软件延时实现LED闪烁
//
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

//外设驱动库DriverLib 头文件包含
//--如果编译此处出错,请查看工程的设置, 看下是否把SDK路径包含进来
//--Properties / Build / ARM Compiler / Include Options
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//有两种方法,通过编译开关选择. 可以输入1或者2
#define EXP_CONFIG  1

void main(void)
{
    volatile uint32_t ui32_delay;

    //关闭看门狗。实验例程, 我们一般不需要看门狗, 直接关闭
    WDT_A_holdTimer();

    //引脚P1.0连接LED1
    //设置引脚P1.0为输出
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

#if(EXP_CONFIG==1)
    //方法一:
    //引脚P1.0交替输出高电平和低电平
    //    +-----+      +------+
    //    |     |      |      |
    //----+     +------+      +---------
    //可能出现的错误:
    //(1)没有加while(1),程序语句按顺序执行1遍就结束了.
    //(2)不加延时,全速运行,常亮.要修改延时时间
    //(3)要在两个地方加软件延时,点亮LED后延时,熄灭LED后也要加延时;
    //
    while(1)
    {
        //输出低电平,LED灭
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        for(ui32_delay=0;ui32_delay<300000;ui32_delay++);

        //输出高电平,LED亮
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        //此处要加延时,延时越长,表示亮的时间越久
        for(ui32_delay=0;ui32_delay<300000;ui32_delay++);
    }

#elif(EXP_CONFIG==2)
    //方法二
    //使用取反函数,每次对引脚的输出值取反. 异或 ^1
    while (1)
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

        //延时
        for(ui32_delay=0;ui32_delay<10000;ui32_delay++)
        {
        }
        
    }
#endif


}
