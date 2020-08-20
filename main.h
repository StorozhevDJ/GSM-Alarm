/**
  ******************************************************************************
  * @file    main.h
  * @author  Storozhev Denis
  * @version V1.4.0
  * @date    15-April-2013
  * @brief   main config file.
  * Update: ������� ��������, ��� �������� �� ������ ������������
  * 		(������ �� ������� ������������ PIR �������)
  ******************************************************************************/

#define DEBUG

#define LED_MODE_ALARM_READY	0x00010001	//00 0000 0000 0011 0000 0000 0000 0011
#define LED_MODE_ALARM_OFF	0x00000001
#define LED_MODE_ALARM_WAIT	0x07FFFFFF
#define LED_MODE_ALARM_TREVOGA 0x09249249	//00 1001 0010 0100 1001 0010 0100 1001

#define OUT2_ON			0xFFFFFFFF
#define OUT2_OFF			0x00000000
#define OUT2_BLINK_FAST		0x55555555
#define OUT2_BLINK_LOW		0x00000001
#define OUT2_BLINK_MIDLE		0x01010101


#define MAX_IBUTTON_KEYS	10


#define SIRENA_TIME_WORK		(5*60)		//��� ����� ������ ������ ����� ������������ ��������
#define ALARM_TIMEOUT		(60*60)		//�������� ������� �� ���������� �������� ������� ������������� ������������ � ����� ������ �� ������ �������
#define ALARM_WARNING_TIMEOUT	(5*60)		//���������� ����������, ��������� ���� �� ��� ������ ������������
#define AKB_CHECK_TIME		(1*60)		//�������� ������� ��� �������� ������������
#define AKB_LOW_LEVEL		(1200)		//����� ������� ���������� ��� (��������)
#define AKB_NOM_LEVEL		(1300)		//����� ����������� ���������� ���
#define AKB_DELTA_V			(20)			//����� ��������� ���������� �� ��� �� ������ ��� ��������� �������� �����/������

#define MODEM_TIME_CHECK_WORK	(60*60)		//�������� �������� ������ �� ���������



struct Tphone_book
	{
	char phone[14];	//Phone number
	bool sms_enable;	//alarm sms send enable
	bool sms_info_enable;//Info SMS send enable
	bool call_enable;	//alarm call enable
	bool access_enable;	//access alarm on/off enable
	bool send_balans;	//send balance report for this number
	};

enum Talarm_mode
	{
	alarm_ready=1,		//����� ������ �������
	alarm_off,		//����� ������ ��������
	alarm_warning_alarm,//����� ������ �������, �������� ������, �������� �� ������ ������������
	alarm_trevoga,		//�������� �������� ������, ����� ����������
	alarm_trevoga_end,	//���������� ���������, ���� ���������� ��������� �������, ���� ���������� � ������ ������
	alarm_wait_on		//�������� ��������� ������� � ����� ������ (�������� ������� �������� � ���� ���������
	};

extern unsigned long time_ms;
extern unsigned int  timeout;
extern unsigned long time_s;

void Timer_1ms(void);
void delay_mcs(int delay);
void delay_ms (unsigned int t);

bool iButtonCheck(void);

bool iButton(void);
