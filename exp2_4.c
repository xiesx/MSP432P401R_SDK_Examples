//*****************************************************************************
// ������ϰ2.4: ���ڶ�ʱ���İ������
//
// MSP432�ж��ֶ�ʱ��,����Timer32, TimerA, WDT, SystemTick.
// �ڸ�������,����ѡ��Timer32��ʱ��. ����һ��20ms�Ķ�ʱ��
// ÿ20msȥ��ȡһ������ֵ
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

//�������
//key_flag,��⵽�����ɿ�,��Ϊ��һ�������İ���
//key_time,������¼�������µ�ʱ��
volatile uint8_t key_flag, key_time;

//������ʱ��Ű����ļ�ֵ
volatile uint8_t pre_key, now_key;

void main(void)
{
    volatile uint32_t ui32_delay;
    volatile uint32_t ui32_delay_config;
    volatile uint8_t index;

    //�رտ��Ź���ʵ������, ����һ�㲻��Ҫ���Ź�, ֱ�ӹر�
    WDT_A_holdTimer();

    //-------ϵͳʱ������---------
    //ʱ������, DCO=1MHz
    CS_setDCOFrequency(CS_1MHZ);
    //MCLK = DCO/1 = 1MHz,
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

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

    //-------��ʱ��Timer32����---------
    //����Timer32������ģʽ. ʱ��ԴΪMCLK,����Ƶ,1MHz
    Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,TIMER32_PERIODIC_MODE);

    //1MHz, 1->1us, 1000->1ms, 20 000 -->20ms
    Timer32_setCount(TIMER32_BASE,20000);
    //ʹ�ܶ�ʱ���ж�
    Timer32_enableInterrupt(TIMER32_BASE);

    Interrupt_enableInterrupt(INT_T32_INT1);

    //�����ж�
    Interrupt_enableMaster();

    //���ö�ʱ��
    Timer32_startTimer(TIMER32_BASE, false);

    pre_key = 1;
    now_key = 1;

    //uart��صĴ����������

    while(1)
    {
        //�а�������,��Ҫ����
        if(key_flag==1)
        {
            //�������öϵ�,�����Լ��̰�����ʱ��

            //�жϳ��������Ƕ̰���
            if(key_time > 12)
            {
                //������
                GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
            }
            else
            {
                //�̰���
                GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            }

            key_flag =0;
            key_time =0;
        }

    }

}

// Timer32�жϴ�����
void T32_INT1_IRQHandler(void)
{
    //����жϱ�־λ
    Timer32_clearInterruptFlag(TIMER32_BASE);

    //��ȡ����ֵ
    now_key= GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1);

    //����δ����ʱδ�ߵ�ƽ,����ʱΪ�͵�ƽ.�԰���ֵ��������ж�
    //pre_key, now_key
    //   1       1      û�а�������
    //   1       0      ��������
    //   0       0      һֱ��ס����,����δ�ɿ� --> key_time ++;
    //   0       1      �����ɿ�  -->���ñ�־λ key_flag=1
    if((pre_key == 1) && (now_key == 0))
    {
        key_time = 1;
    }
    else if((pre_key == 0) && (now_key == 0))
    {
        key_time ++;
    }
    else if((pre_key == 0) && (now_key == 1))
    {
        //һ�ΰ�����������
        key_flag=1;
    }

    //ǰһ�ΰ���ֵΪ��ǰֵ
    pre_key = now_key;
}

