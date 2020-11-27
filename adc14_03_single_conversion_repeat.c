//*****************************************************************************
//
// MSP432 ADC14����03 - ��ͨ�����ת��ʾ��
//
// �����̲��õĹ���ģʽ�ǵ�ͨ�����ת����
// ÿ��ADת���Ľ�����洢�ڱ����У��������ж������öϵ�鿴;
// ����Ϊ�ֶ������ķ�ʽ��ÿ��ת�����������ж����ٴ�������ֶ�������һ��ת����
// ����3.3v�ο���ѹ����ADͨ�������뻻����˶�Ӧ�ĵ�ѹֵ��
//
// ˼��:
// ��ת�����ͨ��Һ��������UART��ʾ����;
//
// -------MSP-EXP432P401RӲ������ -----------
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

//������׼ͷ�ļ�����
#include <stdint.h>
#include <stdbool.h>

//����������DriverLib ͷ�ļ�����
//--�������˴�����,��鿴���̵�����, �����Ƿ��SDK·����������
//--Properties / Build / ARM Compiler / Include Options
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//��������
static volatile uint16_t curADCResult;
static volatile float normalizedADCRes;

int main(void)
{
    //ͣ�ÿ��Ź�
    WDT_A_holdTimer();

    curADCResult = 0;

    //����Flash�ȴ�ʱ��
    FlashCtl_setWaitState(FLASH_BANK0, 1);
    FlashCtl_setWaitState(FLASH_BANK1, 1);
    
    //����DCOΪ48MHz
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
   // CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_setDCOFrequency(CS_48MHZ);

    //ʹ��FPU,ͬʱ�����ж���ʹ�ø�������
    FPU_enableModule();
    FPU_enableLazyStacking();

    //ADC��ʼ�� (MCLK/1/4)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4,ADC_NOROUTE);

    //����GPIO����ģ������(5.5 A0)
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,GPIO_TERTIARY_MODULE_FUNCTION);

    //����ADC�洢�Ĵ���
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);

    //����Ϊ�ֶ������ķ�ʽ
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    //ʹ��ת�������������һ��ת��
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();

    //ʹ��ͨ��0���ж�
    ADC14_enableInterrupt(ADC_INT0);

    //ʹ��ģ����жϣ�ʹ��ȫ���ж�
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    while (1)
    {
        PCM_gotoLPM0();
    }
    
}

//===================================================================
//
// ADC���жϴ������
// ��ת������ҽ������ADC_MEM0��,�����жϡ�
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

