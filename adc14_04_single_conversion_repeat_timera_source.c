//*****************************************************************************
//
// MSP432 ADC14例程04 - 单通道多次转换，使用TimerA作为触发
//
// 该例程ADC14的工作模式是单通道多次转换，使用TimerA CCR1的输出作为触发。
// 对外部通道A0进行采样，直接使用AVCC(3.3v)作为参考电压源;
// 设置Timer_A为增计数模式,启用比较器CCR0,计数至16384(0.5秒),会翻转比较器的输出;
// 即Timer_A启动后，将每间隔0.5s触发ADC14开始转换。
// 数组resultsBuffer用于存放ADC的转换结果(数组循环使用,resPos溢出至0的循环缓冲区)
//
// -------MSP-EXP432P401R硬件描述 -----------
//
//           MSP-EXP432P401R
//         +------------------+
//      /|\|                  |
//       | |                  |
//       --|RST         P5.5  |<--- A0 (Analog Input)
//         |                  |
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

//定时器Timer_A增计数模式的参数配置
const Timer_A_UpModeConfig upModeConfig =
{
    TIMER_A_CLOCKSOURCE_ACLK,            // 选择时钟源为ACLK
    TIMER_A_CLOCKSOURCE_DIVIDER_1,       // 不分频, ACLK/1 = 32Khz
    16384,                               // 16384/32768 = 0.5s
    TIMER_A_TAIE_INTERRUPT_DISABLE,      // 禁用Timer ISR
    TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE, // 禁用CCR0
    TIMER_A_DO_CLEAR                     // Clear Counter
};

//Timer_A比较器参数设置
const Timer_A_CompareModeConfig compareConfig =
{
    TIMER_A_CAPTURECOMPARE_REGISTER_1,          // 使用CCR1
    TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // 禁用CCR中断
    TIMER_A_OUTPUTMODE_SET_RESET,               // 取反输出
    16384                                       // 16384/32768 = 0.5s
};

//变量定义
//宏定义UINT8_MAX在<stdint.h>中,值是0xFF
static volatile uint_fast16_t resultsBuffer[UINT8_MAX];
static volatile uint8_t resPos;

int main(void)
{
    //停用看门狗
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();
    resPos = 0;

    //系统时钟设置，使用默认时钟;
    //默认时钟: MCLK = DCO = ~3MHz, ACLK = REFO = 32Khz
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //ADC初始化 (MCLK/1/1)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    //配置GPIO用作功能引脚(P5.5-->A0)
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);

    //配置为单通道模式， 多次转换
    ADC14_configureSingleSampleMode(ADC_MEM0, true);

    //配置ADC存储寄存器(A0->ADC_MEM0 ), 使用外部基准, 非差分输入
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);

    //根据参数设定,配置定时器Timer_A0
    Timer_A_configureUpMode(TIMER_A0_BASE, &upModeConfig);

    //根据参数设定,配置Timer_A0的CCR1
    Timer_A_initCompare(TIMER_A0_BASE, &compareConfig);

    //配置触发源来自TA0_C1(数据手册)
    ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

    //使能通道0的中断
    ADC14_enableInterrupt(ADC_INT0);
    ADC14_enableConversion();

    //使能模块的中断，使能全局中断
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    //启用定时器,用于触发AD转换
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    //进入睡眠模式
    while (1)
    {
        PCM_gotoLPM0();
    }
}

//===================================================================
//
// 当转换完成且结果放入ADC_MEM0后,触发中断。
// 将结果存放到数据中;
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);


    if (status & ADC_INT0)
    {
        if(resPos == UINT8_MAX)
        {
           resPos = 0; 
        }
        
        resultsBuffer[resPos++] = ADC14_getResult(ADC_MEM0);
    }

}

