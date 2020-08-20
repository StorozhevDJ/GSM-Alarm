
#include "stm32F10x.h"
#include "stm32f10x_conf.h"

#include "In_Out.h"


extern unsigned int timeout;


//------------------------------------------------------------------------------
//					��������� ������ �����/������
//------------------------------------------------------------------------------
void In_Out_GPIO_Init(void)
{
//��������� ������������ ���� ����� A, B
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

GPIO_InitTypeDef GPIO_Init_struct;
//����������� ���� �������� � ��������� � +Vcc
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_6;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPU;
GPIO_Init(GPIOA, &GPIO_Init_struct);
//����������� ���� �������� � ��������� � GND
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_13;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD;
GPIO_Init(GPIOC, &GPIO_Init_struct);
//����������� ����� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_5;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP;
GPIO_Init(GPIOA, &GPIO_Init_struct);
//����������� ����� ��������� ���������� ���
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN;
GPIO_Init(GPIOB, &GPIO_Init_struct);
//����������� ����� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_9;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP;
GPIO_Init(GPIOB, &GPIO_Init_struct);

//����������� ����� 1Wire
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_OD;
GPIO_Init(GPIOA, &GPIO_Init_struct);

//������ ����� - ����� iButton/In 1
EXTI_InitTypeDef EXTI_InitStruct;
EXTI_InitStruct.EXTI_Line=EXTI_Line0|EXTI_Line6;
EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Event;
EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Falling;
EXTI_InitStruct.EXTI_LineCmd=ENABLE;
EXTI_Init(&EXTI_InitStruct);
//�������� ����� - Ring/In2/PIR
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
//		������������� ���, ��� ��������� ���������� �� ������������
//------------------------------------------------------------------------------
void In_Out_ADC_Init(void)
{
//�������� ������������ ���� ���
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

//��������� ������ ���
ADC_DMACmd(ADC1, ENABLE);

//���������� ���
ADC_ResetCalibration(ADC1);
while (ADC_GetResetCalibrationStatus(ADC1));
// Start ADC1 calibration
ADC_StartCalibration(ADC1);
// Check the end of ADC1 calibration
while (ADC_GetCalibrationStatus(ADC1));
//����������� ������ �������������� ���������� ������
ADC_SoftwareStartConvCmd(ADC1,ENABLE);
}



void BatteryCheck_Init(void)
{
//����� ��0, ����� ADC8
ADC_InitTypeDef ADC_InitStructure;
//������������ ���
//��������� ������������
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
//��������� ���������������� ��������
RCC->CFGR &=~RCC_CFGR_ADCPRE;
RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2; //������ �� ������� 12MHz
//������������� ���
ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;
ADC_InitStructure.ADC_ScanConvMode=DISABLE;
ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;
ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Left;
ADC_InitStructure.ADC_NbrOfChannel=2; //���-�� ������� ���
ADC_Init(ADC1, &ADC_InitStructure);
//���������� �� �������������� �� �������� �������������
ADC_ExternalTrigConvCmd(ADC1, ENABLE);
//����� �����������
ADC_AnalogWatchdogThresholdsConfig(ADC1,0xFFFF,0x0000);
//ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE); //���������� �� ���������� AWD
//��������� �������
ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);

ADC_TempSensorVrefintCmd(ENABLE);
// Enable ADC1
ADC_Cmd(ADC1, ENABLE);
//���������� ���
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
//��������� ������
ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);

//����������� ������ �������������� ���������� ������
ADC_SoftwareStartConvCmd(ADC1,ENABLE);

timeout=1000;
while (ADC_GetSoftwareStartConvStatus(ADC1)) if (timeout==0) return 0;

return ((ADC_GetConversionValue(ADC1)*1000)/14327);
}



uint16_t Get_Temperature (void)
{
//��������� ������
//ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 1, ADC_SampleTime_239Cycles5);
ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_239Cycles5);

//����������� ������ �������������� ���������� ������
ADC_SoftwareStartConvCmd(ADC1,ENABLE);

timeout=1000;
while (ADC_GetSoftwareStartConvStatus(ADC1)) if (timeout==0) return 0;

return ADC_GetConversionValue(ADC1);
}