//*****************************************************************************
//
// MSP432 ADC14例程01 - 内部温度传感器测温
//
// 该例程对内部温度传感器进行采样，并计算出所对应的温度。
// ADC14工作在单通道多次转换模式，采用软件触发的方式。
//
// 注意事项:
// 根据手册，采样时间不能少于5us。
// 在中断里发生的计算，启用FPU的Stacking模式
// 计算温度时，先计算出摄氏度，然后再转换成和华氏度。
//
// 思考：
// 请将温度转换的结果实时显示出来~ 通过液晶屏或者uart都可以.
//
// -------MSP-EXP432P401R硬件描述 -----------
//
//           MSP-EXP432P401R
//         +------------------+
//      /|\|                  |
//       | |                  |
//       --|RST         P5.5  |
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

uint32_t cal30;
uint32_t cal85;
float calDifference;
float tempC;
float tempF;

int main(void)
{
    //停用看门狗
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();

    //使能FPU,启动stacking(在中断中使用)
    FPU_enableModule();
    FPU_enableLazyStacking();

    //ADC初始化 (MCLK/1/1),启用内部通道（温度传感器）
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_TEMPSENSEMAP);

    //配置单通道模式,多次转换
    ADC14_configureSingleSampleMode(ADC_MEM0, true);

    //输入通道A22(依数据手册,该通道为内部温度传感器)-->ADC_MEM0, 采用内部的参考电压,非差分输入
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A22, ADC_NONDIFFERENTIAL_INPUTS);

    //配置采样时间为192个ADC CLK，根据手册，采样时间必须大于5us
    ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_192,ADC_PULSE_WIDTH_192);

    //设置采样定时器为自动模式
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    //使能通道0的中断
    ADC14_enableInterrupt(ADC_INT0);

    //设置参考电压为2.5, 使能温度传感器
    REF_A_enableTempSensor();
    REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    REF_A_enableReferenceVoltage();

    //读取芯片上存储的温度校准值
    cal30 = SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,SYSCTL_30_DEGREES_C);
    cal85 = SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,SYSCTL_85_DEGREES_C);
    calDifference = cal85 - cal30;

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
// 此处设置断点,查看变量值;
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status;
    int16_t conRes;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT0)
    {
        conRes = ((ADC14_getResult(ADC_MEM0) - cal30) * 55);
        tempC = (conRes / calDifference) + 30.0f;
        tempF = tempC * 9.0f / 5.0f + 32.0f;
    }

}
