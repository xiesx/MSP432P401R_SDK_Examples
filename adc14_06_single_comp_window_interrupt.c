//*****************************************************************************
//
// MSP432 ADC14例程06 - 单通道多次转换,启用窗口比较器
//
// 该例程采用的工作模式是单通道多次转换，启用窗口比较器的功能。
// 使用内部2.5V的参考电压,对通道A0进行采样;
// 窗口比较器阈值的上限和下限都设置为1V,也就是说值超过或者低于1v就会触发中断;
// 用LED作为指示灯，当输入值大于1V时点亮LED，小于1V时, 熄灭LED;
//
// 思考：
// 这个用在什么场合呢？
// 对代码做修改，阈值的上下限设置为不同数值。
//
// -------MSP-EXP432P401R硬件描述 -----------
//
//           MSP-EXP432P401R
//         +------------------+
//      /|\|                  |
//       | |                  |
//       --|RST         P5.5  |<--- A0 (Analog Input)
//         |                  |
//         |                  |
//         |            P1.0  |---> P1.0 LED
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


int main(void)
{
    //停用看门狗
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();

    //设置Flash的等待时间
    FlashCtl_setWaitState(FLASH_BANK0, 1);
    FlashCtl_setWaitState(FLASH_BANK1, 1);
    
    //设置DCO为48MHz
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
   // CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_setDCOFrequency(CS_48MHZ);

    //设置内部参考源为2.5V, 启用内部参考源
    REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    REF_A_enableReferenceVoltage();

    //ADC初始化 (MCLK/1/4)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4,ADC_NOROUTE);

    //配置P1.0为GPIO输出引脚
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    //配置P5.5用作模式输入引脚A0
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);

    //单通道模式，多次转换
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    //使用内部的参考电压源，非差分输入
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS,ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);
    //设置采样保持/保持时间
    ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_128, ADC_PULSE_WIDTH_128);

    //设置窗口比较器阈值的上限和下限；
    //2^14=16384--2.5V, 9830--1.5V , 6553--1.0V
//    ADC14_setComparatorWindowValue(ADC_COMP_WINDOW0, 6553, 9830);
    ADC14_setComparatorWindowValue(ADC_COMP_WINDOW0, 6553, 6553);
    //启用MEM0的窗口比较器功能
    ADC14_enableComparatorWindow(ADC_MEM0, ADC_COMP_WINDOW0);
    //先清除中断标志位，开启中断
    ADC14_clearInterruptFlag(ADC_HI_INT | ADC_LO_INT);
    ADC14_enableInterrupt(ADC_HI_INT | ADC_LO_INT);

    //设置采样定时器为自动模式
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    //使能转换，软件触发第一次转换
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();

    //使能模块的中断，使能全局中断
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    //计入低功耗模式
    while (1)
    {
        PCM_gotoLPM0();
    }
}

//===================================================================
//
// ADC窗口比较器的中断处理.
// 如果进入该中断,说明输入端A0的电压要么大于1.0V,要么小于1.0V。
//
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(ADC_HI_INT | ADC_LO_INT);

    if(status & ADC_LO_INT)
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        ADC14_enableInterrupt(ADC_HI_INT);
        ADC14_disableInterrupt(ADC_LO_INT);
    }

    if(status & ADC_HI_INT)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        ADC14_enableInterrupt(ADC_LO_INT);
        ADC14_disableInterrupt(ADC_HI_INT);
    }

}

