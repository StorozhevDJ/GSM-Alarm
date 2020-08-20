/**
  ******************************************************************************
  * @file    main.h
  * @author  Storozhev Denis
  * @version V1.4.0
  * @date    15-April-2013
  * @brief   main config file.
  * Update: Добавил задержку, для проверки на ложное срабатывание
  * 		(защита от ложного срабатывания PIR датчика)
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


#define SIRENA_TIME_WORK		(5*60)		//Мах время работы сирены после срабатывания датчиков
#define ALARM_TIMEOUT		(60*60)		//Интервал времени по оконччании которого система автоматически возвращается в режим охраны из режима тревоги
#define ALARM_WARNING_TIMEOUT	(5*60)		//Обнаружена активность, проверяем было ли это ложное срабатывание
#define AKB_CHECK_TIME		(1*60)		//Интервал времени для проверки аккумулятора
#define AKB_LOW_LEVEL		(1200)		//Порог низкого напряжения АКБ (разряжен)
#define AKB_NOM_LEVEL		(1300)		//Порог нормального напряжения АКБ
#define AKB_DELTA_V			(20)			//Порог изменения напряжения на АКБ за минуту для изменения статутса заряд/разряд

#define MODEM_TIME_CHECK_WORK	(60*60)		//Интервал проверки модема на зависания



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
	alarm_ready=1,		//Режим охраны включен
	alarm_off,		//Режим охраны выключен
	alarm_warning_alarm,//Режим охраны включен, сработал датчик, проверка на ложное срабатывание
	alarm_trevoga,		//Сработал охранный датчик, режим оповещения
	alarm_trevoga_end,	//Оповещение завершино, ждем выключения охранного датчика, либо выключения с режима охраны
	alarm_wait_on		//Ожидание включения системы в режим охраны (ожидание прихода датчиков в норм состояние
	};

extern unsigned long time_ms;
extern unsigned int  timeout;
extern unsigned long time_s;

void Timer_1ms(void);
void delay_mcs(int delay);
void delay_ms (unsigned int t);

bool iButtonCheck(void);

bool iButton(void);
