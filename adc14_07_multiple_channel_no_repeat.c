//*****************************************************************************
//
// MSP432 ADC14����07 - ����ͨ������ת��
//
// �����̲��õĹ���ģʽ������ͨ������ת�����Զ������ķ�ʽ��
// ����ADC_MEM0-ADC_MEM7���ڴ������A0-A7��ת�������
// ���һ��ͨ��ת����ɺ󣬽�����ADC_MEM7���жϣ�����洢������resultsBuffer�С�
// ����ת��ģʽ������ͨ��������һ�β������������ж������öϵ�鿴�������
//
// -------MSP-EXP432P401RӲ������ -----------
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

//������׼ͷ�ļ�����
#include <stdint.h>
#include <string.h>

//����������DriverLib ͷ�ļ�����
//--�������˴�����,��鿴���̵�����, �����Ƿ��SDK·����������
//--Properties / Build / ARM Compiler / Include Options
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>


//��������,�洢ת�����
static uint16_t resultsBuffer[8];

int main(void)
{
    //ͣ�ÿ��Ź�
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();

    //�������
    memset(resultsBuffer, 0x00, 8 * sizeof(uint16_t));

    //�ο���ѹԴ���á�
    //���òο���ѹΪ2.5V, ʹ�ܲο���ѹ
    REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    REF_A_enableReferenceVoltage();

    //ADC��ʼ�� (MCLK/1/1)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1,ADC_NOROUTE);

    //����Ӧ���������ó�ģ����������
    //P5.4/P5.3/P5.2/P5.1/P5.0
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN5 | GPIO_PIN4 | GPIO_PIN3 | GPIO_PIN2 | GPIO_PIN1| GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
    //P4.7, P4.6
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,GPIO_PIN7 | GPIO_PIN6, GPIO_TERTIARY_MODULE_FUNCTION);


    //��������ͨ��ģʽ������ת��
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM7, true);
    //����ADC�洢�Ĵ���ADC_MEM0-ADC_MEM7 (A0 - A7), ʹ���ڲ���׼Դ2.5v, �ǲ������
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A1, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM2, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A2, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM3, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A3, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM4, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A4, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM5, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A5, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM6, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A6, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM7, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A7, ADC_NONDIFFERENTIAL_INPUTS);

    //ʹ��ͨ��7���ж� (����ͨ�������1��)
    ADC14_enableInterrupt(ADC_INT7);

    //ʹ��ģ����жϣ�ʹ��ȫ���ж�
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    //���ò�����ʱ��Ϊ�Զ�ģʽ
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    //ʹ��ת�������������һ��ת��
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();

    //����˯��ģʽ
    while (1)
    {
        PCM_gotoLPM0();
    }
}

//===================================================================
// ��ת������ҽ������ADC_MEM7��,�����жϡ�
// ָʾת������, ��������ݴ���resultsBuffer
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


