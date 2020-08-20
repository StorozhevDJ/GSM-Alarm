/*******************************************************************************
* @file    EEPROM_Emulation/inc/eeprom.h
* @version V1.0.0
* @date    09/07/2012
* @brief   This file contains all the functions prototypes for the EEPROM 
*          emulation firmware library.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H
#define __EEPROM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported constants --------------------------------------------------------*/
/* Define the STM32F10Xxx Flash page size depending on the used STM32 device */
#if defined (STM32F10X_LD) || defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL)
  #define PAGE_SIZE  (uint16_t)0x400  // Page size = 1KByte
#elif defined (STM32F10X_HD) || defined (STM32F10X_CL)
  #define PAGE_SIZE  (uint16_t)0x800  // Page size = 2KByte
#endif

// EEPROM start address in Flash
#define EEPROM_START_ADDRESS    ((uint32_t)0x0800FC00) /* EEPROM emulation start address:
                                                  after 64KByte of used Flash memory */
#define EEPROM_END_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + (PAGE_SIZE*1)))





typedef struct
	{
	vu16 EEPROM_id;
	char password[10];		//root password
	unsigned char iButton_keys[10][8];
	unsigned char iButton_kol;
	} EEPROM_TypeDef;

extern EEPROM_TypeDef EEPROM;
extern uint16_t num;


void WriteFlash(EEPROM_TypeDef *data);	//Запись структуры во флеш
EEPROM_TypeDef ReadFlash(uint16_t num);	//Чтение структуры из флеш
uint16_t Find_end_Flash_EEPROM (void);	//Поиск последней записи во флеш

#endif /* __EEPROM_H */
/**************************** END OF FILE************************/