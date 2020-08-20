/*
This is a Beta version.
last modified 14/02/2012.

This library is based on one developed by Arduino Labs
and it is modified to preserve the compability
with the Arduino's product.

The library is modified to use the GSM Shield,
developed by www.open-electronics.org
(http://www.open-electronics.org/arduino-gsm-shield/)
and based on SIM900 chip,
with the same commands of Arduino Shield,
based on QuectelM10 chip.
*/

#include "GSM.h"
#include "WideTextFinder.h"

#include <string.h>

#include "../../main.h"


//De-comment this two lines below if you have the
//first version of GSM GPRS Shield
//#define _GSM_TXPIN_ 4
//#define _GSM_RXPIN_ 5

//De-comment this two lines below if you have the
//second version og GSM GPRS Shield
#define _GSM_TXPIN_ 2
#define _GSM_RXPIN_ 3

GSM::GSM():_cell(_GSM_TXPIN_,_GSM_RXPIN_),_tf(_cell, 10),_status(IDLE){
};



int GSM::begin(long baud_rate)
{
//int response=-1;
int cont=0;
boolean norep=false;
boolean turnedON=false;
SetCommLineStatus(CLS_ATCMD);
_cell.begin(baud_rate);
setStatus(IDLE);
	
for (cont=0; cont<3; cont++)
	{
	if (AT_RESP_ERR_NO_RESP == SendATCmdWaitResp("AT", 500, 100, "OK", 5)&&!turnedON)
		{//check power, there is no response => turn on the module
		#ifdef DEBUG_ON
			Serial.println("DB:NO RESP");
		#endif
		// generate turn on pulse
		GSM_PWRKEY_ON;
		delay_ms(1200);
		GSM_PWRKEY_OFF;
		IWDG_ReloadCounter();
		delay_ms(15000);
		IWDG_ReloadCounter();
		delay_ms(15000);
		IWDG_ReloadCounter();
		delay_ms(15000);
		IWDG_ReloadCounter();
		delay_ms(15000);
		IWDG_ReloadCounter();
		norep=true;
		}
	else
		{
		#ifdef DEBUG_ON
			Serial.println("DB:ELSE");
		#endif
		norep=false;
		}
	}

if (norep==true)
	{
	GSM_RESET_ON;
	delay_ms(1200);
	GSM_RESET_OFF;
	delay_ms(5000);
	GSM_PWRKEY_ON;//digitalWrite(GSM_ON, HIGH);
	delay_ms(1200);
	GSM_PWRKEY_OFF;//digitalWrite(GSM_ON, LOW);
	delay_ms(15000);
	IWDG_ReloadCounter();
	delay_ms(15000);
	IWDG_ReloadCounter();
	}
	
if (AT_RESP_OK == SendATCmdWaitResp("AT", 500, 100, "OK", 5))
	{
	#ifdef DEBUG_ON
		Serial.println("DB:CORRECT BR");
	#endif
	turnedON=true;
	}
if(cont==3&&norep)
	{
	//Serial.println("ERROR: SIM900 doesn't answer. Check power and serial pins in GSM.cpp");
	return 0;
	}

if (AT_RESP_ERR_DIF_RESP == SendATCmdWaitResp("AT", 500, 100, "OK", 5)&&!turnedON)
	{		//check OK
	#ifdef DEBUG_ON
		Serial.println("DB:DIFF RESP");
	#endif
	for (int i=1;i<8;i++)
		{
		switch (i)
			{
			case 1: _cell.begin(2400); break;
			case 2: _cell.begin(4800); break;
			case 3: _cell.begin(9600); break;
			case 4: _cell.begin(19200);break;
			case 5: _cell.begin(38400);break;
			case 6: _cell.begin(57600);break;
			case 7:
				_cell.begin(115200);
				_cell.print("AT+IPR=9600\r");
				_cell.begin(9600);
				delay_ms(500);
			break;
			// if nothing else matches, do the default
			// default is optional
			}
				
		delay_ms(100);

		#ifdef DEBUG_PRINT
			// parameter 0 - because module is off so it is not necessary 
			// to send finish AT<CR> here
			DebugPrint("DEBUG: Stringa ", 0);
			DebugPrint(buff, 0);
		#endif

		if (AT_RESP_OK == SendATCmdWaitResp("AT", 500, 100, "OK", 5))
			{
			#ifdef DEBUG_ON
				Serial.println("DB:FOUND PREV BR");
			#endif
			_cell.print("AT+IPR=");
			_cell.print(baud_rate);    
			_cell.print("\r"); // send <CR>
			delay_ms(500);
			_cell.begin(baud_rate);
			delay_ms(100);
			if (AT_RESP_OK == SendATCmdWaitResp("AT", 500, 100, "OK", 5))
				{
				#ifdef DEBUG_ON
					Serial.println("DB:OK BR");
				#endif
				}
			turnedON=true;
			break;					
			}
		#ifdef DEBUG_ON
			Serial.println("DB:NO BR");
		#endif			
		}

	// communication line is not used yet = free
	SetCommLineStatus(CLS_FREE);
	// pointer is initialized to the first item of comm. buffer
	p_comm_buf = &comm_buf[0];
	}

SetCommLineStatus(CLS_FREE);

if(turnedON)
	{
	InitParam(PARAM_SET_0);
	IWDG_ReloadCounter();
	InitParam(PARAM_SET_1);//configure the module  
	IWDG_ReloadCounter();
	Echo(1);               //enable AT echo
	setStatus(READY);
	return(1);
	}
else
	{
	//just to try to fix some problems with 115200 baudrate
	_cell.begin(115200);
	delay_ms(1000);
	_cell.print("AT+IPR=");
	_cell.print(baud_rate);    
	_cell.print("\r"); // send <CR>		
	return(0);
	}
}



void GSM::InitParam(byte group){
	switch (group) {
	case PARAM_SET_0:
		// check comm line
		//if (CLS_FREE != GetCommLineStatus()) return;

		SetCommLineStatus(CLS_ATCMD);
		// Reset to the factory settings
		SendATCmdWaitResp("AT&F", 1000, 50, "OK", 5);      
		// switch off echo
		SendATCmdWaitResp("ATE1", 500, 50, "OK", 5);
		// setup fixed baud rate
		//SendATCmdWaitResp("AT+IPR=9600", 500, 50, "OK", 5);
		SetCommLineStatus(CLS_FREE);
		break;

	case PARAM_SET_1:
		// check comm line
		//if (CLS_FREE != GetCommLineStatus()) return;
		SetCommLineStatus(CLS_ATCMD);
		// Request calling line identification
		SendATCmdWaitResp("AT+CLIP=1", 500, 50, "OK", 5);
		// Mobile Equipment Error Code
		SendATCmdWaitResp("AT+CMEE=2", 500, 50, "OK", 5);
		// set the SMS mode to PDU
		SendATCmdWaitResp("AT+CMGF=0", 500, 50, "OK", 5);
		//SendATCmdWaitResp("AT+CSCS=\"UCS2\"", 500, 50, "OK", 5);
		SendATCmdWaitResp("AT+CSCS=\"GSM\"", 500, 50, "OK", 5);
		// Auto answer after first ring enabled
		// auto answer is not used
		//SendATCmdWaitResp("ATS0=1", 500, 50, "OK", 5);
		// select ringer sound level
		//SendATCmdWaitResp("AT+CRSL=2", 500, 50, "OK", 5);
		// we must release comm line because SetSpeakerVolume()
		// checks comm line if it is free
		SetCommLineStatus(CLS_FREE);
		// select speaker volume (0 to 14)
		//SetSpeakerVolume(9);
		// select mic gaim
		SendATCmdWaitResp("AT+CMIC=0,15", 500, 50, "OK", 5);
		SendATCmdWaitResp("AT+CMIC=2,15", 500, 50, "OK", 5);
		
		// select phonebook memory storage
		SendATCmdWaitResp("AT+CPBS=\"SM\"", 1000, 50, "OK", 5);
		SendATCmdWaitResp("AT+CIPSHUT", 500, 50, "SHUT OK", 5);
		// init SMS storage
		InitSMSMemory();
		
		//Режим пониженного энергопотребления с рабочим радио и просыпанием по USART
		SetModemSleepMode(false);
		break;
	}
}



byte GSM::WaitResp(uint16_t start_comm_tmout,
			    uint16_t max_interchar_tmout,
                   char const *expected_resp_string)
{
byte status;
byte ret_val;

RxInit(start_comm_tmout, max_interchar_tmout);
// wait until response is not finished
do {
	status = IsRxFinished();
	} while (status == RX_NOT_FINISHED);

if (status == RX_FINISHED)
	{
	// something was received but what was received?
	// ---------------------------------------------
	
	if(IsStringReceived(expected_resp_string))
		{
		// expected string was received
		// ----------------------------
		ret_val = RX_FINISHED_STR_RECV;
		}
	else ret_val = RX_FINISHED_STR_NOT_RECV;
	}
else
	{
	// nothing was received
	// --------------------
	ret_val = RX_TMOUT_ERR;
	}
return (ret_val);
}



/**********************************************************
Method sends AT command and waits for response

return: 
      AT_RESP_ERR_NO_RESP = -1,   // no response received
      AT_RESP_ERR_DIF_RESP = 0,   // response_string is different from the response
      AT_RESP_OK = 1,             // response_string was included in the response
**********************************************************/
char GSM::SendATCmdWaitResp(char const *AT_cmd_string,
                uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                char const *response_string,
                byte no_of_attempts)
{
byte status;
char ret_val = AT_RESP_ERR_NO_RESP;
byte i;

for (i = 0; i < no_of_attempts; i++)
	{
	// delay 500 msec. before sending next repeated AT command 
	// so if we have no_of_attempts=1 tmout will not occurred
	if (i > 0) delay_ms(500); 

	_cell.println(AT_cmd_string);
	status = WaitResp(start_comm_tmout, max_interchar_tmout);
	if (status == RX_FINISHED)
		{
		// something was received but what was received?
		// ---------------------------------------------
		if(IsStringReceived(response_string))
			{
			ret_val = AT_RESP_OK;      
			break;  // response is OK => finish
			}
		else ret_val = AT_RESP_ERR_DIF_RESP;
		}
	else
		{
		// nothing was received
		// --------------------
		ret_val = AT_RESP_ERR_NO_RESP;
		}
	}
return (ret_val);
}



byte GSM::WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
byte status;

RxInit(start_comm_tmout, max_interchar_tmout); 
// wait until response is not finished
do
	{
	status = IsRxFinished();
	} while (status == RX_NOT_FINISHED);
return (status);
}




/*******************************************************************************
Ожидание окончания приема данных по таймаутам
return:
	ERROR ret. val:
	---------------
	RX_TMOUT_ERR	 - вышло время ожидания начала приема данных
	RX_NOT_FINISHED - превышено время ожидания приема следующего байта

	OK ret val:
	-----------
	RX_FINISHED - Прием окончен
*******************************************************************************/
byte GSM::IsRxFinished(void)
{
byte num_of_bytes;
byte ret_val = RX_NOT_FINISHED;  // default not finished

// Rx state machine
if (rx_state == RX_NOT_STARTED)
	{
	// Reception is not started yet - check tmout
	if (!_cell.available())
		{
		// still no character received => check timeout
		if ((millis() - prev_time) >= start_reception_tmout)
			{
			// timeout elapsed => GSM module didn't start with response
			// so communication is takes as finished
			comm_buf[comm_buf_len] = 0x00;
			ret_val = RX_TMOUT_ERR;
			}
		}
	else
		{
		// at least one character received => so init inter-character 
		// counting process again and go to the next state
		prev_time = millis(); // init tmout for inter-character space
		rx_state = RX_ALREADY_STARTED;
		}
	}

if (rx_state == RX_ALREADY_STARTED)
	{
	// Reception already started check new received bytes only in case we have place in the buffer
	num_of_bytes = _cell.available();
	// if there are some received bytes postpone the timeout
	if (num_of_bytes) prev_time = millis();

	// read all received bytes      
	while (num_of_bytes)
		{
		num_of_bytes--;
		if (comm_buf_len < COMM_BUF_LEN)
			{
			// we have still place in the GSM internal comm. buffer =>
			// move available bytes from circular buffer to the rx buffer
			*p_comm_buf = _cell.read();

			p_comm_buf++;
			comm_buf_len++;
			comm_buf[comm_buf_len] = 0x00;// and finish currently received characters
									// so after each character we have
									// valid string finished by the 0x00
			}
		else
			{
			// comm buffer is full, other incoming characters will be discarded 
			// but despite of we have no place for other characters 
			// we still must to wait until inter-character tmout is reached
        
			// so just readout character from circular RS232 buffer 
			// to find out when communication id finished(no more characters
			// are received in inter-char timeout)
			_cell.read();
			}
		}

		// finally check the inter-character timeout
	if ((unsigned long)(millis() - prev_time) >= interchar_tmout)
		{
		// timeout between received character was reached reception is finished
		comm_buf[comm_buf_len] = 0x00;// for sure finish string again
								// but it is not necessary
		ret_val = RX_FINISHED;
		}
	}
return (ret_val);
}



/**********************************************************
Method checks received bytes

compare_string - pointer to the string which should be find

return: 0 - string was NOT received
        1 - string was received
**********************************************************/
byte GSM::IsStringReceived(char const *compare_string)
{
char *ch;
byte ret_val = 0;

if(comm_buf_len)
	{
	ch = strstr((char *)comm_buf, compare_string);
	if (ch != NULL) ret_val = 1;
	}
return (ret_val);
}



void GSM::RxInit(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
rx_state = RX_NOT_STARTED;
start_reception_tmout = start_comm_tmout;
interchar_tmout = max_interchar_tmout;
prev_time = millis();
comm_buf[0] = 0x00; // end of string
p_comm_buf = &comm_buf[0];
comm_buf_len = 0;
_cell.flush(); // erase rx circular buffer
}



void GSM::Echo(byte state)
{
if (state == 0 or state == 1)
	{
	SetCommLineStatus(CLS_ATCMD);

	_cell.print("ATE");
	_cell.print((int)state);    
	_cell.print('\r');
	delay_ms(500);
	SetCommLineStatus(CLS_FREE);
	}
}



char GSM::InitSMSMemory(void) 
{
char ret_val = -1;

  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  ret_val = 0; // not initialized yet
  
  // Disable messages about new SMS from the GSM module 
  SendATCmdWaitResp("AT+CNMI=2,0", 1000, 50, "OK", 2);

  // send AT command to init memory for SMS in the SIM card
  // response:
  // +CPMS: <usedr>,<totalr>,<usedw>,<totalw>,<useds>,<totals>
  if (AT_RESP_OK == SendATCmdWaitResp("AT+CPMS=\"SM\",\"SM\",\"SM\"", 1000, 1000, "+CPMS:", 10)) {
    ret_val = 1;
  }
  else ret_val = 0;

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}

int GSM::isIP(const char* cadena)
{
    int i;
    for (i=0; i<strlen(cadena); i++)
        if (!(cadena[i]=='.' || ( cadena[i]>=48 && cadena[i] <=57)))
            return 0;
    return 1;
}



/*******************************************************************************
	Вывод модема из спящего режима для дальнейшей отправки ему команд
*******************************************************************************/
void GSM::ModemWakeUp(void)
{
if (module_sleep_status) SendATCmdWaitResp("", 100, 100, "", 1);//Будим модем
}



/*******************************************************************************
	Включение/выключение режима пониженного энергоптребления модема
*******************************************************************************/
void GSM::SetModemSleepMode(bool mode)
{
module_sleep_status=(byte)mode;
if (mode) SendATCmdWaitResp("AT+CSCLK=2", 1000, 100, "OK", 1);
else SendATCmdWaitResp("AT+CSCLK=0", 1000, 100, "OK", 1);
}