//*****************************************************************************
//
// MSP432 ADC14����04 - ��ͨ�����ת����ʹ��TimerA��Ϊ����
//
// ������ADC14�Ĺ���ģʽ�ǵ�ͨ�����ת����ʹ��TimerA CCR1�������Ϊ������
// ���ⲿͨ��A0���в�����ֱ��ʹ��AVCC(3.3v)��Ϊ�ο���ѹԴ;
// ����Timer_AΪ������ģʽ,���ñȽ���CCR0,������16384(0.5��),�ᷭת�Ƚ��������;
// ��Timer_A�����󣬽�ÿ���0.5s����ADC14��ʼת����
// ����resultsBuffer���ڴ��ADC��ת�����(����ѭ��ʹ��,resPos�����0��ѭ��������)
//
// -------MSP-EXP432P401RӲ������ -----------
//
//           MSP-EXP432P401R
//         +------------------+
//      /|\|                  |
//       | |                  |
//       --|RST         P5.5  |<--- A0 (Analog Input)
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

//��ʱ��Timer_A������ģʽ�Ĳ�������
const Timer_A_UpModeConfig upModeConfig =
{
    TIMER_A_CLOCKSOURCE_ACLK,            // ѡ��ʱ��ԴΪACLK
    TIMER_A_CLOCKSOURCE_DIVIDER_1,       // ����Ƶ, ACLK/1 = 32Khz
    16384,                               // 16384/32768 = 0.5s
    TIMER_A_TAIE_INTERRUPT_DISABLE,      // ����Timer ISR
    TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE, // ����CCR0
    TIMER_A_DO_CLEAR                     // Clear Counter
};

//Timer_A�Ƚ�����������
const Timer_A_CompareModeConfig compareConfig =
{
    TIMER_A_CAPTURECOMPARE_REGISTER_1,          // ʹ��CCR1
    TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // ����CCR�ж�
    TIMER_A_OUTPUTMODE_SET_RESET,               // ȡ�����
    16384                                       // 16384/32768 = 0.5s
};

//��������
//�궨��UINT8_MAX��<stdint.h>��,ֵ��0xFF
static volatile uint_fast16_t resultsBuffer[UINT8_MAX];
static volatile uint8_t resPos;

int main(void)
{
    //ͣ�ÿ��Ź�
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();
    resPos = 0;

    //ϵͳʱ�����ã�ʹ��Ĭ��ʱ��;
    //Ĭ��ʱ��: MCLK = DCO = ~3MHz, ACLK = REFO = 32Khz
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //ADC��ʼ�� (MCLK/1/1)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    //����GPIO������������(P5.5-->A0)
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);

    //����Ϊ��ͨ��ģʽ�� ���ת��
    ADC14_configureSingleSampleMode(ADC_MEM0, true);

    //����ADC�洢�Ĵ���(A0->ADC_MEM0 ), ʹ���ⲿ��׼, �ǲ������
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);

    //���ݲ����趨,���ö�ʱ��Timer_A0
    Timer_A_configureUpMode(TIMER_A0_BASE, &upModeConfig);

    //���ݲ����趨,����Timer_A0��CCR1
    Timer_A_initCompare(TIMER_A0_BASE, &compareConfig);

    //���ô���Դ����TA0_C1(�����ֲ�)
    ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

    //ʹ��ͨ��0���ж�
    ADC14_enableInterrupt(ADC_INT0);
    ADC14_enableConversion();

    //ʹ��ģ����жϣ�ʹ��ȫ���ж�
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    //���ö�ʱ��,���ڴ���ADת��
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    //����˯��ģʽ
    while (1)
    {
        PCM_gotoLPM0();
    }
}

//===================================================================
//
// ��ת������ҽ������ADC_MEM0��,�����жϡ�
// �������ŵ�������;
//===================================================================
void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);


    if (status & ADC_INT0)
    {
        if(resPos == UINT8_MAX)
        {
           resPos = 0; 
        }
        
        resultsBuffer[resPos++] = ADC14_getResult(ADC_MEM0);
    }

}

