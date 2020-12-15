//*****************************************************************************
// ������ϰ2.2: ͨ��UART����LED��������
//
// �漰�����Ƶ�,�����ȶ���˫��ͨ�ŵ�Э��.
// ����Э��Ϊ4���ֽ�: AA BB CC DD
// ��1���ֽ�Ϊ AA��ʾ ֡ͷ, �̶�Ϊ0x5A
// ��2���ֽ�Ϊ BB��ʾ ����:
//         0x00��ѯLED״̬; 0x01��LED; 0x02,�ر�LED
// ��3���ֽ�Ϊ CC��ʾ ����,��ʾLED�����:
//         0x01,��1��;0x02,��2��;0x03,��3��, 0x04,��4��,0xFF,���е�LED��
// ��4���ֽ�Ϊ DD��ʾ ֡β, �̶�Ϊ0xA5
// ����,�����λ������ 5A 01 01 A5 -- ��LED1
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

#define  RX_BUFF_MAX  4

//����������ʱ��ŵ�����
volatile uint8_t rx_buf[RX_BUFF_MAX];
//������ڽ��յ�һ֡����,�ñ�־λ��1,���������ﴦ��
volatile uint8_t rx_flag;
//������Ŵ��ڽ��յ�������
volatile uint8_t app_cmd, app_data;

//������Ŵ��ڽ��յ�������
volatile uint8_t led1,led2,led3,led4;

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

    //SMCLK = DCO/1 = 12MHz,
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //����P1.0����LED1,��������P1.0Ϊ���
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    //LED2,��������P2.0/P2.1/P2.2Ϊ���
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 +GPIO_PIN1 +GPIO_PIN2);

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

    uart_send_string("Lab2.2 uart control led!\r\n");
    uart_send_string("--------------------------\r\n");
    uart_send_string("USE: 0x5A byte2 byte3 0xA5\r\n");
    uart_send_string("byte2 is cmd:\r\n");
    uart_send_string("----0x00:query led status\r\n");
    uart_send_string("----0x01:turn on led     \r\n");
    uart_send_string("----0x02-turn off led    \r\n");
    uart_send_string("byte3 is data, led number:\r\n");
    uart_send_string("----0x01:led1      \r\n");
    uart_send_string("----0x02:led2 red  \r\n");
    uart_send_string("----0x03:led2 green\r\n");
    uart_send_string("----0x04:led2 blue \r\n");
    uart_send_string("----0xff:all leds  \r\n");
    uart_send_string("--------------------------\r\n");

    //led�ĳ�ʼ״̬��ΪϨ��
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    led1=0;
    led2=0;
    led3=0;
    led4=0;

    rx_flag =0;

    while(1)
    {
        for( ui32_delay=0; ui32_delay <200; ui32_delay++);

        if(rx_flag == 1)
        {
            //���ڽ��յ�һ֡����,��Ҫ����
            rx_flag = 0;
            uart_send_string("uart get user data!\r\n");
            if(app_cmd == 0x00)
            {
                uart_send_string("cmd: query led status!\r\n");

                switch(app_data)
                {
                    case 0x01:
                        if(led1 == 1) uart_send_string("led1 is on!\r\n");
                        else uart_send_string("led1 is off!\r\n");
                        break;
                    case 0x02:
                        if(led2 == 1)uart_send_string("led2 red is on!\r\n");
                        else uart_send_string("led2 red is off!\r\n");
                        break;
                    case 0x03:
                        if(led3 == 1) uart_send_string("led2 green is on!\r\n");
                        else uart_send_string("led2 green is off!\r\n");
                        break;
                    case 0x04:
                        if(led4 == 1) uart_send_string("led2 blue is on!\r\n");
                        else uart_send_string("led2 blue is off!\r\n");
                        break;
                    case 0xFF:
                        //��ѯ���е�LED״̬
                        if(led1 == 1) uart_send_string("led1 is on!\r\n");
                        else uart_send_string("led1 is off!\r\n");
                        //led2-1
                        if(led2 == 1)uart_send_string("led2 red is on!\r\n");
                        else uart_send_string("led2 red is off!\r\n");
                        //led2-2
                        if(led3 == 1) uart_send_string("led2 green is on!\r\n");
                        else uart_send_string("led2 green is off!\r\n");
                        //led2-3
                        if(led4 == 1) uart_send_string("led2 blue is on!\r\n");
                        else uart_send_string("led2 blue is off!\r\n");
                        break;
                    default:
                        uart_send_string("unknown data!\r\n");
                        break;
                }


            }
            else if(app_cmd == 0x01)
            {
                uart_send_string("cmd: turn on led!\r\n");
                switch(app_data)
                {
                    case 0x01:
                        uart_send_string("turn on led1!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        led1=1;
                        break;
                    case 0x02:
                        uart_send_string("turn on led2 red!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                        led2=1;
                        break;
                    case 0x03:
                        uart_send_string("turn on led2 green!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
                        led3=1;
                        break;
                    case 0x04:
                        uart_send_string("turn on led2 blue!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
                        led4=1;
                        break;
                    case 0xFF:
                        uart_send_string("turn on all the leds!\r\n");
                        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        led1=0;
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                        led2=0;
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
                        led3=0;
                        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
                        led4=0;
                        break;
                    default:
                        uart_send_string("unknown data!\r\n");
                        break;
                }

            }
            else if(app_cmd == 0x02)
            {
                switch(app_data)
                {
                    case 0x01:
                        uart_send_string("turn off led1!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        led1=0;
                        break;
                    case 0x02:
                        uart_send_string("turn off led2 red!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                        led2=0;
                        break;
                    case 0x03:
                        uart_send_string("turn off led2 green!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
                        led3=0;
                        break;
                    case 0x04:
                        uart_send_string("turn off led2 blue!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
                        led4=0;
                        break;
                    case 0xFF:
                        uart_send_string("turn off all the leds!\r\n");
                        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        led1=0;
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                        led2=0;
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
                        led3=0;
                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
                        led4=0;
                        break;
                    default:
                        uart_send_string("unknown data!\r\n");
                        break;
                }
            }
            else
            {
                //δ���������
                uart_send_string("unknown command!!!\r\n");
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
        //Э�����ݽ϶�,ֱ�Ӳο�ǰ�ߵ�����
        //�ֶ�����һ������,�Ƚ��ȳ�
        //0-1-2-3; ÿ�ν���ʱ�������ƶ�һ��.
        rx_buf[0]=rx_buf[1];
        rx_buf[1]=rx_buf[2];
        rx_buf[2]=rx_buf[3];
        rx_buf[3]=UART_receiveData(EUSCI_A0_BASE);

        //�Խ��յ������ݽ����ж�,�ж�֡ͷ��֡β,�������Ҫ��,����Ϊ���յ���Ч����
        //ʵ��Ӧ����,һ����У��λ
        if((rx_buf[0] == 0x5A) && (rx_buf[3] == 0xA5))
        {
            //�ж����ֻ���ñ�־λ,ʣ�µĵ���������ߴ���
            if(rx_flag == 0)
            {
                app_cmd = rx_buf[1];
                app_data =rx_buf[2];
                //����������ݽ�����Ч��־λΪ1
                rx_flag = 1;
            }
            else
            {
                //�������,˵��ǰһ�ε����ݻ�δ����,����Ӵ�����
            }
        }
    }

}


