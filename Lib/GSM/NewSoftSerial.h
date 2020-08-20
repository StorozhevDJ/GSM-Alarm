/*
NewSoftSerial.h - Multi-instance software serial library
Copyright (c) 2006 David A. Mellis.  All rights reserved.
-- Interrupt-driven receive and other improvements by ladyada
-- Tuning, circular buffer, derivation from class Print,
   multi-instance support, porting to 8MHz processors,
   various optimizations, PROGMEM delay tables, inverse logic and 
   direct port writing by Mikal Hart

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at
http://arduiniana.org.
*/

#ifndef NewSoftSerial_h
#define NewSoftSerial_h

#include <inttypes.h>
#include "../Print_scan/Print.hpp"


/******************************************************************************
* Definitions
******************************************************************************/

#define GSM_PWRKEY_ON	GPIOA->BSRR=GPIO_BSRR_BS1 //Нажатие кнопки PWRKEY
#define GSM_PWRKEY_OFF	GPIOA->BSRR=GPIO_BSRR_BR1 //Отпустили кнопку PWRKEY

#define GSM_RESET_ON	GPIOA->BSRR=GPIO_BSRR_BR15//Включили RESET
#define GSM_RESET_OFF	GPIOA->BSRR=GPIO_BSRR_BS15//Отпустили RESET

#define GSM_RING		((GPIOA->IDR&GPIO_IDR_IDR12)==0) //Смотрим есть ли входящие



#define _NewSS_MAX_RX_BUFF 64 // RX buffer size
//#define _NewSS_VERSION 10 // software version of this library



/*class Print
{
public:
	virtual void print(const char *str);
	virtual void print(char *str);
	virtual void print(char ch);
	virtual void print(int data);
	virtual void print(long data);
	virtual void print(char data, char b);
	virtual inline void println(char const *str) {print(str); print('\r');};
	virtual inline void println(int data) {print(data); print('\r');};
	virtual inline void println() {print('\r');};
	
	void begin(int);
	void flush (void);
	char available(void);
	char read(void);
	
private:
	void print_str(char const str);
	void print_int(int data);
	void print_dec(int data, char b);
};*/



class NewSoftSerial : public Print
{
private:
  // per object data
  uint8_t _receivePin;
  uint8_t _receiveBitMask;
  volatile uint8_t *_receivePortRegister;
  uint8_t _transmitBitMask;
  volatile uint8_t *_transmitPortRegister;

  uint16_t _buffer_overflow:1;		//Флаг переполнения приемного буфера

  // static data
  static char _receive_buffer[_NewSS_MAX_RX_BUFF];//Приемный кольцевой буфер
  static volatile uint8_t _receive_buffer_tail;	//Номер принятого байта в кольцевом буфере
  static volatile uint8_t _receive_buffer_head;	//Номер обработанного байта в кольцевом буфере
  static NewSoftSerial *active_object;

  // private methods
  bool activate();
  
  virtual void write(uint8_t byte);
  void recv(uint8_t data);
  
  void GSM_USART_Init(void);

public:
  // public methods


	//Конструктор
  NewSoftSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);
	//Деструктор
  ~NewSoftSerial();
	//Инициализируем USART и задаем битрейт
  void begin(long speed);
	//Функция выполняющаяся при выполнении деструктора
  void end();
	//Читаем содержимое приемного буфера USART
  int read();
	//Смотрим есть ли необработанные данные в приемном кольцевом буфере
  uint8_t available(void);
  bool active() { return this == active_object; }
	//Смотрим флаг переполнения буфера
  bool overflow() { bool ret = _buffer_overflow; _buffer_overflow = false; return ret; }
	//Возвращает новер версии библиотеки
  //static int library_version() { return _NewSS_VERSION; }
	//Включение таймера
  static void enable_timer0(bool enable);
	//
  void flush();

  // public only for easy access by interrupt handlers
  static inline void GSM_USART_handle_interrupt(uint8_t data);
};

void GSM_USART_IRQHandler(void);



#endif
