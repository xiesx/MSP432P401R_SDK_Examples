//*****************************************************************************
// 基础练习2.6: 全彩色LED灯设计
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

//对于LED的延时时间,可以通过实验,获取5个不同亮度等级.
//注意,为了降低难度，数组的下标0对应的数字暂时不用,下标index从1-5.
uint32_t const delay_tab[]={0,100000,5000,2000,200,20};

void main(void)
{
    volatile uint32_t ui32_delay;
    volatile uint32_t ui32_delay_config;
    volatile uint8_t index;

    //关闭看门狗。实验例程, 我们一般不需要看门狗, 直接关闭
    WDT_A_holdTimer();

    //引脚P1.0连接LED1,设置引脚P1.0为输出
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //led的初始状态为熄灭
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    //按键引脚设置为输入,而且要使能内部上拉电阻
    //p1.1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    //p1.4
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);


    index = 3;
    ui32_delay_config =delay_tab[index];

    while(1)
    {
        //-----识别用户的按键动作
        //判断按键p1.1
        if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1))
        {
            //增加延时用于按键消抖,再次读取判断
            for(ui32_delay=0;ui32_delay<1000;ui32_delay++);

            if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1))
            {
                //通过改变数组下标,改变延时系数
                //每次按键按下,数组下标左移一位,时间变短,闪烁变快
                index --;
                if(0 == index) index = 5;
                ui32_delay_config =delay_tab[index];
                //等待按键释放
                //思考:会造成软件阻塞,按键按住不松开,会有什么现象?
                while(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1));
            }
        }


        //判断按键p1.4
        if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4))
        {
            //增加延时用于按键消抖,再次读取判断
            for(ui32_delay=0;ui32_delay<1000;ui32_delay++);

            if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4))
            {
                //每次按键按下,数组下标右移一位,时间变长,闪烁变慢
                index ++;
                if(6 == index) index = 1;

                ui32_delay_config =delay_tab[index];

                //等待按键释放
                while(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4));
            }
        }

        //-----实现LED的亮度改变

        //输出高电平,LED亮
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        //此处要加延时,延时越长,表示亮的时间越久
        for(ui32_delay=0;ui32_delay<ui32_delay_config;ui32_delay++);
        
        //输出低电平,LED灭
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        for(ui32_delay=0;ui32_delay<2000;ui32_delay++);

    }

}
