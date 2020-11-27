//*****************************************************************************
//
// MSP432 ADC14例程03 - 单通道多次转换示意
//
// 该例程采用的工作模式是单通道多次转换。
// 每次AD转换的结果都存储在变量中，可以在中断中设置断点查看;
// 设置为手动触发的方式，每次转换结束后在中断中再次用软件手动触发下一次转换；
// 基于3.3v参考电压，对AD通道的输入换算成了对应的电压值。
//
// 思考:
// 将转换结果通过液晶屏或者UART显示出来;
//
// -------MSP-EXP432P401R硬件描述 -----------
//
//           MSP-EXP432P401R
//         +------------------+
//      /|\|                  |
//       | |                  |
//       --|RST         P5.5  |<---- A0 In
//         |                  |
//         |            P5.6  |<---- VREF+
//         |            P5.7  |<---- VREF-
//         |                  |
//
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

//变量定义
static volatile uint16_t curADCResult;
static volatile float normalizedADCRes;

int main(void)
{
    //停用看门狗
    WDT_A_holdTimer();

    curADCResult = 0;

    //设置Flash等待时间
    FlashCtl_setWaitState(FLASH_BANK0, 1);
    FlashCtl_setWaitState(FLASH_BANK1, 1);
    
    //设置DCO为48MHz
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
   // CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_setDCOFrequency(CS_48MHZ);

    //使能FPU,同时启用中断内使用浮点运算
    FPU_enableModule();
    FPU_enableLazyStacking();

    //ADC初始化 (MCLK/1/4)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4,ADC_NOROUTE);

    //配置GPIO用作模拟引脚(5.5 A0)
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,GPIO_TERTIARY_MODULE_FUNCTION);

    //配置ADC存储寄存器
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);

    //设置为手动触发的方式
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    //使能转换，软件触发第一次转换
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();

    //使能通道0的中断
    ADC14_enableInterrupt(ADC_INT0);

    //使能模块的中断，使能全局中断
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    while (1)
    {
        PCM_gotoLPM0();
    }
    
}

//===================================================================
//
// ADC的中断处理程序
// 当转换完成且结果放入ADC_MEM0后,触发中断。
//
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if (ADC_INT0 & status)
    {
        curADCResult = ADC14_getResult(ADC_MEM0);
        normalizedADCRes = (curADCResult * 3.3) / 16384;

        ADC14_toggleConversionTrigger();
    }
}

