//*****************************************************************************
//
// MSP432 ADC14例程02 - 单通道单次转换示意（使用外部基准源）
//
// 例程演示ADC模块使用外部基准源。
// 对其中一个输入通道A0 (P5.5)进行采样/转换，转换结果存储到本地变量adcResult;
// 引脚VREF+ (P5.6)上的电压值为最大值,引脚VREF- (P5.7)上的电压值为最小值;
//
// -------MSP-EXP432P401R硬件描述 -----------
//
//           MSP-EXP432P401R
//         +------------------+
//      /|\|                  |
//       | |                  |
//       --|RST         P5.5  |<---- A0 In
//         |            P5.6  |<---- VREF+
//         |            P5.7  |<---- VREF-
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


volatile uint16_t adcResult;

int main(void)
{
    //停用看门狗
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();
    
    //ADC初始化 (MCLK/1/1),不启用内部通道
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    //配置为单通道模式， 多次转换
    ADC14_configureSingleSampleMode(ADC_MEM0, true);

    //配置ADC存储寄存器(A0->ADC_MEM0 ), 使用外部基准, 非差分输入
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_EXTPOS_VREFNEG_EXTNEG, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);

    //配置GPIOs用作模拟端口引脚 (AD输入和基准引脚)
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN7 | GPIO_PIN6 | GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);

    //设置采样定时器为自动模式
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    //使能通道0的中断
    ADC14_enableInterrupt(ADC_INT0);

    //使能模块的中断，使能全局中断
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    //使能转换，软件触发第一次转换
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();

    //进入睡眠模式
    while (1)
    {
        PCM_gotoLPM0();
    }
}

//===================================================================
//
// 当转换完成且结果放入ADC_MEM0后,触发中断。
//
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT0)
    {
        adcResult = ADC14_getResult(ADC_MEM0);
    }

}
