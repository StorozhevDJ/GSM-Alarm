
//#define OUT2_ZAMOK
#define OUT2_SIGNAL

#define IN_OUT_LED_ON	GPIOB->BSRR=GPIO_BSRR_BS9	//�������� LED
#define IN_OUT_LED_OFF	GPIOB->BSRR=GPIO_BSRR_BR9	//��������� LED

#define IN_OUT_OUT1_ON	GPIOA->BSRR=GPIO_BSRR_BS5	//�������� ����� 1 (��� ������������ ������)
#define IN_OUT_OUT1_OFF	GPIOA->BSRR=GPIO_BSRR_BR5	//��������� �����1
#define IN_OUT_OUT2_ON	GPIOA->BSRR=GPIO_BSRR_BS4	//�������� ����� 2 (��� ���������� � �������� ������)
#define IN_OUT_OUT2_OFF	GPIOA->BSRR=GPIO_BSRR_BR4	//��������� ����� 2

#define SIRENA_ON		IN_OUT_OUT1_ON
#define SIRENA_OFF		IN_OUT_OUT1_OFF
#define ZAMOK_ON		IN_OUT_OUT2_ON
#define ZAMOK_OFF		IN_OUT_OUT2_OFF
#define SIGNAL_ON		IN_OUT_OUT2_ON
#define SIGNAL_OFF		IN_OUT_OUT2_OFF


#define IN_OUT_IN1		(GPIOA->IDR&GPIO_IDR_IDR6)	//������� ��� �� ����� 1
#define IN_OUT_IN2		(GPIOA->IDR&GPIO_IDR_IDR7)	//������� ��� �� ����� 2
#define IN_OUT_PIR		(GPIOC->IDR&GPIO_IDR_IDR13)	//������� ��� �� ����� PIR sensor

#define IN_OUT_IN1_ON	(IN_OUT_IN1==0)	//������� ��� �� ����� 1 (������ �� ���������)
#define IN_OUT_IN2_ON	(IN_OUT_IN2!=0)	//������� ��� �� ����� 2 (������ �� ������)
#define IN_OUT_PIR_ON	(IN_OUT_PIR!=0)	//������� ��� �� ����� PIR (������ ��������)


void In_Out_GPIO_Init(void);
void In_Out_ADC_Init(void);
void BatteryCheck_Init(void);
uint16_t BatteryCheck (void);
uint16_t Get_Temperature (void);



/*
����� ������ ��
1. ������
2. �������� ������
3. ��������� �����
4. ������������������� �����
5. ��������� �� ����������� �����
6. ����� �� �������� ���������

�����
1. ������ ��������
2. ������ �� ����������
3. ������ �� ���������
4. ������ ����
5. ������ ����
6. ������ ����
7. ����������� �����
*/