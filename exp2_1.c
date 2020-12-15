//*****************************************************************************
// ������ϰ2.1: UART�Ļ����շ�ʵ��
//
// �����ʼ������ʹ��������ҳС����:
// http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
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

//uart�Ļ�����������
//9600,��У��λ,ֹͣλ1
//����ҳС������, ѡ��eUSCI, Clock = 12M, 9600, �ó�:
//clockPrescalar:78
//firstModReg:   2
//secondModReg:  0
//overSampleing: 1
const eUSCI_UART_Config uartConfig =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK, //ʱ��Դʹ��SMCLK
    78,                             //BRDIV = 78, ��3����������ͨ��С���߼���
    2,                              //UCxBRF = 2
    0,                              //UCxBRS = 0
    EUSCI_A_UART_NO_PARITY,         //��У��λ
    EUSCI_A_UART_LSB_FIRST,         //LSB First
    EUSCI_A_UART_ONE_STOP_BIT,      //1λֹͣλ
    EUSCI_A_UART_MODE,              //UARTģʽ
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  //�ο�С�������һ��
};

#define   RX_BUFF_MAX  5

volatile uint8_t rx_buf[RX_BUFF_MAX];
volatile uint8_t rx_flag;

//�����ַ����ĺ���
//�����ַ���,�ַ�����βΪ'\0',���ô��ڷ��ͺ���,�ӵ�һ���ֽڿ�ʼ����
void uart_send_string(unsigned char *pt)
{
    while(*pt!= '\0')
    {
        UART_transmitData(EUSCI_A0_BASE, *pt);
        pt++;
    }
}

void main(void)
{
    volatile uint32_t ui32_delay;

    //�رտ��Ź���ʵ������, ����һ�㲻��Ҫ���Ź�, ֱ�ӹر�
    WDT_A_holdTimer();

    //ʱ������, DCO=12MHz
    CS_setDCOFrequency(CS_12MHZ);

    //SMCLK = DCO = 12MHz,
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //����P1.0����LED1,��������P1.0Ϊ���
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    //����P1.2��P1.3������UARTģʽ
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    //����UART
    UART_initModule(EUSCI_A0_BASE, &uartConfig);

    //ʹ������UART
    UART_enableModule(EUSCI_A0_BASE);

    //�ж��������
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);
    Interrupt_enableMaster();

    uart_send_string("Lab2.1 uart experiment!\r\n");

    //UART����"I am AI!",һ��ֻ�ܷ���һ���ַ�.
    //�˴��ɵ�������,PC�������յ������ַ�. ÿ�ζ���ô����,̫�鷳��,����дһ���ַ������ͺ���
    UART_transmitData(EUSCI_A0_BASE, 'I');
    UART_transmitData(EUSCI_A0_BASE, ' ');
    UART_transmitData(EUSCI_A0_BASE, 'a');
    UART_transmitData(EUSCI_A0_BASE, 'm');
    UART_transmitData(EUSCI_A0_BASE, ' ');
    UART_transmitData(EUSCI_A0_BASE, 'A');
    UART_transmitData(EUSCI_A0_BASE, 'I');
    UART_transmitData(EUSCI_A0_BASE, '!');

    //ֱ�ӵ����ַ�������
    uart_send_string("I am AI!\r\n");

    while(1)
    {
        //led����˸,��ʾ����������

        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

        //��ʱ
        for(ui32_delay=0;ui32_delay<1000;ui32_delay++)
        {
            //��led����ʱѭ�����,������ʱ�Դ������ݵ�Ӱ��
            if(1 == rx_flag)
            {
                rx_flag = 0;
                uart_send_string("hello\r\n");

            }
            else if(2 == rx_flag)
            {
                rx_flag = 0;
                uart_send_string("04045073\r\n");

            }
            else if(3 == rx_flag)
            {
                rx_flag = 0;
                uart_send_string("byebye\r\n");

            }
        }
    }
}

//�жϴ�����EUSCI A0 UART ISR
void EUSCIA0_IRQHandler(void)
{
    uint32_t status;

    //��ȡ�жϱ�־
    status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);
    //����жϱ�־
    UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    //����ǰ�߶�ȡ���жϱ�־, �ж��ж�����, �˴��� �����ж�
    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        //һ�㶨�������,ֻ�Ǹı��±�ķ�ʽ.
        //�������̱Ƚϼ�,��ַ�Ϊ5���ַ�. �ֶ�����һ������,�Ƚ��ȳ�
        //0-1-2-3-4 ; ÿ�ν���ʱ�������ƶ�һ��.
        rx_buf[0]=rx_buf[1];
        rx_buf[1]=rx_buf[2];
        rx_buf[2]=rx_buf[3];
        rx_buf[3]=rx_buf[4];
        rx_buf[4]=UART_receiveData(EUSCI_A0_BASE);

        //�Խ��յ������ݽ����ж�
        //hello, who?, what?
        if((rx_buf[0] =='h')&&
                (rx_buf[1] == 'e') &&
                (rx_buf[2] == 'l') &&
                (rx_buf[3] == 'l') &&
                (rx_buf[4] == 'o'))
        {
            rx_flag = 1;
        }
        else if((rx_buf[1] == 'w') &&
                (rx_buf[2] == 'h') &&
                (rx_buf[3] == 'o') &&
                (rx_buf[4] == '?'))
        {
            //Ҫע�����±�Ŷ,�жϺ�ߵ�4��
            rx_flag = 2;
        }
        else if((rx_buf[0] == 'w') &&
                (rx_buf[1] == 'h') &&
                (rx_buf[2] == 'a') &&
                (rx_buf[3] == 't') &&
                (rx_buf[4] == '?'))
        {
            rx_flag = 3;
        }
    }

}

