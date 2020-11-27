//*****************************************************************************
//
// MSP432 ADC14例程07 - 序列通道单次转换
//
// 该例程采用的工作模式是序列通道单次转换，自动触发的方式。
// 配置ADC_MEM0-ADC_MEM7用于存放输入A0-A7的转换结果。
// 最后一个通道转换完成后，将触发ADC_MEM7的中断，结果存储在数组resultsBuffer中。
// 单次转换模式，序列通道仅发生一次采样（可以在中断中设置断点查看结果）。
//
// -------MSP-EXP432P401R硬件描述 -----------
//
//           MSP-EXP432P401R
//         +------------------+
//      /|\|                  |
//       | |                  |
//       --|RST         P5.5  |<--- A0 (Analog Input)
//         |            P5.4  |<--- A1 (Analog Input)
//         |            P5.3  |<--- A2 (Analog Input)
//         |            P5.2  |<--- A3 (Analog Input)
//         |            P5.1  |<--- A4 (Analog Input)
//         |            P5.0  |<--- A5 (Analog Input)
//         |            P4.7  |<--- A6 (Analog Input)
//         |            P4.6  |<--- A7 (Analog Input)
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


//定义数组,存储转换结果
static uint16_t resultsBuffer[8];

int main(void)
{
    //停用看门狗
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();

    //数组填充
    memset(resultsBuffer, 0x00, 8 * sizeof(uint16_t));

    //参考电压源配置。
    //设置参考电压为2.5V, 使能参考电压
    REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    REF_A_enableReferenceVoltage();

    //ADC初始化 (MCLK/1/1)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1,ADC_NOROUTE);

    //将相应的引脚配置成模拟输入引脚
    //P5.4/P5.3/P5.2/P5.1/P5.0
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN5 | GPIO_PIN4 | GPIO_PIN3 | GPIO_PIN2 | GPIO_PIN1| GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
    //P4.7, P4.6
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,GPIO_PIN7 | GPIO_PIN6, GPIO_TERTIARY_MODULE_FUNCTION);


    //配置序列通道模式，单次转换
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM7, true);
    //配置ADC存储寄存器ADC_MEM0-ADC_MEM7 (A0 - A7), 使用内部基准源2.5v, 非差分输入
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A1, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM2, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A2, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM3, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A3, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM4, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A4, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM5, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A5, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM6, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A6, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM7, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A7, ADC_NONDIFFERENTIAL_INPUTS);

    //使能通道7的中断 (序列通道的最后1个)
    ADC14_enableInterrupt(ADC_INT7);

    //使能模块的中断，使能全局中断
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    //设置采样定时器为自动模式
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

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
// 当转换完成且结果放入ADC_MEM7后,触发中断。
// 指示转换结束, 将结果数据存入resultsBuffer
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT7)
    {
        ADC14_getMultiSequenceResult(resultsBuffer);
    }

}


