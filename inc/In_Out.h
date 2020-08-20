
//#define OUT2_ZAMOK
#define OUT2_SIGNAL

#define IN_OUT_LED_ON	GPIOB->BSRR=GPIO_BSRR_BS9	//Включили LED
#define IN_OUT_LED_OFF	GPIOB->BSRR=GPIO_BSRR_BR9	//Выключили LED

#define IN_OUT_OUT1_ON	GPIOA->BSRR=GPIO_BSRR_BS5	//Включили выход 1 (при сробатывании охраны)
#define IN_OUT_OUT1_OFF	GPIOA->BSRR=GPIO_BSRR_BR5	//Выключили выход1
#define IN_OUT_OUT2_ON	GPIOA->BSRR=GPIO_BSRR_BS4	//Включили выход 2 (при нахождении в охранном режиме)
#define IN_OUT_OUT2_OFF	GPIOA->BSRR=GPIO_BSRR_BR4	//Выключили выход 2

#define SIRENA_ON		IN_OUT_OUT1_ON
#define SIRENA_OFF		IN_OUT_OUT1_OFF
#define ZAMOK_ON		IN_OUT_OUT2_ON
#define ZAMOK_OFF		IN_OUT_OUT2_OFF
#define SIGNAL_ON		IN_OUT_OUT2_ON
#define SIGNAL_OFF		IN_OUT_OUT2_OFF


#define IN_OUT_IN1		(GPIOA->IDR&GPIO_IDR_IDR6)	//Смотрим что на входе 1
#define IN_OUT_IN2		(GPIOA->IDR&GPIO_IDR_IDR7)	//Смотрим что на входе 2
#define IN_OUT_PIR		(GPIOC->IDR&GPIO_IDR_IDR13)	//Смотрим что на входе PIR sensor

#define IN_OUT_IN1_ON	(IN_OUT_IN1==0)	//Смотрим что на входе 1 (датчик на замыкание)
#define IN_OUT_IN2_ON	(IN_OUT_IN2!=0)	//Смотрим что на входе 2 (датчик на разрыв)
#define IN_OUT_PIR_ON	(IN_OUT_PIR!=0)	//Смотрим что на входе PIR (датчик движения)


void In_Out_GPIO_Init(void);
void In_Out_ADC_Init(void);
void BatteryCheck_Init(void);
uint16_t BatteryCheck (void);
uint16_t Get_Temperature (void);



/*
Нужны выходы на
1. Сирена
2. Световой сигнал
3. Магнитный замок
4. Электромеханический замок
5. Светодиод на считыватель ключа
6. выход на пожарный светодиод

Входы
1. датчик движения
2. Геркон на размыкание
3. Геркон на замыкание
4. Датчик дыма
5. Датчик огня
6. Датчик воды
7. Считыватель ключа
*/