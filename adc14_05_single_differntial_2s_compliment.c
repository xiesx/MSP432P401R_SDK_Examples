//*****************************************************************************
//
// MSP432 ADC14����05 - ��ͨ�����ת�������ģʽ
//
// ������ADC14�Ĺ���ģʽ�ǵ�ͨ�����ת����ʹ������Զ�������
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
//       --|RST         P5.5  |<--- A0 (ģ������)
//         |            P5.4  |<--- A1 (ģ������)
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


//�������
static volatile float adcResult;
static float convertToFloat(uint16_t result);

int main(void)
{
    //ͣ�ÿ��Ź�
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();

    //ʹ��FPU,����stacking(���ж���ʹ��)
    FPU_enableModule();
    FPU_enableLazyStacking();

    //ADC��ʼ�� (MCLK/1/1)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    //��ͨ��ģʽ�����ת��
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    //�ο���ѹʹ��AVCC, ���ģʽ, ADC_MEM0 --> A0/A1
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, ADC_DIFFERENTIAL_INPUTS);

    //����P5.4,P5.5����ģ����������
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5 | GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

    //�޸�ADCת����������ݸ�ʽ
    ADC14_setResultFormat(ADC_SIGNED_BINARY);

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
// ÿ��ת�����ʱ,���ᴥ���жϡ�
// ��Ϊʹ����FPU��stackingģʽ,���ǿ������ж���߰�ȫ��ִ�и�Ч�ĸ�������
//
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT0)
    {
        adcResult = convertToFloat(ADC14_getResult(ADC_MEM0));
    }

}

//===================================================================
//
// ����3.3V�Ĳο���ѹ,��ADC��14-bitת����� ��ɸ�����
//
//===================================================================
static float convertToFloat(uint16_t result)
{
    int32_t temp;

        if(0x8000 & result)
        {
            temp = (result >> 2) | 0xFFFFC000;
            return ((temp * 3.3f) / 8191);
        }
        else
            return ((result >> 2)*3.3f) / 8191;
}
