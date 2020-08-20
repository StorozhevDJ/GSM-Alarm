
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"

#include "RTC\RTC.h"

#include "In_Out.h"

#include "EE_emul\EEPROM.h"

#include "Print_Scan\Parser.h"

#include "GSM_GPRS\SIM900.h"

#include "PDU/pdu.h"
#include "SMS_protocol.h"

#include "MyGSM.h"

#include "iButton/iButton.h"

//My +7-968-105-52-93


Tphone_book phone_book[10];

Talarm_mode alarm_mode;

char balance_number;
long balance_period=60*60*24*30;//30 ���� �� ���������
char majachok_number;

unsigned char LED_bit;
unsigned int LED_mode=LED_MODE_ALARM_WAIT;
unsigned int OUT2_mode;

unsigned int AKB_voltage;
unsigned int AKB_voltage_last;
bool AKB_low_level=false;
bool power_supply=false;

unsigned int timeout;
unsigned long time_ms;
unsigned long time_s;

unsigned long time_check_bal;
unsigned long time_check_AKB;
unsigned long time_check_iButton;
unsigned long time_alarm_timeout;



//------------------------------------------------------------------------------------
//			������ ��� ������� ������� � ������������� 1��
//------------------------------------------------------------------------------------
void Timer_1ms(void)
{
if (timeout) timeout--;

if (((++time_ms)%1000)==0)
	{
	time_s++;
	}
if ((time_ms%200)==0)
	{
	if (LED_bit>=30) LED_bit=0; else LED_bit++;
	if (LED_mode&(1<<LED_bit)) IN_OUT_LED_ON; else IN_OUT_LED_OFF;
	if (OUT2_mode&(1<<LED_bit)) IN_OUT_OUT2_ON; else IN_OUT_OUT2_OFF;
	}
}



//------------------------------------------------------------------------------------
//			������ ��� ������� ������� � ������������� 1���
//------------------------------------------------------------------------------------
void delay_mcs(int delay)
{
TIM17->CNT=1;
while (delay>(TIM17->CNT));
}



//------------------------------------------------------------------------------------
//				����������� �������� � �������� ��������� � ��
//------------------------------------------------------------------------------------
void delay_ms (unsigned int t)
{
timeout=t;
while (timeout);
}



//������� ���������� �� ����� 1Wire
void OneWire_irqHandler (void)
{

}



//------------------------------------------------------------------------------------
// @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
//   and select PLL as system clock source.
// @param  None
// @retval None
//------------------------------------------------------------------------------------
void SYSCLKConfig_STOP(void)
{
ErrorStatus HSEStartUpStatus;
/* Enable HSE */
RCC_HSEConfig(RCC_HSE_ON);
/* Wait till HSE is ready */
HSEStartUpStatus = RCC_WaitForHSEStartUp();

if(HSEStartUpStatus == SUCCESS)
	{
	// Enable PLL
	RCC_PLLCmd(ENABLE);
	// Wait till PLL is ready
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET){}
	// Select PLL as system clock source
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	// Wait till PLL is used as system clock source
	while(RCC_GetSYSCLKSource() != 0x08){}
	}
}



//------------------------------------------------------------------------------------
//					������ � ������ iButton
//------------------------------------------------------------------------------------
bool iButtonCheck(void)
{
unsigned int LED_mode_tmp=LED_mode;
unsigned char iButton_data[8];
if (iButton_Get_data(iButton_data)==false) return false;	//���� ���� �� ������
SIRENA_ON;
delay_ms(100);
SIRENA_OFF;

//���� �� ���� ���� �� ��������,
if (EEPROM.iButton_kol==0)
	{
	LED_mode=0xffffffff;
	for (char i=0; i<8; i++) EEPROM.iButton_keys[0][i]=iButton_data[i];	//������� ���, �� ����� ��������
	EEPROM.iButton_kol=1;										//�������� ������� ��������� ������
	WriteFlash(&EEPROM);										//�������� �������� �� ����
	while (iButton_Get_data(iButton_data)) delay_ms(100);				//���� ����� ������ ����
	LED_mode=LED_mode_tmp;
	return true;
	}

//���� ����� ���� � ������, ������� �������� ���� �� ����� ����������
for (char i=0; i<EEPROM.iButton_kol; i++)
	{
	if (iButton_compare(EEPROM.iButton_keys[i], iButton_data))
		{
		LED_mode=0xffffffff;
		//���� ������
		if (i==0)		//���� ��� ������, �������� � ���������� �������
			{
			while (iButton_Get_data(iButton_data)) delay_ms(200);		//���� ����� ������ ����
			LED_mode=0x01010101;
			SIRENA_ON;
			delay_ms(500);
			SIRENA_OFF;
			while(1)
				{
				bool key_read=iButton_Get_data(iButton_data);
				delay_ms(100);
				if (key_read)				//���� �������� �����
					{
					LED_mode=0xFFFFFFFF;
					SIRENA_ON;
					delay_ms(300);
					SIRENA_OFF;
					//���� ��������� ������ ����
					if (iButton_compare(EEPROM.iButton_keys[0], iButton_data))
						{
						LED_mode=0xAAAAAAAA;
						SIRENA_ON;
						delay_ms(500);
						SIRENA_OFF;
						int time_tmp=time_s;
						while (iButton_Get_data(iButton_data)) delay_ms(200);//���� ����� ������ ����
						LED_mode=0;
						//���� ������ ���� ������� 5 ��� ����� ������
						if (time_tmp<time_s-5)
							{
							for (char j=1; j<MAX_IBUTTON_KEYS; j++)	//������� ������ ������
								{
								for (char x=0; x<8; x++) EEPROM.iButton_keys[j][x]=0;
								}
							EEPROM.iButton_kol=1;
							}
						else//���� ��� ���� �������� �������,
							{
							WriteFlash(&EEPROM);				//�������� ������ �� ����
							LED_mode=LED_mode_tmp;
							return true;						//�������� ���������� �������
							}
						}
					//���� ��� �� ������, ������� ��� � ������
					else if (EEPROM.iButton_kol<MAX_IBUTTON_KEYS-1)
						{
						for (char k=0; k<8; k++) EEPROM.iButton_keys[EEPROM.iButton_kol][k]=iButton_data[k];
						EEPROM.iButton_kol++;
						while (iButton_Get_data(iButton_data)) delay_ms(200);//���� ����� ������ ����
						}
					}
				IWDG_ReloadCounter();
				}
			}
		else
			{
			LED_mode=LED_mode_tmp;
			while (iButton_Get_data(iButton_data)) delay_ms(200);//���� ����� ������ ����
			return true;	//����� ���� ���� ������ � �� �� ������, ������ ������������� ���������
			}
		}
	}
LED_mode=LED_mode_tmp;
return false;
}



//------------------------------------------------------------------------------------
//			����������/������ � ������ ������ �� ������� TouchMemory
//------------------------------------------------------------------------------------
bool iButton(void)
{
if (iButtonCheck())
	{
	if (alarm_mode==alarm_off)
		{
		//���������� �� ������
		AlarmModeSet(alarm_wait_on);
		IN_OUT_OUT2_ON;
		LED_bit=0;
		
		delay_ms(1000);
		if ((!IN_OUT_IN1_ON)&&(!IN_OUT_IN2_ON)&&(!IN_OUT_PIR_ON))
			{
			//������� � ���������� ���������, �������� � ����� ������
			AlarmModeSet(alarm_ready);
			}
		AlarmSendSMS("������� ������ � ����� ������ �� �����", GSM_FLASH, 'i');
		}
	else
		{
		//������ � ������
		AlarmModeSet(alarm_off);
		delay_ms(1000);
		AlarmSendSMS("������� ����� � ������ ������ �� �����", GSM_FLASH, 'i');
		}
	while (iButtonCheck()) delay_ms(200);
	return true;
	}
else return false;
}



//------------------------------------------------------------------------------------
//				�������� ������������
//------------------------------------------------------------------------------------
void AKB(void)
{
AKB_voltage=BatteryCheck();
if ((AKB_voltage<AKB_LOW_LEVEL)&&(AKB_low_level==false))
	{
	AlarmSendSMS("����������� ��������", GSM_NO_FLASH, 'i');
	AKB_low_level=true;
	OUT2_mode=OUT2_OFF;
	}
if ((AKB_voltage>AKB_NOM_LEVEL)&&(AKB_low_level))
	{
	AKB_low_level=false;
	}
if (((AKB_voltage+AKB_DELTA_V)<AKB_voltage_last)&&(power_supply))
	{
	AlarmSendSMS("������� ������� �������, ������ �� ������������", GSM_NO_FLASH, 'i');
	power_supply=false;
	SIRENA_OFF;
	#ifdef OUT2_SIGNAL
	if (alarm_mode==alarm_off) OUT2_mode=OUT2_OFF;
	else OUT2_mode=OUT2_BLINK_LOW;
	#elif OUT2_ZAMOK
	OUT2_mode=OUT2_ON;
	#endif
	}
if ((AKB_voltage>AKB_voltage_last+AKB_DELTA_V)&&(power_supply==false))
	{
	AlarmSendSMS("������ ������� �������", GSM_NO_FLASH, 'i');
	power_supply=true;
	AlarmModeSet(alarm_mode);//��������������� ���������� ������� ������ ���������
	}
AKB_voltage_last=AKB_voltage;
}



//------------------------------------------------------------------------------------
//							��������� ����������
//------------------------------------------------------------------------------------
void Alarm (void)
{
char x=100;
for (char i=x; i>0; i--)
	{
	delay_ms(10);	//�������� �������
	if (IN_OUT_IN1_ON || IN_OUT_IN2_ON || IN_OUT_PIR_ON) x--;	//� ��������, ��� ������ ��� ��������
	else return;	//���� ������ �� ��������, ������
	}
	
if (x==0)	//� ��������, ��� ������ �������� ������ ���
	{
	AlarmModeSet(alarm_trevoga);
	
	//��������� ��� ���������� � ����������� � ������������ ��������
	if      (IN_OUT_IN1_ON) AlarmSendSMS("�������� �������� ������ �� ���������", GSM_NO_FLASH, 's');
	else if (IN_OUT_IN2_ON) AlarmSendSMS("�������� �������� ������ �� ������", GSM_NO_FLASH, 's');
	else if (IN_OUT_PIR_ON) AlarmSendSMS("�������� ���������� ������ ��������", GSM_NO_FLASH, 's');

	//if (alarm_mode==alarm_trevoga)
		{
		AlarmCall();	//���������� ������ ��� ����������� ���������
		AlarmModeSet(alarm_trevoga_end);
		}
	}
}



void Save_BKP_state(short alarm_mode_debug, char alarm_mode_bkp)
	{
	
	}



/***************************************************************************
*							Main								*
***************************************************************************/
void main(void)
{
#ifndef DEBUG
if (FLASH_GetReadOutProtectionStatus()==RESET) FLASH_ReadOutProtection(ENABLE);
#endif

In_Out_GPIO_Init();

//����������� ��������� ������
SysTick_Config(SystemCoreClock / 1000);

//��������� ������������ ������� 17 (��� ������� �������������� ��������)
RCC->APB2ENR|= RCC_APB2ENR_TIM17EN;
TIM17->PSC = (SystemCoreClock/1000000) - 1;	//The counter clock frequency is equal to fCK_PSC/(PSC[15:0]+1).
TIM17->ARR = 65500 - 1;		//ARR is the value to be loaded in the actual auto-reload register
TIM17->CR1 = (TIM_CR1_CEN);	//CEN:  Counter enabled

delay_ms(5000);

RTC_Init();
time_s=RTC_GetUnixTime();
time_check_bal=time_s+balance_period;

//Watchdog Init
IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
IWDG_SetPrescaler(IWDG_Prescaler_256);
IWDG_ReloadCounter();
//#ifndef DEBUG
//IWDG_Enable();
//#endif
IWDG_ReloadCounter();

//iButton Init peripheral
iButton_init();

BatteryCheck_Init();
BatteryCheck();

IN_OUT_LED_OFF;

delay_ms(5000);
IN_OUT_LED_OFF;

IWDG_ReloadCounter();
//GSM module power on
gsm.begin(115200);
IN_OUT_LED_OFF;
IWDG_ReloadCounter();

//������ ��������� � ���������� �� �������� �����
ReadPhoneBook();
IN_OUT_LED_OFF;
IWDG_ReloadCounter();

//��������� ����� ��� ������ � ��� �����
if (gsm.Get_CSCA(MySM_PARAM.SCA, 16)==false)
	{
	//���� ����� ��� ������ �� ������� �� �����
	char opsos_name[10];
	if (gsm.Get_OpSoS_name(opsos_name, 10))	//������ ��� ���������
		{
		//����� ��� ������ �� ����� ���������
		if (strcmp(opsos_name, "Beeline")==0) strcpy(MySM_PARAM.SCA, "79037011111");
		else if (strcmp(opsos_name, "MTS RUS")==0) strcpy(MySM_PARAM.SCA, "79139869990");
		else if (strcmp(opsos_name, "TELE2")==0) strcpy(MySM_PARAM.SCA, "79043290000");
		else if (strcmp(opsos_name, "MegaFon")==0) strcpy(MySM_PARAM.SCA, "79232909090");
		}
	}

//������ ����� ��� ������� ������� � ������ ���� ��� �����
char opsos_name[10];
if (gsm.Get_OpSoS_name(opsos_name, 10))	//������ ��� ���������
	{
	//����� ������ ��� ������ �� ����� ���������
	if (strcmp(opsos_name, "Beeline")==0) majachok_number=144;
	else if (strcmp(opsos_name, "MTS RUS")==0) majachok_number=116;
	else if (strcmp(opsos_name, "TELE2")==0) majachok_number=118;
	else if (strcmp(opsos_name, "MegaFon")==0) majachok_number=143;
	}

AKB_voltage_last=BatteryCheck();
if (AKB_voltage_last>1420) power_supply=true;

//������������� �������� ������
num=Find_end_Flash_EEPROM();
EEPROM=ReadFlash(num);
if (EEPROM.EEPROM_id==0xffff)//������� ��� � ������, �������� ������ �� ���������
	{
	EEPROM.password[0]=0;
	EEPROM.iButton_keys[0][0]=0;
	EEPROM.iButton_kol=0;
	}

//�������� ��������� ��������� ��� ������� ������ ������ (�������� �������)
#ifndef DEBUG
AFIO->MAPR|=AFIO_MAPR_SWJ_CFG_DISABLE; //��������� ����� SWD
#endif
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.GPIO_Pin=GPIO_Pin_14;
GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPD;
GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;
GPIO_Init(GPIOA, &GPIO_InitStruct);
//�������� ���� ��� ������ ������ �����
if ((GPIOA->IDR)&GPIO_IDR_IDR14)
	{
	for (char i=0; i<10; i++) for (char j=0; j<8; j++) EEPROM.iButton_keys[i][j]=0;
	EEPROM.iButton_kol=0;
	WriteFlash(&EEPROM);
	IN_OUT_LED_OFF;
	}

//gsm.SendATCmdWaitResp("AT+DDET=1", 1000, 100, "OK", 1);

//------------------------------------------------------------------------------------
//��������� �������������� �������� ������ ������
alarm_mode=(Talarm_mode)(BKP_ReadBackupRegister(BKP_DR2)&0xFF);	//������ ������� ��������� ��������
switch (alarm_mode)					//�������� ��������� � ������������ � ��������� ������� ������
	{
	case alarm_ready:
		AlarmModeSet(alarm_ready);
	break;
	case alarm_off:
		AlarmModeSet(alarm_off);
	break;
	case alarm_trevoga:
		AlarmModeSet(alarm_trevoga_end);
	case alarm_trevoga_end:
		AlarmModeSet(alarm_trevoga_end);
	break;
	case alarm_wait_on:
		AlarmModeSet(alarm_wait_on);
	break;
	default:							//���� �� ����� ������� ���� ���������
		AlarmModeSet(alarm_wait_on);
	break;
	}
if (time_s<ALARM_TIMEOUT) time_s=ALARM_TIMEOUT;
time_alarm_timeout=time_s-ALARM_TIMEOUT;

char smsmsg[60];
#ifdef DEBUG
sprintf(smsmsg, "���������� ��������, CSR: %x, MODE: %x", RCC->CSR, BKP_ReadBackupRegister(BKP_DR2));
RCC->CSR|=RCC_CSR_RMVF;
PWR_BackupAccessCmd(ENABLE);
BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
PWR_BackupAccessCmd(DISABLE);
#else
if (alarm_mode==alarm_trevoga_end) sprintf(smsmsg, "���������� ��������", RCC->CSR);
#endif
AlarmSendSMS(smsmsg, GSM_NO_FLASH, 'i');


//--------------------------------------------------------------------------------------
//����������� ���� � ������� ����� ��������� ���� ���������
//--------------------------------------------------------------------------------------
while(1)
{
//--------------------------------------------------------------------------------------
//���� ������� ������� ������� ��������� (������ ��� ���), ���������� ���
if (GSM_RING)
	{
	//SMS_protocol_check();
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (2<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	//��������, ���� �� �������� ������ � �������� ����������� ��������
	CallCheck();
	//���������� ���������
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	}

//--------------------------------------------------------------------------------------
//���� �������� �������� �������� ������ �� ��������� ��������� ���������
if ((IN_OUT_IN1_ON || IN_OUT_IN2_ON || IN_OUT_PIR_ON)&&
    (alarm_mode==alarm_warning_alarm)&&
    (time_s>time_alarm_timeout))
	{
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (3<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	Alarm();
	
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	}

//--------------------------------------------------------------------------------------
//���� ������� �������� �������� ������
if ((IN_OUT_IN1_ON || IN_OUT_IN2_ON || IN_OUT_PIR_ON)&&
    (alarm_mode==alarm_ready))
	{
	time_alarm_timeout=time_s+5;
	AlarmModeSet(alarm_warning_alarm);
	}

//--------------------------------------------------------------------------------------
//���� � ������� �������� ������������ ������ ����� �������, ������ ���� ������ ������������
if ((alarm_mode==alarm_warning_alarm)&&(time_s>time_alarm_timeout+ALARM_WARNING_TIMEOUT))
	{
	AlarmModeSet(alarm_ready);
	}

//--------------------------------------------------------------------------------------
//���� ���������� � ������������ ������� ���������, 
if (((!IN_OUT_IN1_ON)&&(!IN_OUT_IN2_ON)&&(!IN_OUT_PIR_ON))&&//���� ����� ������� ������� � ���������� ���������
    (alarm_mode==alarm_trevoga_end)&&					//��������� ��� ������������ ��� ����������
    (time_s>(time_alarm_timeout+ALARM_TIMEOUT)))			//� ������ 60 ����� � ������� ������������ �������������
	{
	alarm_mode=alarm_ready;
	SIRENA_OFF;
	OUT2_mode=OUT2_ON;
	AlarmSendSMS("������� ��������� � ����� ������", GSM_FLASH, 'i');
	}

//--------------------------------------------------------------------------------------
//���������� �� ������
if (((!IN_OUT_IN1_ON)&&(!IN_OUT_IN2_ON)&&(!IN_OUT_PIR_ON))&&//���� ������� ������ � ���������� ���������
    (alarm_mode==alarm_wait_on))						//� ������� ����� ���������� �� ������ 
	{
	alarm_mode=alarm_ready;
	SIRENA_OFF;
	OUT2_mode=OUT2_ON;
	}

//���� ������ �������� ������ 5 �����, �������� �
if ((alarm_mode==alarm_trevoga_end) && (time_s>(time_alarm_timeout+SIRENA_TIME_WORK))) SIRENA_OFF;

//--------------------------------------------------------------------------------------
//���� ���� ��������� ������
if (time_s>=time_check_bal)
	{
	time_check_bal=time_s+balance_period;
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (4<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	BalanceCheck();
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	}

//--------------------------------------------------------------------------------------
//�������� �������� �� ����
if (time_ms>=time_check_iButton)
	{
	time_check_iButton=time_ms+200;
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (5<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	iButton();
	
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	}

//--------------------------------------------------------------------------------------
//�������� ����� ������������
if (time_s>=time_check_AKB)
	{
	time_check_AKB=time_s+AKB_CHECK_TIME;
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (6<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	AKB();
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	}

//--------------------------------------------------------------------------------------
//������� �������
if ((time_ms%200)==100) IWDG_ReloadCounter();

//--------------------------------------------------------------------------------------
//���� �������� �� ��� � �� ����� ������ �����, �������� � ����� ������� ����������� (STOP mode)
if (AKB_low_level)
	{
	time_s=RTC_GetUnixTime();
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (100<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	MyRTC_SetAlarm(time_s+1);
	
	SIRENA_OFF;		//��������� ������
	IN_OUT_LED_OFF;	//��������� ���������
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (101<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);	//��������
	//����������
	
	IWDG_ReloadCounter();				//������� �������
	SYSCLKConfig_STOP();
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (102<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	}
//���� ��� ����� ���������� ������� ������, �� ��� � ��� �������� ����� �� ���������
else if ((time_s%MODEM_TIME_CHECK_WORK)==0)
	{
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	gsm.ModemWakeUp();//����� ����� (���� �� ����)
	if (gsm.SendATCmdWaitResp("AT", 100, 100, "OK", 5)!=AT_RESP_OK) gsm.begin(115200);
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (7<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	}
}
}
