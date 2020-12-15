//*****************************************************************************
// ������ϰ2.5: ��PWM����LED������
//
//
// -------------MSP-EXP432P401RӲ������ -----------------
// (1) ���� 1����λ����,S3
// (2) ���������1���û�����,S1(P1.1),S2(P1.4)
// (3) 1����ɫ�����LED1(P1.0��)
// (4) 1����ɫ�����LED2(P2.0��/P2.1��/P2.2��)
//
//         MSP-EXP432P401R
//         +-------------+
//     /|\ |             |
//      |  |             |
//  S3  +--|RST          |
//         |         P1.0|---> LED1(��)
//  S1 --->|P1.1         |
//         |         P2.0|---> LED2(��)
//  S2 --->|P1.4     P2.1|---> LED2(��)
//         |         P2.2|---> LED2(��)
//         |             |
//         | P1.3/UCA0TXD|----> PC
//         | P1.2/UCA0RXD|<---- PC
//         |             |
//         |             |
// ������������Ϊ����,��ʹ���ڲ���������.����δ����ʱ,��ȡ�����Ǹߵ�ƽ;��������ʱ,�ӵ�,��ȡ�����ǵ͵�ƽ;
// LED��������Ϊ���������ߵ�ƽʱ������LED; ����͵�ƽʱ��Ϩ��LED��
//
//-------------------------------------------------------
//
//
//*****************************************************************************

//������׼ͷ�ļ�����
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

//����������DriverLib ͷ�ļ�����
//--�������˴�����,��鿴���̵�����, �����Ƿ��SDK·����������
//--Properties / Build / ARM Compiler / Include Options
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

//����LED����ʱʱ��,����ͨ��ʵ��,��ȡ5����ͬ���ȵȼ�.
//ע��,Ϊ�˽����Ѷȣ�������±�0��Ӧ��������ʱ����,�±�index��1-5.
uint32_t const delay_tab[]={0,100000,5000,2000,200,20};


#define KEY1_VALUE  0x01
#define KEY2_VALUE  0x02
#define NO_KEY      0x00

//�������
volatile uint8_t key_value;

//������ʱ��Ű����ļ�ֵ
volatile uint8_t pre_key1, now_key1;
volatile uint8_t pre_key2, now_key2;


//pwm���
Timer_A_PWMConfig pwmConfig =
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    32000,
    TIMER_A_CAPTURECOMPARE_REGISTER_0,
    TIMER_A_OUTPUTMODE_RESET_SET,
    3200
};


void main(void)
{
    volatile uint32_t ui32_delay;
    volatile uint32_t ui32_delay_config;
    volatile uint8_t index;

    //�رտ��Ź���ʵ������, ����һ�㲻��Ҫ���Ź�, ֱ�ӹر�
    WDT_A_holdTimer();

    //-------ϵͳʱ������---------
    //ʱ������, DCO=16MHz
    CS_setDCOFrequency(CS_16MHZ);
    //MCLK = DCO/1 = 16MHz
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //SMCLK = DCO/16 = 1MHz
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_16);

    //-------GPIO������---------
    //����P1.0����LED1,��������P1.0Ϊ���
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    //led�ĳ�ʼ״̬ΪϨ��
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    //p2.0
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    //������������Ϊ����,����Ҫʹ���ڲ���������,p1.1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    //p1.4
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);

    //����P2.0ΪPWM���
    //���ŵ����� �ɽ������� https://dev.ti.com/sysconfig
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring Timer_A to have a period of approximately 500ms and
     * an initial duty cycle of 10% of that (3200 ticks)  */
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    //-------��ʱ��Timer32����---------
    //����Timer32������ģʽ. ʱ��ԴΪMCLK,16��Ƶ,1MHz
    Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_16, TIMER32_32BIT,TIMER32_PERIODIC_MODE);

    //1MHz, 1->1us, 1000->1ms, 20 000 -->20ms
    Timer32_setCount(TIMER32_BASE,20000);
    //ʹ�ܶ�ʱ���ж�
    Timer32_enableInterrupt(TIMER32_BASE);

    Interrupt_enableInterrupt(INT_T32_INT1);

    //�����ж�
    Interrupt_enableMaster();

    //���ö�ʱ��
    Timer32_startTimer(TIMER32_BASE, false);

    pre_key1 = 1;
    now_key1 = 1;
    pre_key2 = 1;
    now_key2 = 1;

    //uart��صĴ����������

    while(1)
    {
        //�а�������,��Ҫ����
        if(key_value == KEY1_VALUE)
        {
            //����S1
            key_value = NO_KEY;

            if(pwmConfig.dutyCycle == 28800)
                pwmConfig.dutyCycle = 3200;
            else
                pwmConfig.dutyCycle += 3200;

            Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

        }
        else if(key_value == KEY2_VALUE)
        {
            //����S2
            key_value = NO_KEY;

            if(pwmConfig.dutyCycle == 3200)
                pwmConfig.dutyCycle = 28800;
            else
                pwmConfig.dutyCycle -= 3200;

            Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
        }

    }

}

// Timer32�жϴ�����
void T32_INT1_IRQHandler(void)
{
    //����жϱ�־λ
    Timer32_clearInterruptFlag(TIMER32_BASE);

    //��ȡ����ֵ1
    now_key1= GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1);
    //����δ����ʱδ�ߵ�ƽ,����ʱΪ�͵�ƽ.�԰���ֵ��������ж�
    //pre_key, now_key
    //   1       1      û�а�������
    //   1       0      ��������  --��Ϊ��һ����Ч����
    //   0       0      һֱ��ס����,����δ�ɿ�
    //   0       1      �����ɿ�
    if((pre_key1 == 1) && (now_key1 == 0))
    {
        key_value = KEY1_VALUE;
    }
    //ǰһ�ΰ���ֵΪ��ǰֵ
    pre_key1 = now_key1;

    //��ȡ����ֵ2
    now_key2= GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4);
    if((pre_key2 == 1) && (now_key2 == 0))
    {
        key_value = KEY2_VALUE;
    }
    //ǰһ�ΰ���ֵΪ��ǰֵ
    pre_key2 = now_key2;
}

