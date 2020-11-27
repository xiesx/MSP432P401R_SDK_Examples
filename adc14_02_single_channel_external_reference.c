//*****************************************************************************
//
// MSP432 ADC14����02 - ��ͨ������ת��ʾ�⣨ʹ���ⲿ��׼Դ��
//
// ������ʾADCģ��ʹ���ⲿ��׼Դ��
// ������һ������ͨ��A0 (P5.5)���в���/ת����ת������洢�����ر���adcResult;
// ����VREF+ (P5.6)�ϵĵ�ѹֵΪ���ֵ,����VREF- (P5.7)�ϵĵ�ѹֵΪ��Сֵ;
//
// -------MSP-EXP432P401RӲ������ -----------
//
//           MSP-EXP432P401R
//         +------------------+
//      /|\|                  |
//       | |                  |
//       --|RST         P5.5  |<---- A0 In
//         |            P5.6  |<---- VREF+
//         |            P5.7  |<---- VREF-
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


volatile uint16_t adcResult;

int main(void)
{
    //ͣ�ÿ��Ź�
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();
    
    //ADC��ʼ�� (MCLK/1/1),�������ڲ�ͨ��
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    //����Ϊ��ͨ��ģʽ�� ���ת��
    ADC14_configureSingleSampleMode(ADC_MEM0, true);

    //����ADC�洢�Ĵ���(A0->ADC_MEM0 ), ʹ���ⲿ��׼, �ǲ������
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_EXTPOS_VREFNEG_EXTNEG, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);

    //����GPIOs����ģ��˿����� (AD����ͻ�׼����)
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN7 | GPIO_PIN6 | GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);

    //���ò�����ʱ��Ϊ�Զ�ģʽ
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    //ʹ��ͨ��0���ж�
    ADC14_enableInterrupt(ADC_INT0);

    //ʹ��ģ����жϣ�ʹ��ȫ���ж�
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

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
//
// ��ת������ҽ������ADC_MEM0��,�����жϡ�
//
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT0)
    {
        adcResult = ADC14_getResult(ADC_MEM0);
    }

}
