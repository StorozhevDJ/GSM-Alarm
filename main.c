
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
long balance_period=60*60*24*30;//30 дней по умолчанию
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
//			Таймер для отсчета времени с дискретностью 1мс
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
//			Таймер для отсчета времени с дискретностью 1мкс
//------------------------------------------------------------------------------------
void delay_mcs(int delay)
{
TIM17->CNT=1;
while (delay>(TIM17->CNT));
}



//------------------------------------------------------------------------------------
//				Программная задержка с заданным значением в мс
//------------------------------------------------------------------------------------
void delay_ms (unsigned int t)
{
timeout=t;
while (timeout);
}



//Внешние прерывания по линии 1Wire
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
//					Работа с ключем iButton
//------------------------------------------------------------------------------------
bool iButtonCheck(void)
{
unsigned int LED_mode_tmp=LED_mode;
unsigned char iButton_data[8];
if (iButton_Get_data(iButton_data)==false) return false;	//Если ключ не считан
SIRENA_ON;
delay_ms(100);
SIRENA_OFF;

//Если ни один ключ не добавлен,
if (EEPROM.iButton_kol==0)
	{
	LED_mode=0xffffffff;
	for (char i=0; i<8; i++) EEPROM.iButton_keys[0][i]=iButton_data[i];	//добавим его, он будет мастером
	EEPROM.iButton_kol=1;										//Увеличим счетчик записаных ключей
	WriteFlash(&EEPROM);										//Сохраним значения во флеш
	while (iButton_Get_data(iButton_data)) delay_ms(100);				//Ждем когда уберут ключ
	LED_mode=LED_mode_tmp;
	return true;
	}

//Если ключи есть в памяти, сравним считаный ключ со всеми записаными
for (char i=0; i<EEPROM.iButton_kol; i++)
	{
	if (iButton_compare(EEPROM.iButton_keys[i], iButton_data))
		{
		LED_mode=0xffffffff;
		//ключ найден
		if (i==0)		//Если это мастер, перейдем к управлению ключами
			{
			while (iButton_Get_data(iButton_data)) delay_ms(200);		//Ждем когда уберут ключ
			LED_mode=0x01010101;
			SIRENA_ON;
			delay_ms(500);
			SIRENA_OFF;
			while(1)
				{
				bool key_read=iButton_Get_data(iButton_data);
				delay_ms(100);
				if (key_read)				//Ключ поднесли сново
					{
					LED_mode=0xFFFFFFFF;
					SIRENA_ON;
					delay_ms(300);
					SIRENA_OFF;
					//если подносили мастер ключ
					if (iButton_compare(EEPROM.iButton_keys[0], iButton_data))
						{
						LED_mode=0xAAAAAAAA;
						SIRENA_ON;
						delay_ms(500);
						SIRENA_OFF;
						int time_tmp=time_s;
						while (iButton_Get_data(iButton_data)) delay_ms(200);//Ждем когда уберут ключ
						LED_mode=0;
						//Если мастер ключ держали 5 или более секунд
						if (time_tmp<time_s-5)
							{
							for (char j=1; j<MAX_IBUTTON_KEYS; j++)	//Очистим список ключей
								{
								for (char x=0; x<8; x++) EEPROM.iButton_keys[j][x]=0;
								}
							EEPROM.iButton_kol=1;
							}
						else//если это было короткое нажатие,
							{
							WriteFlash(&EEPROM);				//Сохраним данные во флеш
							LED_mode=LED_mode_tmp;
							return true;						//завершим управление списком
							}
						}
					//Если это не мастер, добавим его в список
					else if (EEPROM.iButton_kol<MAX_IBUTTON_KEYS-1)
						{
						for (char k=0; k<8; k++) EEPROM.iButton_keys[EEPROM.iButton_kol][k]=iButton_data[k];
						EEPROM.iButton_kol++;
						while (iButton_Get_data(iButton_data)) delay_ms(200);//Ждем когда уберут ключ
						}
					}
				IWDG_ReloadCounter();
				}
			}
		else
			{
			LED_mode=LED_mode_tmp;
			while (iButton_Get_data(iButton_data)) delay_ms(200);//Ждем когда уберут ключ
			return true;	//иначе если ключ найден и он не мастер, вернем положительный результат
			}
		}
	}
LED_mode=LED_mode_tmp;
return false;
}



//------------------------------------------------------------------------------------
//			Постановка/снятие с режима охраны по ключаму TouchMemory
//------------------------------------------------------------------------------------
bool iButton(void)
{
if (iButtonCheck())
	{
	if (alarm_mode==alarm_off)
		{
		//Постановка на охрану
		AlarmModeSet(alarm_wait_on);
		IN_OUT_OUT2_ON;
		LED_bit=0;
		
		delay_ms(1000);
		if ((!IN_OUT_IN1_ON)&&(!IN_OUT_IN2_ON)&&(!IN_OUT_PIR_ON))
			{
			//Датчики в нормальном состоянии, перейдем в режим охраны
			AlarmModeSet(alarm_ready);
			}
		AlarmSendSMS("Система встала в режим охраны по ключу", GSM_FLASH, 'i');
		}
	else
		{
		//снятие с охраны
		AlarmModeSet(alarm_off);
		delay_ms(1000);
		AlarmSendSMS("Система снята с режима охраны по ключу", GSM_FLASH, 'i');
		}
	while (iButtonCheck()) delay_ms(200);
	return true;
	}
else return false;
}



//------------------------------------------------------------------------------------
//				Проверка аккумулятора
//------------------------------------------------------------------------------------
void AKB(void)
{
AKB_voltage=BatteryCheck();
if ((AKB_voltage<AKB_LOW_LEVEL)&&(AKB_low_level==false))
	{
	AlarmSendSMS("Аккумулятор разряжен", GSM_NO_FLASH, 'i');
	AKB_low_level=true;
	OUT2_mode=OUT2_OFF;
	}
if ((AKB_voltage>AKB_NOM_LEVEL)&&(AKB_low_level))
	{
	AKB_low_level=false;
	}
if (((AKB_voltage+AKB_DELTA_V)<AKB_voltage_last)&&(power_supply))
	{
	AlarmSendSMS("Пропало внешнее питание, работа от аккумулятора", GSM_NO_FLASH, 'i');
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
	AlarmSendSMS("Подано внешнее питание", GSM_NO_FLASH, 'i');
	power_supply=true;
	AlarmModeSet(alarm_mode);//Восстанавливаем нормальные рабочие режимы индикации
	}
AKB_voltage_last=AKB_voltage;
}



//------------------------------------------------------------------------------------
//							Тревожное оповещение
//------------------------------------------------------------------------------------
void Alarm (void)
{
char x=100;
for (char i=x; i>0; i--)
	{
	delay_ms(10);	//Подождем немного
	if (IN_OUT_IN1_ON || IN_OUT_IN2_ON || IN_OUT_PIR_ON) x--;	//И убедимся, что датчик еще сработал
	else return;	//Если датчик не сработал, выйдем
	}
	
if (x==0)	//И убедимся, что датчик сработал каждый раз
	{
	AlarmModeSet(alarm_trevoga);
	
	//Рассылаем СМС оповещения с информацией о сработавшиих датчиках
	if      (IN_OUT_IN1_ON) AlarmSendSMS("Сработал охранный датчик на замыкание", GSM_NO_FLASH, 's');
	else if (IN_OUT_IN2_ON) AlarmSendSMS("Сработал охранный датчик на разрыв", GSM_NO_FLASH, 's');
	else if (IN_OUT_PIR_ON) AlarmSendSMS("Сработал встроенный датчик движения", GSM_NO_FLASH, 's');

	//if (alarm_mode==alarm_trevoga)
		{
		AlarmCall();	//Производим дозвон для возможности прослушки
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

//Настраиваем системный таймер
SysTick_Config(SystemCoreClock / 1000);

//Разрешаем тактирование таймера 17 (для задания микросекундных задержек)
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

//Чтение телефонов и параметров из записной книги
ReadPhoneBook();
IN_OUT_LED_OFF;
IWDG_ReloadCounter();

//Считываем номер СМС центра с СИМ карты
if (gsm.Get_CSCA(MySM_PARAM.SCA, 16)==false)
	{
	//Если номер СМС центра не записан на карте
	char opsos_name[10];
	if (gsm.Get_OpSoS_name(opsos_name, 10))	//Узнаем имя оператора
		{
		//Выбор СМС центра по имени оператора
		if (strcmp(opsos_name, "Beeline")==0) strcpy(MySM_PARAM.SCA, "79037011111");
		else if (strcmp(opsos_name, "MTS RUS")==0) strcpy(MySM_PARAM.SCA, "79139869990");
		else if (strcmp(opsos_name, "TELE2")==0) strcpy(MySM_PARAM.SCA, "79043290000");
		else if (strcmp(opsos_name, "MegaFon")==0) strcpy(MySM_PARAM.SCA, "79232909090");
		}
	}

//Узнаем номер для отсылки маячков в случае если нет денег
char opsos_name[10];
if (gsm.Get_OpSoS_name(opsos_name, 10))	//Узнаем имя оператора
	{
	//Выбор номера для маячка по имени оператора
	if (strcmp(opsos_name, "Beeline")==0) majachok_number=144;
	else if (strcmp(opsos_name, "MTS RUS")==0) majachok_number=116;
	else if (strcmp(opsos_name, "TELE2")==0) majachok_number=118;
	else if (strcmp(opsos_name, "MegaFon")==0) majachok_number=143;
	}

AKB_voltage_last=BatteryCheck();
if (AKB_voltage_last>1420) power_supply=true;

//Инициализация эмуляции ЕЕПРОМ
num=Find_end_Flash_EEPROM();
EEPROM=ReadFlash(num);
if (EEPROM.EEPROM_id==0xffff)//Записей нет в памяти, создадим записи по умолчанию
	{
	EEPROM.password[0]=0;
	EEPROM.iButton_keys[0][0]=0;
	EEPROM.iButton_kol=0;
	}

//Проверка замыкания перемычки для очистки памяти ключей (удаления мастера)
#ifndef DEBUG
AFIO->MAPR|=AFIO_MAPR_SWJ_CFG_DISABLE; //Отключаем линии SWD
#endif
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.GPIO_Pin=GPIO_Pin_14;
GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPD;
GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;
GPIO_Init(GPIOA, &GPIO_InitStruct);
//Проверка ноги для сброса мастер ключа
if ((GPIOA->IDR)&GPIO_IDR_IDR14)
	{
	for (char i=0; i<10; i++) for (char j=0; j<8; j++) EEPROM.iButton_keys[i][j]=0;
	EEPROM.iButton_kol=0;
	WriteFlash(&EEPROM);
	IN_OUT_LED_OFF;
	}

//gsm.SendATCmdWaitResp("AT+DDET=1", 1000, 100, "OK", 1);

//------------------------------------------------------------------------------------
//Установка первоночальных значений режима работы
alarm_mode=(Talarm_mode)(BKP_ReadBackupRegister(BKP_DR2)&0xFF);	//Читаем прошлое состояние регистра
switch (alarm_mode)					//Выставим значиения в соответствии с последним режимом работы
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
	default:							//Если до этого питание было отключено
		AlarmModeSet(alarm_wait_on);
	break;
	}
if (time_s<ALARM_TIMEOUT) time_s=ALARM_TIMEOUT;
time_alarm_timeout=time_s-ALARM_TIMEOUT;

char smsmsg[60];
#ifdef DEBUG
sprintf(smsmsg, "Устройство включено, CSR: %x, MODE: %x", RCC->CSR, BKP_ReadBackupRegister(BKP_DR2));
RCC->CSR|=RCC_CSR_RMVF;
PWR_BackupAccessCmd(ENABLE);
BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
PWR_BackupAccessCmd(DISABLE);
#else
if (alarm_mode==alarm_trevoga_end) sprintf(smsmsg, "Устройство включено", RCC->CSR);
#endif
AlarmSendSMS(smsmsg, GSM_NO_FLASH, 'i');


//--------------------------------------------------------------------------------------
//Бесконечный цикл в котором будет крутиться наша программа
//--------------------------------------------------------------------------------------
while(1)
{
//--------------------------------------------------------------------------------------
//Если имеется какоето внешнее сообщение (звонок или СМС), обработаем его
if (GSM_RING)
	{
	//SMS_protocol_check();
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (2<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	//Проверим, есть ли входящий звонок и совершим необходимые действия
	CallCheck();
	//Запоминаем состояние
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	}

//--------------------------------------------------------------------------------------
//Если повторно сработал охранный датчик по истечении защитного интервала
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
//Если впервые сработал охранный датчик
if ((IN_OUT_IN1_ON || IN_OUT_IN2_ON || IN_OUT_PIR_ON)&&
    (alarm_mode==alarm_ready))
	{
	time_alarm_timeout=time_s+5;
	AlarmModeSet(alarm_warning_alarm);
	}

//--------------------------------------------------------------------------------------
//Если с момента прошлого срабатывания прошло много времени, значит было ложное срабатывание
if ((alarm_mode==alarm_warning_alarm)&&(time_s>time_alarm_timeout+ALARM_WARNING_TIMEOUT))
	{
	AlarmModeSet(alarm_ready);
	}

//--------------------------------------------------------------------------------------
//Если оповещение о срабатывании датчика завершено, 
if (((!IN_OUT_IN1_ON)&&(!IN_OUT_IN2_ON)&&(!IN_OUT_PIR_ON))&&//ждем когда датчики прийдут в нормальное состояние
    (alarm_mode==alarm_trevoga_end)&&					//Проверяем что сигнализация уже оповестила
    (time_s>(time_alarm_timeout+ALARM_TIMEOUT)))			//И прошло 60 минут с момента срабатывания сингнализации
	{
	alarm_mode=alarm_ready;
	SIRENA_OFF;
	OUT2_mode=OUT2_ON;
	AlarmSendSMS("Система вернулась в режим охраны", GSM_FLASH, 'i');
	}

//--------------------------------------------------------------------------------------
//Постановка на охрану
if (((!IN_OUT_IN1_ON)&&(!IN_OUT_IN2_ON)&&(!IN_OUT_PIR_ON))&&//Если датчики пришли в нормальное состояние
    (alarm_mode==alarm_wait_on))						//и включен режим постановки на охрану 
	{
	alarm_mode=alarm_ready;
	SIRENA_OFF;
	OUT2_mode=OUT2_ON;
	}

//Если сирена работает больше 5 минут, выключим её
if ((alarm_mode==alarm_trevoga_end) && (time_s>(time_alarm_timeout+SIRENA_TIME_WORK))) SIRENA_OFF;

//--------------------------------------------------------------------------------------
//Если пора проверить баланс
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
//Проверим поднесли ли ключ
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
//Проверим заряд аккумулятора
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
//Сбросим ватчдог
if ((time_ms%200)==100) IWDG_ReloadCounter();

//--------------------------------------------------------------------------------------
//Если работаем от АКБ и он имеет низкий заряд, перейдем в режим низкого потребления (STOP mode)
if (AKB_low_level)
	{
	time_s=RTC_GetUnixTime();
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (100<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	MyRTC_SetAlarm(time_s+1);
	
	SIRENA_OFF;		//Выключаем сирену
	IN_OUT_LED_OFF;	//Выключаем светодиод
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (101<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);	//Засыпаем
	//Проснулись
	
	IWDG_ReloadCounter();				//Сбросим ватчдог
	SYSCLKConfig_STOP();
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (102<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	}
//Если акк имеет нормальный уровень заряда, то раз в час проверим модем на зависание
else if ((time_s%MODEM_TIME_CHECK_WORK)==0)
	{
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (1<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	gsm.ModemWakeUp();//Будим модем (если он спит)
	if (gsm.SendATCmdWaitResp("AT", 100, 100, "OK", 5)!=AT_RESP_OK) gsm.begin(115200);
	#ifdef DEBUG
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR2, (7<<8)|(alarm_mode));
	PWR_BackupAccessCmd(DISABLE);
	#endif
	}
}
}
