/*;****************************************************************************************
; 1-Wire by Nigorodova A. V.
;****************************************************************************************
.equ port_1wire = portd							;
.equ ddr_1wire = ddrd 							;
.equ pin_1wire = pind							;
.equ DQ = pd4								;

*/

#include "1wire.h"
#include "main.h"


void Init_1Wire_HW(void)
{
//Настраеваем вывод 1Wire
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

GPIO_InitTypeDef GPIO_Init_struct;
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;
GPIO_Init_struct.GPIO_Speed=GPIO_Speed_2MHz;
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_OD;
GPIO_Init(GPIOA, &GPIO_Init_struct);
}






char init_1wire(char data)
{
char result;

// generate reset pulse
clear_DQ;
delay_mcs(500);
set_DQ;
	
// wait for release DQ
result = 100;
while (test_DQ)
	{
	delay_mcs(2);
	result--;
	if (result == 0) break;
	}

if (result != 0)
	{
	result = 40;
	while (!test_DQ)
		{
		delay_mcs(2);
		result--;
		if (result == 0) break;
		}
	}
delay_mcs(500);
if (result != 0) write_byte_1wire(data);
return (result);
}



void write_byte_1wire(char data)
{
char temp = data;

for (unsigned char i = 8; i > 0; i--)
	{
	// pull down DQ line for ~8 mcs
	clear_DQ;
	delay_mcs(8);
	// bit to write
	
	if (temp&(1 << 0)) set_DQ;
	// wait for time slot
	delay_mcs(80);	
	set_DQ;
	// restore bus for ~8 mcs
	delay_mcs(8); 	
	temp >>= 1;
	}
}



char read_byte_1wire(void)
{
char temp = 0;

for (unsigned char i = 8; i > 0; i--)
	{
	temp >>= 1;

	// pull down DQ line
	clear_DQ;
	delay_mcs(2);
	// release DQ line
	set_DQ;
	delay_mcs(10);
	// test DQ line
	if test_DQ temp |= 0x80;
	// restore DQ line
	
	delay_mcs(80);
  	}
return(temp);
}

