//*****************************************************************************
//-------------------------------------------------------
// MSP432 PWM练习: 输出1路PWM波形
//-------------------------------------------------------
// PWM的参数：周期为100Hz,占空比为10%;
// TimerA工作在增计数模式下,输出单元配置为工作模式6；
//
//
//*****************************************************************************
//MSP432驱动库的头文件
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//标准头文件
#include <stdint.h>
#include <stdbool.h>

//宏定义,方便用户修改. 16位定时器
#define TIMER_PERIOD    10000
#define DUTY_CYCLE  1000

//Timer_A增计数工作模式的参数配置
const Timer_A_UpModeConfig upConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // 选择时钟信号SMCLK
        TIMER_A_CLOCKSOURCE_DIVIDER_3,      // SMCLK/3 =1MHz
        TIMER_PERIOD,                           // 周期
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // 禁止Timer溢出中断
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE, // 禁用CCR0中断
        TIMER_A_DO_CLEAR                        // Clear value
};

//Timer_A比较单元的参数配置(PWM)
const Timer_A_CompareModeConfig compareConfig_PWM =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_1,          // Use CCR1
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // 关闭中断
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // 输出模式6
        DUTY_CYCLE                                 //占空比计数
};

int main(void)
{
    //关闭看门狗
    WDT_A_holdTimer();

    //使用系统默认的振荡器 DCO = ~3MHz, REFOCLK= ~32768
    //对系统的时钟进行配置
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //P7.7/PM_TA1.1,配置引脚P7.7作为比较单元的输出
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

    //配置Timer_A1工作在增计数模式下
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);

    //配置比较单元产生PWM
    Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_PWM);

    //TimerA开始计数
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    //进入低功耗模式
    while (1)
    {
        PCM_gotoLPM0();
    }
}

