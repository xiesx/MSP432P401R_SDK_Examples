//*****************************************************************************
// 基础练习1.4: 按键控制LED灯闪烁
//
// 怎么通过按键改变闪烁的频率？
// 定义一个数组,存放不同的延时系数(可实现调好,几个明显不同的速度)
// 用户按键,改变数组的下标,进而改变LED的延时时间，改变闪烁的快和慢.
//
// 按键的实现方法:
// 读取按键的方法有多种. 本例程使用的是方法一.
// 方法一:
//   采用轮询的方式,在程序的主循环中不断的去读取引脚的数值,如果低电平则说明按键按下.
//   按键消抖,可以软件延时20ms之后再去读取按键的值;
// 方法二:
//   采用定时扫描的方式. 设置20ms的定时器,每隔20ms读取一次引脚的输入值.判断数值是否发生变化.
//   按键未按下时,为高电平;按下时为低电平.也就是说,前一次为高电平,这一次为低电平则可认定为1次有效按键.
// 方法三:
//   采用中断的方式.
//
// 一次按键动作可能的波形如下:
// ----<按键按下--------------------><按键松开------
// ----+ +-+ +-+                  +-+ +-+ +----------------
//     | | | | |                  | | | | |
//     +-+ +-+ +------------------+ +-+ +-+
//
// 可能出现的错误:
// (1)要考虑到按键的消抖问题,不然1次按键动作可能识别为多次按键。
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

//对于LED的延时时间,可以通过实验,获取8个不同亮度的延时系数.
//注意,为了降低难度，数组的下标0对应的数字暂时不用,下标index从1-8.
uint32_t const delay_tab[]={0,5000,10000,16000,24000,40000,60000,80000,100000};

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


    index = 5;
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
                if(0 == index) index = 8;
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
                if(9 == index) index = 1;

                ui32_delay_config =delay_tab[index];

                //等待按键释放
                while(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4));
            }
        }

        //-----实现LED的闪烁效果
        //输出低电平,LED灭
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        for(ui32_delay=0;ui32_delay<ui32_delay_config;ui32_delay++);

        //输出高电平,LED亮
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        //此处要加延时,延时越长,表示亮的时间越久
        for(ui32_delay=0;ui32_delay<ui32_delay_config;ui32_delay++);
    }

}
