

#include "1wire/1wire.h"




void iButton_init(void)
{
Init_1Wire_HW();
}


//------------------------------------------------------------------------------
bool iButton_Get_data (unsigned char ibutton[])
{
clear_DQ;//PORT_1WIRE &= ~(1 << DQ);
//DDR_1WIRE &= ~(1 << DQ);
if (init_1wire(0xF0))
	{
	if (read_byte_1wire() == 0xF5)
		{
		if (init_1wire(0x33))
			{
			for (char i=0; i<8; i++) ibutton[7-i] = read_byte_1wire();
			}
		return true;
		}
	else for (char i=0; i<8; i++)  ibutton[i] = 0;
	}
else for (char i=0; i<8; i++)  ibutton[i] = 0;
return false;
}



bool iButton_compare(unsigned char ibutton1[], unsigned char ibutton2[])
{
for (char i=0; i<8; i++) if (ibutton1[i]!=ibutton2[i]) return false;
return true;
}