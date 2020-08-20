/*******************************************************************************
* @file    EEPROM_Emulation/inc/eeprom.c
* @version V1.0.0
* @date    09/07/2012
* @brief   This file contains all the functions for the EEPROM emulation 
		 firmware library.
*******************************************************************************/

#include "stm32f10x_conf.h"
#include "eeprom.h"





EEPROM_TypeDef EEPROM;
uint16_t num;



//------------------------------------------------------------------------------
//					Запись структуры во флеш
//------------------------------------------------------------------------------
void WriteFlash(EEPROM_TypeDef *data)
{
uint32_t adr_start, adr_end;

//Начало адреса для записи данных (первый байт)
adr_start = EEPROM_START_ADDRESS+(num+1)*sizeof(*data);
//Конец адреса для записи данных (последний байт)
adr_end   = EEPROM_START_ADDRESS+(num+2)*sizeof(*data)-1;
//Если адрес для записи страници достиг конца области флеши для эмуляции EEPROM
if (adr_end>=EEPROM_END_ADDRESS)
	{
	num=0;
	adr_start = EEPROM_START_ADDRESS;
	adr_end   = EEPROM_START_ADDRESS+sizeof(*data)-1;
	data->EEPROM_id=0;
	}
else
	{
	data->EEPROM_id++;
	num++;
	}
//если не хватает места для записи в текущей странице, очистим следуюшую
if (adr_end%PAGE_SIZE<=sizeof(*data))
	{
	FLASH_Unlock();
	FLASH_ErasePage(adr_end);
	FLASH_Lock();
	}
//Запишим данные из структуры во флеш
uint16_t *ptr;
ptr=(uint16_t *) data;
FLASH_Unlock();
for (uint16_t i=0; i<sizeof(*data); i+=2)
	{
	FLASH_ProgramHalfWord(adr_start+i, *ptr++);
	}
FLASH_Lock();
}



//------------------------------------------------------------------------------
//					Чтение структуры из флеш
//------------------------------------------------------------------------------
EEPROM_TypeDef ReadFlash(uint16_t num)
{
return *(EEPROM_TypeDef *)(EEPROM_START_ADDRESS+(num*sizeof(EEPROM_TypeDef)));
}



//------------------------------------------------------------------------------
//					Поиск последней записи во флеш
//------------------------------------------------------------------------------
uint16_t Find_end_Flash_EEPROM (void)
{
EEPROM_TypeDef data_EEPROM;

int i, size;
vu16 ee_id;
data_EEPROM=ReadFlash(0);

size=(EEPROM_END_ADDRESS-EEPROM_START_ADDRESS)/sizeof(data_EEPROM);

if (data_EEPROM.EEPROM_id==(vuc16)-1) return size-1;

for (i=1; i<size; i++)
	{
	ee_id=data_EEPROM.EEPROM_id;
	data_EEPROM=ReadFlash(i);
	if (data_EEPROM.EEPROM_id==(vuc16)-1) return i-1;
	else if (data_EEPROM.EEPROM_id!=ee_id+1) return i-1;
	}
return size-1;
}
