
#include "stm32F10x.h"
#include "stm32f10x_conf.h"

#include "In_Out.h"


extern unsigned int timeout;


//------------------------------------------------------------------------------
//					Настройка портов ввода/вывода
//------------------------------------------------------------------------------
void In_Out_GPIO_Init(void)
{
//Разрешаем тактирование шины порта A, B
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

GPIO_InitTypeDef GPIO_Init_struct;
//Настраеваем вход датчиков с подтяжкой к +Vcc
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_6;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPU;
GPIO_Init(GPIOA, &GPIO_Init_struct);
//Настраеваем вход датчиков с подтяжкой к GND
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_13;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD;
GPIO_Init(GPIOC, &GPIO_Init_struct);
//Настраеваем выход управления
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_5;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP;
GPIO_Init(GPIOA, &GPIO_Init_struct);
//Настраеваем входы измерения напряжения АКБ
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN;
GPIO_Init(GPIOB, &GPIO_Init_struct);
//Настраеваем выход светодиода
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_9;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP;
GPIO_Init(GPIOB, &GPIO_Init_struct);

//Настраеваем вывод 1Wire
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_OD;
GPIO_Init(GPIOA, &GPIO_Init_struct);

//Задний фронт - Линии iButton/In 1
EXTI_InitTypeDef EXTI_InitStruct;
EXTI_InitStruct.EXTI_Line=EXTI_Line0|EXTI_Line6;
EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Event;
EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Falling;
EXTI_InitStruct.EXTI_LineCmd=ENABLE;
EXTI_Init(&EXTI_InitStruct);
//Передний фронт - Ring/In2/PIR
EXTI_InitStruct.EXTI_Line=EXTI_Line17|EXTI_Line7|EXTI_Line13;
EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Event;
EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Rising;
EXTI_InitStruct.EXTI_LineCmd=ENABLE;
EXTI_Init(&EXTI_InitStruct);
/*
EXTI_InitStruct.EXTI_Line=EXTI_Line0;
EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Falling;
EXTI_InitStruct.EXTI_LineCmd=ENABLE;
EXTI_Init(&EXTI_InitStruct);

GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
NVIC_InitTypeDef NVIC_InitStructure;
NVIC_InitStructure.NVIC_IRQChannel=EXTI0_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
NVIC_Init(&NVIC_InitStructure);*/
//EXTI_GenerateSWInterrupt(EXTI_Line0);
}



//------------------------------------------------------------------------------
//		Инициализация АЦП, для измерения напряжения на аккумуляторе
//------------------------------------------------------------------------------
void In_Out_ADC_Init(void)
{
//Включаем тактирование шины АЦП
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

//
ADC_InitTypeDef ADC_Init_struct;
ADC_Init_struct.ADC_Mode=ADC_Mode_Independent;
ADC_Init_struct.ADC_ScanConvMode=DISABLE;
ADC_Init_struct.ADC_ContinuousConvMode=ENABLE;
ADC_Init_struct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
ADC_Init_struct.ADC_DataAlign=ADC_DataAlign_Left;
ADC_Init_struct.ADC_NbrOfChannel=1;

ADC_Init(ADC1, &ADC_Init_struct);

//Разрешаем работу АЦП
ADC_DMACmd(ADC1, ENABLE);

//Калибровка АЦП
ADC_ResetCalibration(ADC1);
while (ADC_GetResetCalibrationStatus(ADC1));
// Start ADC1 calibration
ADC_StartCalibration(ADC1);
// Check the end of ADC1 calibration
while (ADC_GetCalibrationStatus(ADC1));
//Программный запуск преобразования регулирной группы
ADC_SoftwareStartConvCmd(ADC1,ENABLE);
}



void BatteryCheck_Init(void)
{
//вывод РВ0, канал ADC8
ADC_InitTypeDef ADC_InitStructure;
//Конфигурация АЦП
//Разрешаем тактирование
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
//Установка предварительного делителя
RCC->CFGR &=~RCC_CFGR_ADCPRE;
RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2; //Работа на частоте 12MHz
//Инициализация АЦП
ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;
ADC_InitStructure.ADC_ScanConvMode=DISABLE;
ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;
ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Left;
ADC_InitStructure.ADC_NbrOfChannel=2; //кол-во каналов АЦП
ADC_Init(ADC1, &ADC_InitStructure);
//Разрешение на преобразование от внешнего переключателя
ADC_ExternalTrigConvCmd(ADC1, ENABLE);
//Режим компаратора
ADC_AnalogWatchdogThresholdsConfig(ADC1,0xFFFF,0x0000);
//ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE); //Разрешение по прерыванию AWD
//Настройка каналов
ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);

ADC_TempSensorVrefintCmd(ENABLE);
// Enable ADC1
ADC_Cmd(ADC1, ENABLE);
//Калибровка АЦП
ADC_ResetCalibration(ADC1);
timeout=1000;
while (ADC_GetResetCalibrationStatus(ADC1)) if (timeout==0) return;
// Start ADC1 calibration
ADC_StartCalibration(ADC1);
// Check the end of ADC1 calibration
while (ADC_GetCalibrationStatus(ADC1)) if (timeout==0) return;
}



uint16_t BatteryCheck (void)
{
//Настройка канала
ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);

//Программный запуск преобразования регулирной группы
ADC_SoftwareStartConvCmd(ADC1,ENABLE);

timeout=1000;
while (ADC_GetSoftwareStartConvStatus(ADC1)) if (timeout==0) return 0;

return ((ADC_GetConversionValue(ADC1)*1000)/14327);
}



uint16_t Get_Temperature (void)
{
//Настройка канала
//ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 1, ADC_SampleTime_239Cycles5);
ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_239Cycles5);

//Программный запуск преобразования регулирной группы
ADC_SoftwareStartConvCmd(ADC1,ENABLE);

timeout=1000;
while (ADC_GetSoftwareStartConvStatus(ADC1)) if (timeout==0) return 0;

return ADC_GetConversionValue(ADC1);
}