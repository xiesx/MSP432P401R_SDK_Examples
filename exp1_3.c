//*****************************************************************************
// ������ϰ1.3: ��������LED������
//
// ������ʵ�ַ���:
// ��ȡ�����ķ����ж���. ������ʹ�õ��Ƿ���һ.
// ����һ:
//   ������ѯ�ķ�ʽ,�ڳ������ѭ���в��ϵ�ȥ��ȡ���ŵ���ֵ,����͵�ƽ��˵����������.
//   ��������,���������ʱ20ms֮����ȥ��ȡ������ֵ;
// ������:
//   ���ö�ʱɨ��ķ�ʽ. ����20ms�Ķ�ʱ��,ÿ��20ms��ȡһ�����ŵ�����ֵ.�ж���ֵ�Ƿ����仯.
//   ����δ����ʱ,Ϊ�ߵ�ƽ;����ʱΪ�͵�ƽ.Ҳ����˵,ǰһ��Ϊ�ߵ�ƽ,��һ��Ϊ�͵�ƽ����϶�Ϊ1����Ч����.
// ������:
//   �����жϵķ�ʽ.
//
// һ�ΰ����������ܵĲ�������:
// ----<��������--------------------><�����ɿ�------
// ----+ +-+ +-+                  +-+ +-+ +----------------
//     | | | | |                  | | | | |
//     +-+ +-+ +------------------+ +-+ +-+
//
// ���ܳ��ֵĴ���:
// (1)Ҫ���ǵ���������������,��Ȼ1�ΰ�����������ʶ��Ϊ��ΰ�����
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

//����������DriverLib ͷ�ļ�����
//--�������˴�����,��鿴���̵�����, �����Ƿ��SDK·����������
//--Properties / Build / ARM Compiler / Include Options
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>


void main(void)
{
    volatile uint32_t ui32_delay;

    //�رտ��Ź���ʵ������, ����һ�㲻��Ҫ���Ź�, ֱ�ӹر�
    WDT_A_holdTimer();

    //����P1.0����LED1,��������P1.0Ϊ���
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    //P2.0������ɫLED2�ĺ�ɫ,��������P2.0Ϊ���
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);

    //led�ĳ�ʼ״̬ΪϨ��
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    //������������Ϊ����,����Ҫʹ���ڲ���������
    //p1.1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    //p1.4
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);

    while(1)
    {
        //�жϰ���p1.1
        if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1))
        {
            //������ʱ���ڰ�������,�ٴζ�ȡ�ж�
            for(ui32_delay=0;ui32_delay<1000;ui32_delay++);

            if(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1))
            {
                //ȡ��LED�����,������,�����.
                GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
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
                //ȡ��LED�����,������,�����.
                GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
                //�ȴ������ͷ�
                while(GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4));
            }
        }

    }
}
