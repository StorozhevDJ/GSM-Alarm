/*
NewSoftSerial.cpp - Multi-instance software serial library
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


// 
// Includes
// 
#include <string.h>



#include "NewSoftSerial.h"

#include "stm32F10x.h"




//
// Statics
//
NewSoftSerial *NewSoftSerial::active_object = 0;
char NewSoftSerial::_receive_buffer[_NewSS_MAX_RX_BUFF]; 
volatile uint8_t NewSoftSerial::_receive_buffer_tail = 0;
volatile uint8_t NewSoftSerial::_receive_buffer_head = 0;



//------------------------------------------------------------------------------
//		Инициализация интерфейса USART для подключения GSM модуля
//------------------------------------------------------------------------------
void NewSoftSerial::GSM_USART_Init(void)
{
//Разрешаем тактирование шины порта USART
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

GPIO_InitTypeDef GPIO_Init_struct;
//Настраеваем вход RX
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_3;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
GPIO_Init(GPIOA, &GPIO_Init_struct);
//Настраеваем выход TX
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP;
GPIO_Init(GPIOA, &GPIO_Init_struct);
//Настраеваем выход включения питания PWRKEY и RESET
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_1|GPIO_Pin_15;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP;
GPIO_Init(GPIOA, &GPIO_Init_struct);
//Настраеваем вход от Ring
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_12;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
GPIO_Init(GPIOA, &GPIO_Init_struct);

GSM_RESET_OFF;
}





//Вывод в USART строки
/*void Print::print(const char *str)
{
for (int i=strlen(str); i>0; i--) print(*str++);
}



//Вывод в USART строки
void Print::print(char *str)
{
for (int i=strlen(str); i>0; i--) print(*str++);
}


//Вывод в USART символа
void Print::print(char ch)
{
//Ждем если прошлый байт еще не передан (Transmit Data Register Empty)
while (!USART2->SR&USART_SR_TXE);
//Поcылаем по USART байт
USART2->DR=ch;
}


//Вывод в USART числа int в текстовом виде
void Print::print(long data)
{
char str[11];
itoa(data, &str[0]);
print(&str[0]);
}


//Вывод в USART числа long в текстовом виде
void Print::print(int data)
{
char str[11];
itoa(data, &str[0]);
print(&str[0]);
}


//Вывод в USART числа в двоичном виде
void Print::print(char data, char b)
{
//Ждем если прошлый байт еще не передан (Transmit Data Register Empty)
while (!USART2->SR&USART_SR_TXE);
//Поcылаем по USART байт
USART2->DR=data;
}
*/



//------------------------------------------------------------------------------
//			Прерывание по приему/переаче данных по USART
//------------------------------------------------------------------------------
void GSM_USART_IRQHandler(void)
{
//Если прерывания по приему
while (USART2->SR & USART_SR_RXNE)
	{
	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	//USART2->SR &= ~USART_SR_RXNE;
	
	//Передаем принятый байт
	NewSoftSerial::GSM_USART_handle_interrupt((uint8_t)USART2->DR);
	}
// Если прерывание по завершению передачи.
if(USART2->SR & USART_SR_TC) 
	{
	USART2->SR &= ~USART_SR_TC;
	}
}



/* static */
inline void NewSoftSerial::GSM_USART_handle_interrupt(uint8_t data)
{
  if (active_object)
  {
    active_object->recv(data);
  }
}



//******************************************************************************
// Private methods
//******************************************************************************


// This function sets the current object as the "active"
// one and returns true if it replaces another 
bool NewSoftSerial::activate(void)
{
if (active_object != this)
	{
	_buffer_overflow = false;
	////uint8_t oldSREG = SREG;
	////cli();
	_receive_buffer_head = _receive_buffer_tail = 0;
	active_object = this;
	////SREG = oldSREG;
	return true;
	}
return false;
}



//------------------------------------------------------------------------------
// Принимаем байт и ложим его в буфер
// The receive routine called by the interrupt handler
//------------------------------------------------------------------------------
void NewSoftSerial::recv(uint8_t data)
{
// if buffer full, set the overflow flag and return
if ((_receive_buffer_tail + 1) % _NewSS_MAX_RX_BUFF != _receive_buffer_head) 
	{
	//читаем содержимое регистра данных USART
	// save new data in buffer: tail points to where byte goes
	_receive_buffer[_receive_buffer_tail] = data; // save new byte
	_receive_buffer_tail = (_receive_buffer_tail + 1) % _NewSS_MAX_RX_BUFF;
	} 
else _buffer_overflow = true;




/*uint8_t d = 0;

// If RX line is high, then we don't see any start bit
// so interrupt is probably not for us
if (_inverse_logic ? rx_pin_read() : !rx_pin_read())
	{
	// Wait approximately 1/2 of a bit width to "center" the sample
	tunedDelay(_rx_delay_centering);

	// Read each of the 8 bits
	for (uint8_t i=0x1; i; i <<= 1)
		{
		tunedDelay(_rx_delay_intrabit);

		uint8_t noti = ~i;
		if (rx_pin_read()) d |= i;
		// else clause added to ensure function timing is ~balanced
		else  d &= noti;
		}

	// skip the stop bit
	tunedDelay(_rx_delay_stopbit);

	if (_inverse_logic) d = ~d;

	// if buffer full, set the overflow flag and return
	if ((_receive_buffer_tail + 1) % _NewSS_MAX_RX_BUFF != _receive_buffer_head) 
		{
		// save new data in buffer: tail points to where byte goes
		_receive_buffer[_receive_buffer_tail] = d; // save new byte
		_receive_buffer_tail = (_receive_buffer_tail + 1) % _NewSS_MAX_RX_BUFF;
		} 
	else _buffer_overflow = true;
	}*/
}



//
// Constructor
//
NewSoftSerial::NewSoftSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic /* = false */) : 
  _buffer_overflow(false)
{
/*  setTX(transmitPin);
  setRX(receivePin);*/
}

//
// Destructor
//
NewSoftSerial::~NewSoftSerial()
{
  end();
}

/*void NewSoftSerial::setTX(uint8_t tx)
{

}

void NewSoftSerial::setRX(uint8_t rx)
{

}*/

//
// Public methods
//

void NewSoftSerial::begin(long speed)
{
GSM_USART_Init();

//Включаем тактирование шины USART2
RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

//Инициализируем USART1
USART_InitTypeDef USART_Init_struct;
USART_StructInit(&USART_Init_struct);

USART_Init_struct.USART_BaudRate=speed;
USART_Init_struct.USART_WordLength=USART_WordLength_8b;
USART_Init_struct.USART_StopBits=USART_StopBits_1;
USART_Init_struct.USART_Parity=USART_Parity_No;
USART_Init_struct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
USART_Init_struct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;

USART_Init(USART2, &USART_Init_struct);

NVIC_InitTypeDef NVIC_InitStructure;

NVIC_InitStructure.NVIC_IRQChannel=USART2_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
NVIC_Init(&NVIC_InitStructure);

//Разрешаем прерывания по приему //и передаче
USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
//USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

//Включаем USART2
USART_Cmd(USART2, ENABLE);

activate();
}



void NewSoftSerial::end()
{
/*  if (digitalPinToPCMSK(_receivePin))
    *digitalPinToPCMSK(_receivePin) &= ~_BV(digitalPinToPCMSKbit(_receivePin));*/
}



// Read data from buffer
int NewSoftSerial::read(void)
{
uint8_t d;

// A newly activated object never has any rx data
if (activate()) return -1;

// Empty buffer?
if (_receive_buffer_head == _receive_buffer_tail) return -1;

// Read from "head"
d = _receive_buffer[_receive_buffer_head]; // grab next byte
_receive_buffer_head = (_receive_buffer_head + 1) % _NewSS_MAX_RX_BUFF;
return d;
}



uint8_t NewSoftSerial::available(void)
{
// A newly activated object never has any rx data
if (activate()) return 0;

return (_receive_buffer_tail + _NewSS_MAX_RX_BUFF - _receive_buffer_head) % _NewSS_MAX_RX_BUFF;
}



void NewSoftSerial::write(uint8_t b)
{
//Ждем если прошлый байт еще не передан (Transmit Data Register Empty)
while (!(USART2->SR & USART_SR_TXE));
//Поcылаем по USART байт
USART2->DR=b;


/*if (_tx_delay == 0) return;

activate();

uint8_t oldSREG = SREG;
cli();  // turn off interrupts for a clean txmit

// Write the start bit
tx_pin_write(_inverse_logic ? HIGH : LOW);
tunedDelay(_tx_delay + XMIT_START_ADJUSTMENT);

// Write each of the 8 bits
if (_inverse_logic)
	{
	for (byte mask = 0x01; mask; mask <<= 1)
		{
		// choose bit
		if (b & mask) tx_pin_write(LOW); // send 1
		else tx_pin_write(HIGH); // send 0
		tunedDelay(_tx_delay);
		}

	tx_pin_write(LOW); // restore pin to natural state
	}
else
	{
	for (byte mask = 0x01; mask; mask <<= 1)
		{
		// choose bit
		if (b & mask) tx_pin_write(HIGH); // send 1
		else tx_pin_write(LOW); // send 0
		tunedDelay(_tx_delay);
		}
	tx_pin_write(HIGH); // restore pin to natural state
	}

SREG = oldSREG; // turn interrupts back on
tunedDelay(_tx_delay);*/
}



/*void NewSoftSerial::enable_timer0(bool enable) 
{
  if (enable)
#if defined(__AVR_ATmega8__)
    sbi(TIMSK, TOIE0);
#else
    sbi(TIMSK0, TOIE0);
#endif
  else 
#if defined(__AVR_ATmega8__)
    cbi(TIMSK, TOIE0);
#else
    cbi(TIMSK0, TOIE0);
#endif
}*/



void NewSoftSerial::flush()
{
if (active_object == this)
	{
	////uint8_t oldSREG = SREG;
	////cli();
	_receive_buffer_head = _receive_buffer_tail = 0;
	////SREG = oldSREG;*/
	}
}
