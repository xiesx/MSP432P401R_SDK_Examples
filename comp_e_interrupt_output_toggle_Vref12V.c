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
//         |              P1.0|----> 'high'(Vcompare>1.2V); 'low'(Vcompare<1.2)
//         |                  |  |
//         |                  |  | LED 'ON'(Vcompare>1.2V); 'OFF'(Vcompare<1.2V)
//         |                  |
//
//-------------------------------------------------------
//
//
//*****************************************************************************
/******************************************************************************
 * MSP432 Comparator - Interrupt Capability; Vcompare is compared against
 *                    internal 1.2V reference
 *
 * Description: Use COMPE and internal reference to determine if input'Vcompare'
 * is high or low.  For the first time, when Vcompare exceeds the 1.2V internal
 * reference, CEIFG is set and device enters the COMPE ISR. In the ISR, CEIES is
 * toggled such that when Vcompare is less than 1.2V internal reference;
 * CEIFG is set. The LED is toggled inside the ISR to illustrate the change.
 * The filter feature of the Comp module is used to filter out any noise on
 * the line and assure no spurious interrupts occur.
 *
 ******************************************************************************/
//常见标准头文件包含
#include <stdint.h>
#include <stdbool.h>

//外设驱动库DriverLib 头文件包含
//--如果编译此处出错,请查看工程的设置, 看下是否把SDK路径包含进来
//--Properties / Build / ARM Compiler / Include Options
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//![Simple Comp Config] 
/* Comparator configuration structure */
const COMP_E_Config compConfig =
{
        COMP_E_VREF,                  // Positive Input Terminal
        COMP_E_INPUT6,                // Negative Input Terminal
        COMP_E_FILTEROUTPUT_DLYLVL4,  // Delay Level 4 Filter (max)
        COMP_E_NORMALOUTPUTPOLARITY   // Normal Output Polarity
};


int main(void)
{
    //停用看门狗
    WDT_A_holdTimer();

    /* Set P1.0 as an output pin for LED */
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Set P5.7 to be comparator in (C1.6) */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN7,GPIO_TERTIARY_MODULE_FUNCTION);

    /* Initialize the Comparator module
     *  Comparator Instance 1
     *  Reference Voltage to Postive(+) Terminal
     *  Pin CE16 to negative(-) Terminal
     *  Normal Power Mode
     *  Output Filter On with max delay
     *  Non-Inverted Output Polarity
     */
    COMP_E_initModule(COMP_E1_BASE, &compConfig);

    /*
     * Base Address of Comparator E,
     * Reference Voltage of 1.2 V,
     * Lower Limit of 1.2*(32/32) = 1.2V,
     * Upper Limit of 1.2*(32/32) = 1.2V
     */
    COMP_E_setReferenceVoltage(COMP_E1_BASE, COMP_E_VREFBASE1_2V, 32, 32);

    /* Enable COMP_E Interrupt on default rising edge for CEIFG */
    COMP_E_setInterruptEdgeDirection(COMP_E1_BASE, COMP_E_RISINGEDGE);

    /*  Enable Interrupts
     *  Comparator Instance 1,
     *  Enable COMPE Interrupt on default rising edge for CEIFG
     */
    COMP_E_clearInterruptFlag(COMP_E1_BASE, COMP_E_OUTPUT_INTERRUPT);
    COMP_E_enableInterrupt(COMP_E1_BASE, COMP_E_OUTPUT_INTERRUPT);
    Interrupt_enableInterrupt(INT_COMP_E1);
    Interrupt_enableMaster();
    //![Simple COMP_E Example]

    /* Allow power to Comparator module */
    COMP_E_enableModule(COMP_E1_BASE);

    while (1)
    {
        /* Goto Sleep */
        PCM_gotoLPM0();
    }
}

/******************************************************************************
 *
 * This is the COMP_VECTOR interrupt vector service routine.
 *
 ******************************************************************************/
void COMP_E1_IRQHandler(void)
{

    /* Toggle the edge at which an interrupt is generated */
    COMP_E_toggleInterruptEdgeDirection(COMP_E1_BASE);

    /* Clear Interrupt flag */
    COMP_E_clearInterruptFlag(COMP_E1_BASE, COMP_E_OUTPUT_INTERRUPT_FLAG);

    /* Toggle P1.0 output pin */
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

}
