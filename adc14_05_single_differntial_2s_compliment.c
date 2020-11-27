//*****************************************************************************
//
// MSP432 ADC14例程05 - 单通道多次转换，差分模式
//
// 该例程ADC14的工作模式是单通道多次转换，使用软件自动触发。
// 对外部通道A0和A1进行采样，差分模式，直接使用AVCC(3.3v)作为参考电压源;
// 对于转换结果，格式为补码形式。
// 在中断中将值转换为浮点型，以供用户在调试器中观察。
//
// -------MSP-EXP432P401R硬件描述 -----------
//
//           MSP-EXP432P401R
//         +------------------+
//      /|\|                  |
//       | |                  |
//       --|RST         P5.5  |<--- A0 (模拟输入)
//         |            P5.4  |<--- A1 (模拟输入)
//         |                  |
//
//-------------------------------------------------------
//
//
//*****************************************************************************

//常见标准头文件包含
#include <stdint.h>
#include <string.h>

//外设驱动库DriverLib 头文件包含
//--如果编译此处出错,请查看工程的设置, 看下是否把SDK路径包含进来
//--Properties / Build / ARM Compiler / Include Options
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>


//定义变量
static volatile float adcResult;
static float convertToFloat(uint16_t result);

int main(void)
{
    //停用看门狗
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();

    //使能FPU,启动stacking(在中断中使用)
    FPU_enableModule();
    FPU_enableLazyStacking();

    //ADC初始化 (MCLK/1/1)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    //单通道模式，多次转换
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    //参考电压使用AVCC, 差分模式, ADC_MEM0 --> A0/A1
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, ADC_DIFFERENTIAL_INPUTS);

    //配置P5.4,P5.5用作模拟输入引脚
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5 | GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

    //修改ADC转换结果的数据格式
    ADC14_setResultFormat(ADC_SIGNED_BINARY);

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
// 每次转换完成时,都会触发中断。
// 因为使能了FPU的stacking模式,我们可以在中断里边安全地执行高效的浮点运算
//
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT0)
    {
        adcResult = convertToFloat(ADC14_getResult(ADC_MEM0));
    }

}

//===================================================================
//
// 基于3.3V的参考电压,将ADC的14-bit转换结果 变成浮点数
//
//===================================================================
static float convertToFloat(uint16_t result)
{
    int32_t temp;

        if(0x8000 & result)
        {
            temp = (result >> 2) | 0xFFFFC000;
            return ((temp * 3.3f) / 8191);
        }
        else
            return ((result >> 2)*3.3f) / 8191;
}
