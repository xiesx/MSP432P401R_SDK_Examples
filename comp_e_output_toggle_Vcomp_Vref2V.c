/*******************************************************************************
 * MSP432 Comparator - Comparator Toggle from SLEEP; input channel C6;
 *      Vcompare is compared against the internal reference 2.0V
 *
 * Description: Use the comparator (input channel C6) and internal reference to
 * determine if input'Vcompare'is high or low.  When Vcompare exceeds 2.0V
 * COUT goes low and when Vcompare is less than 2.0V then COUT goes high.
 *
 *                 MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST        P5.7/C6|<--Vcompare
 *            |                  |
 *            |         P7.2/COUT|----> 'high'(Vcompare<2.0V); 'low'(Vcompare>2.0V)
 *            |                  |  
 *            |                  |  
 *            |                  |
 ******************************************************************************/
//*****************************************************************************
//
// MSP432�Ƚ�������01 - �򵥵�ѹ�Ƚ�
//
// �������ϵ������ѹ���ڲ�1.2V�ο���ѹ���бȽϣ�
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

//�Ƚ����Ĳ�������
const COMP_E_Config compConfig =
{
     COMP_E_VREF,                  // Positive Input Terminal
     COMP_E_INPUT6,                // Negative Input Terminal
     COMP_E_FILTEROUTPUT_DLYLVL4,  // Filter level of 4
     COMP_E_NORMALOUTPUTPOLARITY   // Normal Output Polarity
};

int main(void)
{
    //ͣ�ÿ��Ź�
    MAP_WDT_A_holdTimer();

    /*
     * Select Port 7
     * Set Pin 1 to output Primary Module Function, (COUT).
     */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN2,
            GPIO_PRIMARY_MODULE_FUNCTION);

    /* Set P5.7 to be comparator in (C1.6) */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P7, GPIO_PIN7,
            GPIO_TERTIARY_MODULE_FUNCTION);

    /* Initialize the Comparator E module
     *  Comparator Instance 1,
     *  Pin CE6 to Positive(+) Terminal,
     *  Reference Voltage to Negative(-) Terminal,
     *  Normal Power Mode,
     *  Output Filter On with max delay,
     *  Non-Inverted Output Polarity
     */
    MAP_COMP_E_initModule(COMP_E1_BASE, &compConfig);

    /*Set the reference voltage that is being supplied to the (-) terminal
     *  Comparator Instance 1,
     *  Reference Voltage of 2.0 V,
     *  Lower Limit of 2.0*(32/32) = 2.0V,
     *  Upper Limit of 2.0*(32/32) = 2.0V
     */
    MAP_COMP_E_setReferenceVoltage(COMP_E1_BASE, COMP_E_VREFBASE2_0V, 32, 32);

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

    MAP_PCM_gotoLPM0();
    __no_operation();
}
