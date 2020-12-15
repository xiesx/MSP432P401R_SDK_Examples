//*****************************************************************************
// ������ϰ2.3: ͨ����ʱ��ʵ��LED��˸
//
// MSP432�ж��ֶ�ʱ��,����Timer32, TimerA, WDT, SystemTick.
// �ڸ�������,����ѡ��Timer32��ʱ��.
// ������SDK���Ҹ�����timer32_periodic_mode_led_toggle,�ڴ˻������޸�.
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



void main(void)
{

    //�رտ��Ź���ʵ������, ����һ�㲻��Ҫ���Ź�, ֱ�ӹر�
    WDT_A_holdTimer();

    //ʱ������, DCO=1MHz
    CS_setDCOFrequency(CS_1MHZ);

    //MCLK = DCO/1 = 1MHz,
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //����P1.0����LED1,��������P1.0Ϊ���
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //led�ĳ�ʼ״̬ΪϨ��
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);


    //����Timer32������ģʽ. ʱ��ԴΪMCLK,����Ƶ,1MHz
    Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,TIMER32_PERIODIC_MODE);

    //1MHz, 1->1us, 1000->1ms, 500 000 -->500ms=0.5s
    Timer32_setCount(TIMER32_BASE,500000);
    //ʹ�ܶ�ʱ���ж�
    Timer32_enableInterrupt(TIMER32_BASE);

    Interrupt_enableInterrupt(INT_T32_INT1);

    //�����ж�
    Interrupt_enableMaster();

    //���ö�ʱ��
    Timer32_startTimer(TIMER32_BASE, false);

    while (1)
    {
        
    }

}

