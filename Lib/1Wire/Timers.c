


unsigned int time_button = 300;
unsigned char ibutton[10];







//==============================================================================
#pragma vector=TIMER0_OVF_vect
__interrupt void tm0(void)
{
if (time_button) time_button--;
else
    {   
      GET_DATA_IBUTTON();
      time_button = 200;
    }
}
