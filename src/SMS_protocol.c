
#include "main.h"

#include "EE_emul\EEPROM.h"

#include "Print_Scan\Parser.h"

#include "GSM_GPRS\SIM900.h"
#include "GSM_GPRS\call.h"
#include "GSM_GPRS\sms.h"
#include "GSM_GPRS\GSM.h"

#include "PDU/pdu.h"

#include "SMS_protocol.h"



SMSGSM sms;

SM_PARAM MySM_PARAM;
char smsbuffer[320];

long sms_new_timeout;

extern Tphone_book phone_book[];



char SendSms(SM_PARAM *MySM_PARAM)
{
unsigned char pDst[361];
char cmd[20];

gsm.ModemWakeUp();//Будим модем (если он спит)

sprintf(cmd, "AT+CMGS=%d\r\n", gsmEncodePdu( MySM_PARAM, (char *) pDst ));

if (gsm.SendATCmdWaitResp(cmd, 1500, 200, ">", 1))
	{
	gsm.SendATCmdWaitResp((char const *)pDst, 1000, 100, "\r", 1);
	if (gsm.WaitResp(20000, 1000, "OK")==RX_FINISHED_STR_RECV) return 0;
	}
delay_ms(3000);
return 1;
}



//***************************************************************************************
//							SMS уведомления
//***************************************************************************************
void AlarmSendSMS(char * str, char flash_type, char sms_type)
{
strcpy ((char *)MySM_PARAM.TP_UD, str);
MySM_PARAM.TP_DCS = GSM_UCS2|flash_type;

gsm.ModemWakeUp();//Будим модем (если он спит)
if (gsm.CheckRegistration()==REG_REGISTERED)
//Отправим оповещение о срабатывании датчика на все номера которым это требуется
for (char i=0; i<10; i++)
	{
	if (phone_book[i].phone[0]!=0)	//Номер записан в телефонной книге
		{
		if (((phone_book[i].sms_enable)&&(sms_type=='s'))||	//Отправка охранного уведомления по смс разрешена
		    ((phone_book[i].sms_info_enable)&&(sms_type=='i')))//Или отправка информационного уведомления по смс разрешена
			{
			if (phone_book[i].phone[0]=='+') strcpy(MySM_PARAM.TPA, &phone_book[i].phone[1]);//Добавим номер телефона получателя
			else strcpy(MySM_PARAM.TPA, &phone_book[i].phone[0]);
			MySM_PARAM.TP_UDlen=strlen((char const *) MySM_PARAM.TP_UD);//Укажим длинну сообщения
			IWDG_ReloadCounter();
			if (iButton()) return;
			//Отправим СМС
			if (SendSms(&MySM_PARAM))
				{
				IWDG_ReloadCounter();
				SendMajachok(&phone_book[i].phone[0], 116);
				}
			}
		IWDG_ReloadCounter();
		}
	}
}



bool SMS_protocol_check (void)
{
//Проверим наличие СМС
char position;
position=sms.Is_PDU_SMSPresent(SMS_ALL);
if (position==0) return false;
while (position>0)	//обработаем все принятые смс
	{
	//Читаем СМС из памяти
	if (gsm.readSMS_PDU(&smsbuffer[0], 320, position))
		{
		//Преобразуем СМС из формата PDU в текстовый
		gsmDecodePdu((unsigned char *)smsbuffer, &MySM_PARAM);
		//обработаем содержимое СМС
		//Проверим наличие пароля
		if (MySM_PARAM.TP_UD[0]=='!')	//Пароль указан
			{
			char pas[9];
			sscanf((const char *)MySM_PARAM.TP_UD, "!%8[^\n]", pas);
			
			if ((EEPROM.password[0]==0xff)||(EEPROM.password[0]==0x00))	//но не задан, 
				{
				strcpy(EEPROM.password, (const char *)&pas[0]);	//значит зададим его
				WriteFlash(&EEPROM);
				}
			if (strcmp(&pas[0], EEPROM.password)==0)		//Если пароль совпал
				{
				char cmd[5];
				char data[32];
				sscanf((const char *)MySM_PARAM.TP_UD, "%*[^\n]\n%4[^=]=%31[^\n]", cmd, data);
				if (cmd[0]=='t')						//Команда действия с телефонными номерами
					{
					if (cmd[1]=='?')					//Запрос списка номеров
						{
						
						}
					else	if ((cmd[1]<=0x30)&&(cmd[1]>=0x39))//Добавление/удаление номера
						{
						
						}
					}
				}
			}
		}
	sms.DeleteSMS(position);
	position=sms.Is_PDU_SMSPresent(SMS_UNREAD);
	}
return true;
}



void SendMajachok(char * phone_num, char majachok_number)
{
char CUSD_num[20];
sprintf(CUSD_num, "*%d*%s#", majachok_number, &phone_num[1]);

gsm.ModemWakeUp();//Будим модем (если он спит)
gsm.Send_CUSD("", 0, CUSD_num);
}