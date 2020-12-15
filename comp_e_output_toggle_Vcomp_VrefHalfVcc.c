/*******************************************************************************
 * MSP432 Comparator - Comparator Toggle from SLEEP; input channel C6;
 *      Vcompare is compared against Vcc/2
 *
 * Description: Use comparator and shared reference to determine if input 'Vcompare'
 * is high or low.  When Vcompare exceeds Vcc*1/2 COUT goes low and when
 * Vcompare is less than Vcc*1/2 then CEOUT goes high.
 *
 *
 *                 MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST        P5.7/C6|<--Vcompare
 *            |                  |
 *            |         P7.2/COUT|----> 'low'(Vcompare>Vcc*1/2); 'high'(Vcompare<Vcc*1/2)
 *            |                  |  
 *            |                  |  
 *            |                  |
 ******************************************************************************/
//*****************************************************************************
//
// MSP432比较器例程01 - 简单电压比较
//
// 该例程将引脚上的输入电压与内部1.2V参考电压进行比较；
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
//       --|RST     P5.7/C1.6 |<--Vcompare
//         |                  |
//         |         P7.2/COUT|----> 'low'(Vcompare>Vcc*1/2); 'high'(Vcompare<Vcc*1/2)
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

/* Comparator configuration structure */
const COMP_E_Config compConfig =
{
        COMP_E_VREF,                  // Positive Input Terminal
        COMP_E_INPUT6,                // Negative Input Terminal
        COMP_E_FILTEROUTPUT_DLYLVL4,  // Level 4 Filter
        COMP_E_NORMALOUTPUTPOLARITY   // Normal Output Polarity
};

int main(void)
{
    volatile uint32_t ii;

    //停用看门狗
    MAP_WDT_A_holdTimer();

    /*
     * Select Port 7
     * Set Pin 2 to output Primary Module Function, (COUT).
     */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN2,
            GPIO_PRIMARY_MODULE_FUNCTION);
            
    /* Set P5.7 to be comparator in (C1.6) */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN7,
            GPIO_TERTIARY_MODULE_FUNCTION);

    /* Initialize the Comparator E module
     *  Comparator Instance 0,
     *  Pin CE6 to Positive(+) Terminal,
     *  Reference Voltage to Negative(-) Terminal,
     *  Normal Power Mode,
     *  Output Filter On with max delay,
     *  Non-Inverted Output Polarity
     */
    MAP_COMP_E_initModule(COMP_E1_BASE, &compConfig);

    /*Set the reference voltage that is being supplied to the (-) terminal
     *  Comparator Instance 0,
     *  Reference Voltage of Vcc V,
     *  Lower Limit of Vcc*(16/32) = Vcc*1/2, (aprox. 1.65v at Vcc=3.3v)
     *  Upper Limit of Vcc*(16/32) = Vcc*1/2
     */
    MAP_COMP_E_setReferenceVoltage(COMP_E1_BASE,
                                   COMP_E_REFERENCE_AMPLIFIER_DISABLED, 16, 16);

    /* Disable Input Buffer on CE1.6
     *  Base Address of Comparator E,
     *  Input Buffer port
     *  Selecting the CEx input pin to the comparator
     *  multiplexer with the CEx bits automatically
     *  disables output driver and input buffer for
     *  that pin, regardless of the state of the
     *  associated CEPD.x bit
     */
    MAP_COMP_E_disableInputBuffer(COMP_E1_BASE, COMP_E_INPUT6);

    /* Allow power to Comparator module */
    MAP_COMP_E_enableModule(COMP_E1_BASE);

    /* Delaying to allow comparator to settle */
    for (ii = 0; ii < 400; ii++)
        ;

    while (1)
    {
        MAP_PCM_gotoLPM0();
    }
}
