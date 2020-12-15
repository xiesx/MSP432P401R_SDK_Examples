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
// MSP432�Ƚ�������01 - �򵥵�ѹ�Ƚ�
//
// �����̽������ϵ������ѹ���ڲ�1.2V�ο���ѹ���бȽϣ�
// ���ⲿͨ��A0��A1���в��������ģʽ��ֱ��ʹ��AVCC(3.3v)��Ϊ�ο���ѹԴ;
// ����ת���������ʽΪ������ʽ��
// ���ж��н�ֵת��Ϊ�����ͣ��Թ��û��ڵ������й۲졣
//
// -------MSP-EXP432P401RӲ������ -----------
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

//������׼ͷ�ļ�����
#include <stdint.h>
#include <stdbool.h>

//����������DriverLib ͷ�ļ�����
//--�������˴�����,��鿴���̵�����, �����Ƿ��SDK·����������
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

    //ͣ�ÿ��Ź�
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
