//*****************************************************************************
//
// MSP432 ADC14����06 - ��ͨ�����ת��,���ô��ڱȽ���
//
// �����̲��õĹ���ģʽ�ǵ�ͨ�����ת�������ô��ڱȽ����Ĺ��ܡ�
// ʹ���ڲ�2.5V�Ĳο���ѹ,��ͨ��A0���в���;
// ���ڱȽ�����ֵ�����޺����޶�����Ϊ1V,Ҳ����˵ֵ�������ߵ���1v�ͻᴥ���ж�;
// ��LED��Ϊָʾ�ƣ�������ֵ����1Vʱ����LED��С��1Vʱ, Ϩ��LED;
//
// ˼����
// �������ʲô�����أ�
// �Դ������޸ģ���ֵ������������Ϊ��ͬ��ֵ��
//
// -------MSP-EXP432P401RӲ������ -----------
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

//������׼ͷ�ļ�����
#include <stdint.h>
#include <stdbool.h>

//����������DriverLib ͷ�ļ�����
//--�������˴�����,��鿴���̵�����, �����Ƿ��SDK·����������
//--Properties / Build / ARM Compiler / Include Options
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>


int main(void)
{
    //ͣ�ÿ��Ź�
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();

    //����Flash�ĵȴ�ʱ��
    FlashCtl_setWaitState(FLASH_BANK0, 1);
    FlashCtl_setWaitState(FLASH_BANK1, 1);
    
    //����DCOΪ48MHz
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
   // CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_setDCOFrequency(CS_48MHZ);

    //�����ڲ��ο�ԴΪ2.5V, �����ڲ��ο�Դ
    REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    REF_A_enableReferenceVoltage();

    //ADC��ʼ�� (MCLK/1/4)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4,ADC_NOROUTE);

    //����P1.0ΪGPIO�������
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    //����P5.5����ģʽ��������A0
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);

    //��ͨ��ģʽ�����ת��
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    //ʹ���ڲ��Ĳο���ѹԴ���ǲ������
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS,ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);
    //���ò�������/����ʱ��
    ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_128, ADC_PULSE_WIDTH_128);

    //���ô��ڱȽ�����ֵ�����޺����ޣ�
    //2^14=16384--2.5V, 9830--1.5V , 6553--1.0V
//    ADC14_setComparatorWindowValue(ADC_COMP_WINDOW0, 6553, 9830);
    ADC14_setComparatorWindowValue(ADC_COMP_WINDOW0, 6553, 6553);
    //����MEM0�Ĵ��ڱȽ�������
    ADC14_enableComparatorWindow(ADC_MEM0, ADC_COMP_WINDOW0);
    //������жϱ�־λ�������ж�
    ADC14_clearInterruptFlag(ADC_HI_INT | ADC_LO_INT);
    ADC14_enableInterrupt(ADC_HI_INT | ADC_LO_INT);

    //���ò�����ʱ��Ϊ�Զ�ģʽ
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    //ʹ��ת�������������һ��ת��
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();

    //ʹ��ģ����жϣ�ʹ��ȫ���ж�
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    //����͹���ģʽ
    while (1)
    {
        PCM_gotoLPM0();
    }
}

//===================================================================
//
// ADC���ڱȽ������жϴ���.
// ���������ж�,˵�������A0�ĵ�ѹҪô����1.0V,ҪôС��1.0V��
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

