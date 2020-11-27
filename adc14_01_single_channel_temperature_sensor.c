//*****************************************************************************
//
// MSP432 ADC14����01 - �ڲ��¶ȴ���������
//
// �����̶��ڲ��¶ȴ��������в����������������Ӧ���¶ȡ�
// ADC14�����ڵ�ͨ�����ת��ģʽ��������������ķ�ʽ��
//
// ע������:
// �����ֲᣬ����ʱ�䲻������5us��
// ���ж��﷢���ļ��㣬����FPU��Stackingģʽ
// �����¶�ʱ���ȼ�������϶ȣ�Ȼ����ת���ɺͻ��϶ȡ�
//
// ˼����
// �뽫�¶�ת���Ľ��ʵʱ��ʾ����~ ͨ��Һ��������uart������.
//
// -------MSP-EXP432P401RӲ������ -----------
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

//������׼ͷ�ļ�����
#include <stdint.h>
#include <stdbool.h>

//����������DriverLib ͷ�ļ�����
//--�������˴�����,��鿴���̵�����, �����Ƿ��SDK·����������
//--Properties / Build / ARM Compiler / Include Options

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

uint32_t cal30;
uint32_t cal85;
float calDifference;
float tempC;
float tempF;

int main(void)
{
    //ͣ�ÿ��Ź�
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();

    //ʹ��FPU,����stacking(���ж���ʹ��)
    FPU_enableModule();
    FPU_enableLazyStacking();

    //ADC��ʼ�� (MCLK/1/1),�����ڲ�ͨ�����¶ȴ�������
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_TEMPSENSEMAP);

    //���õ�ͨ��ģʽ,���ת��
    ADC14_configureSingleSampleMode(ADC_MEM0, true);

    //����ͨ��A22(�������ֲ�,��ͨ��Ϊ�ڲ��¶ȴ�����)-->ADC_MEM0, �����ڲ��Ĳο���ѹ,�ǲ������
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A22, ADC_NONDIFFERENTIAL_INPUTS);

    //���ò���ʱ��Ϊ192��ADC CLK�������ֲᣬ����ʱ��������5us
    ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_192,ADC_PULSE_WIDTH_192);

    //���ò�����ʱ��Ϊ�Զ�ģʽ
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    //ʹ��ͨ��0���ж�
    ADC14_enableInterrupt(ADC_INT0);

    //���òο���ѹΪ2.5, ʹ���¶ȴ�����
    REF_A_enableTempSensor();
    REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    REF_A_enableReferenceVoltage();

    //��ȡоƬ�ϴ洢���¶�У׼ֵ
    cal30 = SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,SYSCTL_30_DEGREES_C);
    cal85 = SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,SYSCTL_85_DEGREES_C);
    calDifference = cal85 - cal30;

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
// �˴����öϵ�,�鿴����ֵ;
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
