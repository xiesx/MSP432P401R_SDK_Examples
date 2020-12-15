//*****************************************************************************
// ������ϰ2.6: ȫ��ɫLED�����
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

void main(void)
{
    volatile uint32_t ui32_delay;
    volatile uint32_t ui32_delay_config;
    volatile uint8_t index;

    //�رտ��Ź���ʵ������, ����һ�㲻��Ҫ���Ź�, ֱ�ӹر�
    WDT_A_holdTimer();

    //����P1.0����LED1,��������P1.0Ϊ���
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //led�ĳ�ʼ״̬ΪϨ��
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    //������������Ϊ����,����Ҫʹ���ڲ���������
    //p1.1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    //p1.4
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);


    index = 3;
    ui32_delay_config =delay_tab[index];

    while(1)
    {
        //-----ʶ���û��İ�������
        //�жϰ���p1.1
        if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1))
        {
            //������ʱ���ڰ�������,�ٴζ�ȡ�ж�
            for(ui32_delay=0;ui32_delay<1000;ui32_delay++);

            if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1))
            {
                //ͨ���ı������±�,�ı���ʱϵ��
                //ÿ�ΰ�������,�����±�����һλ,ʱ����,��˸���
                index --;
                if(0 == index) index = 5;
                ui32_delay_config =delay_tab[index];
                //�ȴ������ͷ�
                //˼��:������������,������ס���ɿ�,����ʲô����?
                while(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1));
            }
        }


        //�жϰ���p1.4
        if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4))
        {
            //������ʱ���ڰ�������,�ٴζ�ȡ�ж�
            for(ui32_delay=0;ui32_delay<1000;ui32_delay++);

            if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4))
            {
                //ÿ�ΰ�������,�����±�����һλ,ʱ��䳤,��˸����
                index ++;
                if(6 == index) index = 1;

                ui32_delay_config =delay_tab[index];

                //�ȴ������ͷ�
                while(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4));
            }
        }

        //-----ʵ��LED�����ȸı�

        //����ߵ�ƽ,LED��
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        //�˴�Ҫ����ʱ,��ʱԽ��,��ʾ����ʱ��Խ��
        for(ui32_delay=0;ui32_delay<ui32_delay_config;ui32_delay++);
        
        //����͵�ƽ,LED��
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        for(ui32_delay=0;ui32_delay<2000;ui32_delay++);

    }

}
