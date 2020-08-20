
#include <stdlib.h>

#include "main.h"

#include "EE_emul\EEPROM.h"

#include "In_Out.h"

//#include "GSM_Shield.h"
#include "GSM_GPRS\SIM900.h"
#include "GSM_GPRS\inetGSM.h"
#include "GSM_GPRS\call.h"
#include "GSM_GPRS\sms.h"
#include "GSM_GPRS\GSM.h"

#include "PDU/pdu.h"

#include "SMS_protocol.h"

#include "MyGSM.h"



extern Tphone_book phone_book[];

extern char balance_number;
extern long balance_period;

extern Talarm_mode alarm_mode;
extern unsigned int LED_mode;
extern unsigned int OUT2_mode;

extern unsigned long time_ms;
extern unsigned long time_s;


//GSM gsm;
CallGSM call_gsm;
InetGSM inet;



char number[13];  //Destination number 



//***************************************************************************************
//������ ���������� ����� � ��� ����� � ���
//***************************************************************************************
void ReadPhoneBook(void)
{
char phone_num=0;
char phone[13];

for (char i=1; i<100; i++)	//������ ��� ���������� �����
	{
	char param[20];
	if (gsm.GetPhoneNumberName(i, phone, param)==1)//���� ������ ����
		{
		if (((param[0]=='t')||(param[0]=='T'))&&		//���� ��� ����� ������������ � "tn="
		    ((param[2]=='=')))
			{
			phone_num=(param[1]&0x0f)%10;
			strcpy (phone_book[phone_num].phone, phone);
			for (char j=3; j<20; j++)				//�������� �������� ����������
				{
				if (param[j])		//���� �� ����� ������
					{
					if      ((param[j]=='s')||(param[j]=='S')) phone_book[phone_num].sms_enable=true;
					else if ((param[j]=='c')||(param[j]=='C')) phone_book[phone_num].call_enable=true;
					else if ((param[j]=='a')||(param[j]=='A')) phone_book[phone_num].access_enable=true;
					else if ((param[j]=='b')||(param[j]=='B')) phone_book[phone_num].send_balans=true;
					else if ((param[j]=='i')||(param[j]=='I')) phone_book[phone_num].sms_info_enable=true;
					}
				else break;
				}
			}
		else if (((param[0]=='b')||(param[0]=='B'))&&	//���� ��� ����� ������������ � "Bal"
			    ((param[1]=='a')||(param[1]=='A'))&&
			    ((param[2]=='l')||(param[2]=='L'))&&
			    ((param[3]=='=')))
			{
			//������ ��� ����� ��� �������� �������
			balance_number=atoi(&phone[1]);
			balance_period=(unsigned long)(atoi(&param[4])*(60*60*24));
			if (balance_period<(60*60*24)) balance_period=(60*60*24)*30;
			}
		}
	}
}



//***************************************************************************************
//�������� ������� ��������� ������ (��� ��������� � ����������/������ � ������ ������)
//***************************************************************************************
void CallCheck(void)
{
IWDG_ReloadCounter();

byte call_stat=call_gsm.CallStatusWithAuth(&number[0], 0, 0);
if (call_stat==CALL_INCOM_VOICE_AUTH)
	{
	char i;
	//���� ����� ��������� � ���������� �����
	for (i=0; i<10; i++) if (strcmp(&number[0], phone_book[i].phone)==0) break;
	if (i<10)				//���� ����� ������
		{
		SIRENA_OFF;
		char s;
		for (s=0; s<15; s++)	//� ����� �� 15 ���
			{
			while(time_ms%1000);	//���� ��������� �������
			byte call_stat=call_gsm.CallStatus();//����������� ������ ������
			
			if ((call_stat==CALL_NO_RESPONSE)||//���� ��� ������
			    (call_stat==CALL_NONE)||		//��� ��������
			    (call_stat==CALL_COMM_LINE_BUSY))//��� ����� ������
				{
				break;					//�������� ������
				}
			}
		IWDG_ReloadCounter();
		
		//������ ������� �� ��������� 15 ���, � ��������� ��������� ������� ������, ����� ������� ����� ������
		if ((s<15)&&(phone_book[i].access_enable))
			{
			//���� �������� ����� � ������� �� ���������,
			if (alarm_mode!=alarm_off)
				{
				LED_mode=LED_MODE_ALARM_WAIT;
				OUT2_mode=OUT2_BLINK_MIDLE;
				
				//���� 60 ��� ��� ������������ �������
				int time_tmp=time_s+60;
				while (time_tmp>time_s)
					{
					if (IN_OUT_IN1_ON || IN_OUT_IN2_ON || IN_OUT_PIR_ON)//���� ������ ��������
						{
						AlarmModeSet(alarm_off);		//�������� ����� ������
						char smsmsg[60]={"������ ��������� �� ������: "};
						strcpy(&smsmsg[27], phone_book[i].phone);
						AlarmSendSMS(smsmsg, GSM_FLASH, 'i');
						return;
						}
					while(time_ms%100);
					IWDG_ReloadCounter();
					}
				//���� � ������ �� ����� � ������� 60 ��� (������ �� ��������), �� �������� � �������� �����
				AlarmModeSet(alarm_ready);
				return;
				}
			else
				{
				//�������� ����� �� ����������� ������
				AlarmModeSet(alarm_wait_on);
				char smsmsg[60]={"������ �������� �� ������: "};
				strcpy(&smsmsg[27], phone_book[i].phone);
				AlarmSendSMS(smsmsg, GSM_FLASH, 'i');
				}
			}
		else		//����� �� ���������� � ������� 15 ���, ������ �������
			{
			SIRENA_OFF;
			call_gsm.PickUp();	//�������� ������
			short call_time=0;
			while (call_gsm.CallStatus()!=CALL_NONE)
				{
				while(time_ms%1000);
				IWDG_ReloadCounter();
				if (call_time++>=60*60) break;
				}
			}
		}
	else call_gsm.HangUp();	//������� ������, �� ����� �������� � ������������
	}
}



//***************************************************************************************
//��������� � ���������� �������, �������� �� ��� ������, ������� ��� ���������
//***************************************************************************************
void AlarmCall(void)
{
call_ret_val_enum call_stat;
char number[13];
short s;
unsigned long time_tmp;

for (char i=0; i<10; i++)
	{
	if ((phone_book[i].call_enable)&&	//�������� ���������� ������� ���������
	    (phone_book[i].phone[0]!=0))	//� ����� ������� � ���������� �����
		{
		gsm.ModemWakeUp();			//����� ����� (���� �� ����)
		//�������� �����
		call_gsm.Call(&phone_book[i].phone[0]);
		for (s=0; s<60; s++)		//���� �� 1 ����� ���� �������
			{
			time_tmp=time_ms;
			while(time_ms<(time_tmp+1000))//���� �� ��������� ������� ��� ������� �������
				{
				//���� �������� ����
				if (time_ms%200) if (iButton())
					{
					delay_ms(100);
					call_gsm.HangUp();	//���������� � ������� �� ������ �������
					return;
					};
				}
			delay_ms(10);
			IWDG_ReloadCounter();
			
			//����������� ������
			call_stat=call_gsm.CallStatusWithAuth(number, 0, 0);
			if (call_stat!=CALL_VOICE_CONNECTING) break; //���� �������� (������� ������)
			}
		
		//���� ��������, �������
		for (s=0; s<30*60; s++)	//���� �� 30 �����
			{
			time_tmp=time_ms;
			while(time_ms<(time_tmp+1000))	//���� �� ��������� �������
				{
				if (time_ms%200) if (iButton())//���� �������� ����
					{
					delay_ms(100);
					call_gsm.HangUp();
					return;
					};
				}
			delay_ms(1);
			IWDG_ReloadCounter();
			
			gsm.ModemWakeUp();				//����� ����� (���� �� ����)
			call_stat=call_gsm.CallStatusWithAuth(number, 0, 0);
			if (call_stat!=CALL_ACTIVE_VOICE) break;
			else {SIRENA_OFF; return;}
			}
		//����������
		delay_ms(1000);
		call_gsm.HangUp();
		delay_ms(1000);
		}
	}
}



//-----------------------------------------------------------------------------------------
//�������� ��������� �������� ����� ��� ����� � �������� ����������
//-----------------------------------------------------------------------------------------
void BalanceCheck (void)
{
gsm.ModemWakeUp();		//����� ����� (���� �� ����)
//����������� ������
if (gsm.Get_CUSD(&smsbuffer[0], 320, balance_number))
	{
	//���� ����� �������
	if (smsbuffer[0]<0x20)			//� �� � ��������� UCS2, ����������� ��� � �����
		{
		gsmString2Bytes((unsigned char *)smsbuffer, (unsigned char *)smsbuffer, 320);
		gsmDecodeUcs2((unsigned char const *)smsbuffer, (unsigned char *)smsbuffer, 320);
		
		MySM_PARAM.TP_DCS = GSM_UCS2;
		}
	else MySM_PARAM.TP_DCS = GSM_7BIT;	//���� �� �� �� ��������, ������ �������� � GSM 7Bit
	
	IWDG_ReloadCounter();
	
	//�������� ���������� ����� �� ��� ������ � ������� ��������� ��������� ������ � �������
	for (char i=0; i<10; i++)
		{
		if ((phone_book[i].send_balans)&&	//��������� ������ � ������� ���������
		    (phone_book[i].phone[0]!=0))	//����� ������� � ���������� �����
			{
			if (phone_book[i].phone[0]=='+') strcpy(MySM_PARAM.TPA, &phone_book[i].phone[1]);//������� ����� �������� ����������
			else strcpy(MySM_PARAM.TPA, &phone_book[i].phone[0]);
			strcpy((char *)MySM_PARAM.TP_UD, (char const *) &smsbuffer[0]);	//��������� ����� ������ �� USSD ������
			MySM_PARAM.TP_UDlen=strlen((char *)MySM_PARAM.TP_UD);			//������ ������ 
			SendSms(&MySM_PARAM);		//�������� ���
			}
		IWDG_ReloadCounter();
		}
	}
}



void AlarmModeSet(Talarm_mode mode)
{
switch (mode)
	{
	case alarm_ready:
		alarm_mode=alarm_wait_on;
		OUT2_mode=OUT2_ON;
		SIRENA_OFF;
		LED_mode=LED_MODE_ALARM_READY;
	break;
	case alarm_off:
		alarm_mode=alarm_off;
		OUT2_mode=OUT2_OFF;
		SIRENA_OFF;
		LED_mode=LED_MODE_ALARM_OFF;
	break;
	case alarm_wait_on:
		alarm_mode=alarm_wait_on;
		OUT2_mode=OUT2_BLINK_MIDLE;
		SIRENA_OFF;
		LED_mode=LED_MODE_ALARM_WAIT;
	break;
	case alarm_trevoga:
		alarm_mode=alarm_trevoga;
		#ifdef OUT2_SIGNAL
			OUT2_mode=OUT2_BLINK_FAST;
		#elif OUT2_ZAMOK
			OUT2_mode=OUT2_ON;
		#endif
		SIRENA_ON;
		LED_mode=LED_MODE_ALARM_TREVOGA;
	break;
	case alarm_trevoga_end:
		alarm_mode=alarm_trevoga_end;
		OUT2_mode=OUT2_BLINK_FAST;
		SIRENA_OFF;
		LED_mode=LED_MODE_ALARM_TREVOGA;
	break;
	case alarm_warning_alarm:
		alarm_mode=alarm_warning_alarm;
		OUT2_mode=OUT2_BLINK_FAST;
		SIRENA_OFF;
		LED_mode=LED_MODE_ALARM_TREVOGA;
	break;
	}
}