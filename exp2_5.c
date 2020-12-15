//*****************************************************************************
// 基础练习2.5: 用PWM控制LED的亮度
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


#define KEY1_VALUE  0x01
#define KEY2_VALUE  0x02
#define NO_KEY      0x00

//定义变量
volatile uint8_t key_value;

//用来临时存放按键的键值
volatile uint8_t pre_key1, now_key1;
volatile uint8_t pre_key2, now_key2;


//pwm输出
Timer_A_PWMConfig pwmConfig =
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    32000,
    TIMER_A_CAPTURECOMPARE_REGISTER_0,
    TIMER_A_OUTPUTMODE_RESET_SET,
    3200
};


void main(void)
{
    volatile uint32_t ui32_delay;
    volatile uint32_t ui32_delay_config;
    volatile uint8_t index;

    //关闭看门狗。实验例程, 我们一般不需要看门狗, 直接关闭
    WDT_A_holdTimer();

    //-------系统时钟设置---------
    //时钟配置, DCO=16MHz
    CS_setDCOFrequency(CS_16MHZ);
    //MCLK = DCO/1 = 16MHz
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //SMCLK = DCO/16 = 1MHz
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_16);

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
    //p1.4
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);

    //配置P2.0为PWM输出
    //引脚的配置 可借助工具 https://dev.ti.com/sysconfig
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring Timer_A to have a period of approximately 500ms and
     * an initial duty cycle of 10% of that (3200 ticks)  */
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    //-------定时器Timer32设置---------
    //配置Timer32在周期模式. 时钟源为MCLK,16分频,1MHz
    Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_16, TIMER32_32BIT,TIMER32_PERIODIC_MODE);

    //1MHz, 1->1us, 1000->1ms, 20 000 -->20ms
    Timer32_setCount(TIMER32_BASE,20000);
    //使能定时器中断
    Timer32_enableInterrupt(TIMER32_BASE);

    Interrupt_enableInterrupt(INT_T32_INT1);

    //开总中断
    Interrupt_enableMaster();

    //启用定时器
    Timer32_startTimer(TIMER32_BASE, false);

    pre_key1 = 1;
    now_key1 = 1;
    pre_key2 = 1;
    now_key2 = 1;

    //uart相关的代码自行添加

    while(1)
    {
        //有按键动作,需要处理
        if(key_value == KEY1_VALUE)
        {
            //按键S1
            key_value = NO_KEY;

            if(pwmConfig.dutyCycle == 28800)
                pwmConfig.dutyCycle = 3200;
            else
                pwmConfig.dutyCycle += 3200;

            Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

        }
        else if(key_value == KEY2_VALUE)
        {
            //按键S2
            key_value = NO_KEY;

            if(pwmConfig.dutyCycle == 3200)
                pwmConfig.dutyCycle = 28800;
            else
                pwmConfig.dutyCycle -= 3200;

            Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
        }

    }

}

// Timer32中断处理函数
void T32_INT1_IRQHandler(void)
{
    //清除中断标志位
    Timer32_clearInterruptFlag(TIMER32_BASE);

    //读取按键值1
    now_key1= GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1);
    //按键未按下时未高电平,按下时为低电平.对按键值进行组合判断
    //pre_key, now_key
    //   1       1      没有按键动作
    //   1       0      按键按下  --认为是一次有效按键
    //   0       0      一直按住按键,按键未松开
    //   0       1      按键松开
    if((pre_key1 == 1) && (now_key1 == 0))
    {
        key_value = KEY1_VALUE;
    }
    //前一次按键值为当前值
    pre_key1 = now_key1;

    //读取按键值2
    now_key2= GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4);
    if((pre_key2 == 1) && (now_key2 == 0))
    {
        key_value = KEY2_VALUE;
    }
    //前一次按键值为当前值
    pre_key2 = now_key2;
}

