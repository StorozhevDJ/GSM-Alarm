#include "SIM900.h"  
#include "Streaming.h"

#include <stdio.h>
#include <string.h>


#include "../../main.h"


#define _GSM_CONNECTION_TOUT_ 5
#define _TCP_CONNECTION_TOUT_ 20
#define _GSM_DATA_TOUT_ 10

//#define RESETPIN 7

SIMCOM900 gsm;
SIMCOM900::SIMCOM900(){};
SIMCOM900::~SIMCOM900(){};
 
char SIMCOM900::forceON(){
	char ret_val=0;
	char *p_char; 
	char *p_char1;
	
	SimpleWriteln("AT+CREG?");
	WaitResp(5000, 100, "OK");
	if(IsStringReceived("OK")){
		ret_val=1;
	}
	//BCL
	p_char = strchr((char *)(gsm.comm_buf),',');
	p_char1 = p_char+1;  //we are on the first char of BCS
	*(p_char1+2)=0;
	p_char = strchr((char *)(p_char), ',');
	if (p_char != NULL) {
          *p_char = 0; 
    }

	if((*p_char1)=='4'){
		GSM_PWRKEY_ON;//digitalWrite(GSM_ON, HIGH);
		delay_ms(1200);
		GSM_PWRKEY_OFF;//digitalWrite(GSM_ON, LOW);
		delay_ms(10000);
		ret_val=2;
	}

	return ret_val;
}

int SIMCOM900::configandwait(char* pin)
{
  int connCode;
  _tf.setTimeout(_GSM_CONNECTION_TOUT_);

  if(pin) setPIN(pin); //syv

  // Try 10 times to register in the network. Note this can take some time!
  for(int i=0; i<10; i++)
  {  	
    //Ask for register status to GPRS network.
    _cell << "AT+CGREG?" <<  _DEC(cr) << endl; 

    //Se espera la unsolicited response de registered to network.
    while (_tf.find("+CGREG: 0,"))  // CHANGE!!!!
	{
		connCode=_tf.getValue();
		if((connCode==1)||(connCode==5))
		{
		  setStatus(READY);
		  
		_cell << "AT+CMGF=1" <<  _DEC(cr) << endl; //SMS text mode.
		delay_ms(200);
		  // Buah, we should take this to readCall()
		_cell << "AT+CLIP=1" <<  _DEC(cr) << endl; //SMS text mode.
		delay_ms(200);
		//_cell << "AT+QIDEACT" <<  _DEC(cr) << endl; //To make sure not pending connection.
		//delay_ms(1000);
	  
		  return 1;
		}
	}
  }
  return 0;
}

int SIMCOM900::read(char* result, int resultlength)
{
  // Or maybe do it with AT+QIRD

  int charget;
  _tf.setTimeout(3);
  // Not well. This way we read whatever comes in one second. If a CLOSED 
  // comes, we have spent a lot of time
    //charget=_tf.getString("",'\0',result, resultlength);
    charget=_tf.getString("","",result, resultlength);
  /*if(strtok(result, "CLOSED")) // whatever chain the Q10 returns...
  {
    // TODO: use strtok to delete from the chain everything from CLOSED
    if(getStatus()==TCPCONNECTEDCLIENT)
      setStatus(ATTACHED);
    else
      setStatus(TCPSERVERWAIT);
  }  */
  
  return charget;
}



int SIMCOM900::readCellData(int &mcc, int &mnc, long &lac, long &cellid)
{
  if (getStatus()==IDLE)
    return 0;
    
   _tf.setTimeout(_GSM_DATA_TOUT_);
   //_cell.flush();
  _cell << "AT+QENG=1,0" << endl; 
  _cell << "AT+QENG?" << endl; 
  if(!_tf.find("+QENG:"))
    return 0;

  mcc=_tf.getValue(); // The first one is 0
  mcc=_tf.getValue();
  mnc=_tf.getValue();
  lac=_tf.getValue();
  cellid=_tf.getValue();
  _tf.find("OK");
  _cell << "AT+QENG=1,0" << endl; 
  _tf.find("OK");
  return 1;
}



boolean SIMCOM900::readSMS_text(char* msg, int msglength, char* number, int nlength)
{
  long index;
  /*
  if (getStatus()==IDLE)
    return false;
  */
  _tf.setTimeout(_GSM_DATA_TOUT_);
  //_cell.flush();
  _cell << "AT+CMGL=\"REC UNREAD\",1" << endl;
  if(_tf.find("+CMGL: "))
  {
    index=_tf.getValue();
    _tf.getString("\"+", "\"", number, nlength);
    _tf.getString("\n", "\nOK", msg, msglength);
    _cell << "AT+CMGD=" << index << endl;
    _tf.find("OK");
    return true;
  };
  return false;
};



/*******************************************************************************
					������ ��� � ������� PDU
value:
	msg - ��������� �� ������������ ��������� � ������� PDU
	msglength - ������ ������
	number - ����� ��� � ������ �� SIM �����
return:
	true	 - ������ ��������� �������
	false - �� ������� ��������� ��� (�� ������ ����� ��������� 
		��� ��� �� ������� ������ �����������)
*******************************************************************************/
boolean SIMCOM900::readSMS_PDU(char* msg, int msglength, char number)
{
if ((number<1)||(number>20)) return false;

gsm.SetCommLineStatus(CLS_ATCMD);

_tf.setTimeout(_GSM_DATA_TOUT_);

gsm.SimpleWrite("AT+CMGR=");
gsm.SimpleWriteln((int)number);

if(_tf.find("+CMGR:"))
	{
	if (_tf.getString("\n", "\r\nOK", msg, msglength))
		{
		gsm.SetCommLineStatus(CLS_FREE);
		return true;
		}
	};
gsm.SetCommLineStatus(CLS_FREE);
return false;
};



/*******************************************************************************
					���������� CUSD �������
value:
	msg - ��������� �� ������������ ��������� �������
	msglength - ������ ������
	number - ����� ��� ������� CUSD, ���������� ������ ����� ��� �������� * � #
return:
	true	 - ������ �������� �������
	false - ������ �� ��������
*******************************************************************************/
boolean SIMCOM900::Get_CUSD(char* msg, int msglength, char number)
{
gsm.SetCommLineStatus(CLS_ATCMD);

_tf.setTimeout(_GSM_DATA_TOUT_);

if (number==0) return false;

gsm.SimpleWrite("AT+CUSD=1,\"#");
gsm.SimpleWrite((int)number);
gsm.SimpleWriteln("#\"");

if(_tf.find("+CUSD:"))
	{
	if (_tf.getString(",\"", "\",", msg, msglength))
		{
		gsm.SetCommLineStatus(CLS_FREE);
		return true;
		}
	};
gsm.SetCommLineStatus(CLS_FREE);
return false;
};



/*******************************************************************************
		���������� CUSD ������� �� �������� �����
value:
	number - ������ � ������� ��� ������� CUSD
return:
	true	 - ������ �������� �������
	false - ������ �� ��������
*******************************************************************************/
boolean SIMCOM900::Send_CUSD(char* msg, int msglength, char* number)
{
gsm.SetCommLineStatus(CLS_ATCMD);

_tf.setTimeout(_GSM_DATA_TOUT_);

if (number[0]==0) return false;

gsm.SimpleWrite("AT+CUSD=1,\"");
gsm.SimpleWrite(number);
gsm.SimpleWriteln("\"");

if(_tf.find("+CUSD:"))
	{
	if (_tf.getString(",\"", "\",", msg, msglength))
		{
		gsm.SetCommLineStatus(CLS_FREE);
		return true;
		}
	};
gsm.SetCommLineStatus(CLS_FREE);
return false;
};



boolean SIMCOM900::readCall(char* number, int nlength)
{
//  int index;

  if (getStatus()==IDLE)
    return false;
  
  _tf.setTimeout(_GSM_DATA_TOUT_);

  if(_tf.find("+CLIP: \""))
  {
    _tf.getString("", "\"", number, nlength);
    _cell << "ATH" << endl;
    delay_ms(1000);
    //_cell.flush();
    return true;
  };
  return false;
};



boolean SIMCOM900::call(char* number, unsigned int milliseconds)
{ 
  if (getStatus()==IDLE)
    return false;
  
  _tf.setTimeout(_GSM_DATA_TOUT_);

  _cell << "ATD" << number << ";" << endl;
  delay_ms(milliseconds);
  _cell << "ATH" << endl;

  return true;
}



int SIMCOM900::setPIN(char *pin)
{
  //Status = READY or ATTACHED.
  if((getStatus() != IDLE))
    return 2;
      
  _tf.setTimeout(_GSM_DATA_TOUT_);	//Timeout for expecting modem responses.

  //_cell.flush();

  //AT command to set PIN.
  _cell << "AT+CPIN=" << pin <<  _DEC(cr) << endl; // Establecemos el pin

  //Expect "OK".
  if(!_tf.find("OK"))
    return 0;
  else  
    return 1;
}


int SIMCOM900::changeNSIPmode(char mode)
{
    _tf.setTimeout(_TCP_CONNECTION_TOUT_);
    
    //if (getStatus()!=ATTACHED)
    //    return 0;

    //_cell.flush();

    _cell << "AT+QIDNSIP=" << mode <<  _DEC(cr) << endl;

    if(!_tf.find("OK")) return 0;
    
    return 1;
}



int SIMCOM900::getCCI(char *cci)
{
  //Status must be READY
  if((getStatus() != READY))
    return 2;
      
  _tf.setTimeout(_GSM_DATA_TOUT_);	//Timeout for expecting modem responses.

  //_cell.flush();

  //AT command to get CCID.
  _cell << "AT+QCCID" << _DEC(cr) << endl; // Establecemos el pin
  
  //Read response from modem
  _tf.getString("AT+QCCID\r\r\r\n","\r\n",cci, 21);
  
  //Expect "OK".
  if(!_tf.find("OK"))
    return 0;
  else  
    return 1;
}



int SIMCOM900::getIMEI(char *imei)
{
      
  _tf.setTimeout(_GSM_DATA_TOUT_);	//Timeout for expecting modem responses.

  //_cell.flush();

  //AT command to get IMEI.
  _cell << "AT+GSN" << /*_DEC(cr)*/'\r' << endl; 
  
  //Read response from modem
  _tf.getString("AT+GSN\r\r\n\r\n","\r\n",imei, 16);
  
  //Expect "OK".
  if(!_tf.find("OK"))
    return 0;
  else  
    return 1;
}



uint8_t SIMCOM900::read()
{
  return _cell.read();
}



uint8_t SIMCOM900::find(char *target)
{
return _tf.find(target);
}



int SIMCOM900::getString(char *pre_string, char *post_string, char *buffer, int length)
{
return _tf.getString(pre_string, post_string, buffer, length);
}



void SIMCOM900::SimpleRead()
{
	char datain;
	if(_cell.available()>0){
		datain=_cell.read();
		if(datain>0){
			//Serial.print(datain);
		}
	}
}



void SIMCOM900::SimpleWrite(char *comm)
{
	_cell.print(comm);
}



void SIMCOM900::SimpleWrite(const char *comm)
{
	_cell.print(comm);
}



void SIMCOM900::SimpleWrite(int comm)
{
	_cell.print(comm);
}



void SIMCOM900::SimpleWriteln(char *comm)
{
	_cell.println(comm);
}



void SIMCOM900::SimpleWriteln(char const *comm)
{
	_cell.println(comm);
}



void SIMCOM900::SimpleWriteln(int comm)
{
	_cell.println(comm);
}



void SIMCOM900::WhileSimpleRead()
{
	char datain;
	while(_cell.available()>0){
		datain=_cell.read();
		if(datain>0){
			//Serial.print(datain);
		}
	}
}



//---------------------------------------------
/**********************************************************
Turns on/off the speaker

off_on: 0 - off
        1 - on
**********************************************************/
void GSM::SetSpeaker(byte off_on)
{
  if (CLS_FREE != GetCommLineStatus()) return;
  SetCommLineStatus(CLS_ATCMD);
  if (off_on) {
    //SendATCmdWaitResp("AT#GPIO=5,1,2", 500, 50, "#GPIO:", 1);
  }
  else {
    //SendATCmdWaitResp("AT#GPIO=5,0,2", 500, 50, "#GPIO:", 1);
  }
  SetCommLineStatus(CLS_FREE);
}



byte GSM::IsRegistered(void)
{
  return (module_status & STATUS_REGISTERED);
}



byte GSM::IsInitialized(void)
{
  return (module_status & STATUS_INITIALIZED);
}



/**********************************************************
Method checks if the GSM module is registered in the GSM net
- this method communicates directly with the GSM module
  in contrast to the method IsRegistered() which reads the
  flag from the module_status (this flag is set inside this method)

- must be called regularly - from 1sec. to cca. 10 sec.

return values: 
      REG_NOT_REGISTERED  - not registered
      REG_REGISTERED      - GSM module is registered
      REG_NO_RESPONSE     - GSM doesn't response
      REG_COMM_LINE_BUSY  - comm line between GSM module and Arduino is not free
                            for communication
**********************************************************/
byte GSM::CheckRegistration(void)
{
byte status;
byte ret_val = REG_NOT_REGISTERED;

if (CLS_FREE != GetCommLineStatus()) return (REG_COMM_LINE_BUSY);
SetCommLineStatus(CLS_ATCMD);
_cell.println("AT+CREG?");
// 5 sec. for initial comm tmout
// 50 msec. for inter character timeout
status = WaitResp(5000, 50); 

if (status == RX_FINISHED)
	{
	// something was received but what was received?
	// ---------------------------------------------
	if(IsStringReceived("+CREG: 0,1") 
     || IsStringReceived("+CREG: 0,5"))
		{
		// it means module is registered
		// ----------------------------
		module_status |= STATUS_REGISTERED;

		// in case GSM module is registered first time after reset
		// sets flag STATUS_INITIALIZED
		// it is used for sending some init commands which 
		// must be sent only after registration
		// --------------------------------------------
		if (!IsInitialized())
			{
			module_status |= STATUS_INITIALIZED;
			SetCommLineStatus(CLS_FREE);
			InitParam(PARAM_SET_1);
			}
		ret_val = REG_REGISTERED;      
		}
	else
		{
		// NOT registered
		// --------------
		module_status &= ~STATUS_REGISTERED;
		ret_val = REG_NOT_REGISTERED;
		}
	}
else
	{
	// nothing was received
	// --------------------
	ret_val = REG_NO_RESPONSE;
	}
SetCommLineStatus(CLS_FREE);
return (ret_val);
}


/**********************************************************
Method sets speaker volume

speaker_volume: volume in range 0..14

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module did not answer in timeout
        -3 - GSM module has answered "ERROR" string

        OK ret val:
        -----------
        0..14 current speaker volume 
**********************************************************/
/*
char GSM::SetSpeakerVolume(byte speaker_volume)
{
  
  char ret_val = -1;

  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // remember set value as last value
  if (speaker_volume > 14) speaker_volume = 14;
  // select speaker volume (0 to 14)
  // AT+CLVL=X<CR>   X<0..14>
  _cell.print("AT+CLVL=");
  _cell.print((int)speaker_volume);    
  _cell.print("\r"); // send <CR>
  // 10 sec. for initial comm tmout
  // 50 msec. for inter character timeout
  if (RX_TMOUT_ERR == WaitResp(10000, 50)) {
    ret_val = -2; // ERROR
  }
  else {
    if(IsStringReceived("OK")) {
      last_speaker_volume = speaker_volume;
      ret_val = last_speaker_volume; // OK
    }
    else ret_val = -3; // ERROR
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}
*/
/**********************************************************
Method increases speaker volume

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module did not answer in timeout
        -3 - GSM module has answered "ERROR" string

        OK ret val:
        -----------
        0..14 current speaker volume 
**********************************************************/
/*
char GSM::IncSpeakerVolume(void)
{
  char ret_val;
  byte current_speaker_value;

  current_speaker_value = last_speaker_volume;
  if (current_speaker_value < 14) {
    current_speaker_value++;
    ret_val = SetSpeakerVolume(current_speaker_value);
  }
  else ret_val = 14;

  return (ret_val);
}
*/
/**********************************************************
Method decreases speaker volume

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module did not answer in timeout
        -3 - GSM module has answered "ERROR" string

        OK ret val:
        -----------
        0..14 current speaker volume 
**********************************************************/
/*
char GSM::DecSpeakerVolume(void)
{
  char ret_val;
  byte current_speaker_value;

  current_speaker_value = last_speaker_volume;
  if (current_speaker_value > 0) {
    current_speaker_value--;
    ret_val = SetSpeakerVolume(current_speaker_value);
  }
  else ret_val = 0;

  return (ret_val);
}
*/

/**********************************************************
Method sends DTMF signal
This function only works when call is in progress

dtmf_tone: tone to send 0..15

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - GSM module has answered "ERROR" string

        OK ret val:
        -----------
        0.. tone
**********************************************************/
/*
char GSM::SendDTMFSignal(byte dtmf_tone)
{
  char ret_val = -1;

  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  // e.g. AT+VTS=5<CR>
  _cell.print("AT+VTS=");
  _cell.print((int)dtmf_tone);    
  _cell.print("\r");
  // 1 sec. for initial comm tmout
  // 50 msec. for inter character timeout
  if (RX_TMOUT_ERR == WaitResp(1000, 50)) {
    ret_val = -2; // ERROR
  }
  else {
    if(IsStringReceived("OK")) {
      ret_val = dtmf_tone; // OK
    }
    else ret_val = -3; // ERROR
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}
*/

/**********************************************************
Method returns state of user button


return: 0 - not pushed = released
        1 - pushed
**********************************************************/
byte GSM::IsUserButtonPushed(void)
{
byte ret_val = 0;
if (CLS_FREE != GetCommLineStatus()) return(0);
SetCommLineStatus(CLS_ATCMD);
//if (AT_RESP_OK == SendATCmdWaitResp("AT#GPIO=9,2", 500, 50, "#GPIO: 0,0", 1)) {
	// user button is pushed
	//  ret_val = 1;
	//}
//else ret_val = 0;
SetCommLineStatus(CLS_FREE);
return (ret_val);
}



/**********************************************************
Method reads phone number string from specified SIM position

position:     SMS position <1..20>

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0
        phone_number is empty string

        OK ret val:
        -----------
        0 - there is no phone number on the position
        1 - phone number was found
        phone_number is filled by the phone number string finished by 0x00
                     so it is necessary to define string with at least
                     15 bytes(including also 0x00 termination character)

an example of usage:
        GSM gsm;
        char phone_num[20]; // array for the phone number string

        if (1 == gsm.GetPhoneNumber(1, phone_num)) {
          // valid phone number on SIM pos. #1 
          // phone number string is copied to the phone_num array
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG phone number: ", 0);
            gsm.DebugPrint(phone_num, 1);
          #endif
        }
        else {
          // there is not valid phone number on the SIM pos.#1
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG there is no phone number", 1);
          #endif
        }
**********************************************************/
char GSM::GetPhoneNumber(byte position, char *phone_number)
{
char ret_val = -1;

char *p_char; 
char *p_char1;

if (position == 0) return (-3);
if (CLS_FREE != GetCommLineStatus()) return (ret_val);
SetCommLineStatus(CLS_ATCMD);
ret_val = 0; // not found yet
phone_number[0] = 0; // phone number not found yet => empty string
  
//send "AT+CPBR=XY" - where XY = position
_cell.print("AT+CPBR=");
_cell.print((int)position);  
_cell.print("\r");

// 5000 msec. for initial comm tmout
// 50 msec. for inter character timeout
switch (WaitResp(5000, 50, "+CPBR"))
	{
	case RX_TMOUT_ERR:
		// response was not received in specific time
		ret_val = -2;
	break;

	case RX_FINISHED_STR_RECV:
		// response in case valid phone number stored:
		// <CR><LF>+CPBR: <index>,<number>,<type>,<text><CR><LF>
		// <CR><LF>OK<CR><LF>

		// response in case there is not phone number:
		// <CR><LF>OK<CR><LF>
		p_char = strchr((char *)(comm_buf),'"');
		if (p_char != NULL)
			{
			p_char++;       // we are on the first phone number character
			// find out '"' as finish character of phone number string
			p_char1 = strchr((char *)(p_char),'"');
			if (p_char1 != NULL)
				{
				*p_char1 = 0; // end of string
				}
			// extract phone number string
			strcpy(phone_number, (char *)(p_char));
			// output value = we have found out phone number string
			ret_val = 1;
			}
	break;

	case RX_FINISHED_STR_NOT_RECV:
		// only OK or ERROR => no phone number
		ret_val = 0; 
	break;
	}

SetCommLineStatus(CLS_FREE);
return (ret_val);
}



/**********************************************************
Method reads phone number string from specified SIM position

position:     SMS position <1..20>

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0
        phone_number is empty string

        OK ret val:
        -----------
        0 - there is no phone number on the position
        1 - phone number was found
        phone_number is filled by the phone number string finished by 0x00
                     so it is necessary to define string with at least
                     15 bytes(including also 0x00 termination character)

an example of usage:
        GSM gsm;
        char phone_num[20]; // array for the phone number string

        if (1 == gsm.GetPhoneNumber(1, phone_num)) {
          // valid phone number on SIM pos. #1 
          // phone number string is copied to the phone_num array
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG phone number: ", 0);
            gsm.DebugPrint(phone_num, 1);
          #endif
        }
        else {
          // there is not valid phone number on the SIM pos.#1
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG there is no phone number", 1);
          #endif
        }
**********************************************************/
char GSM::GetPhoneNumberName(byte position, char *phone_number, char *phone_name)
{
char ret_val = -1;

char *p_char; 

if (position == 0) return (-3);
if (CLS_FREE != GetCommLineStatus()) return (ret_val);
SetCommLineStatus(CLS_ATCMD);
ret_val = 0; // not found yet
phone_number[0] = 0; // phone number not found yet => empty string
  
//send "AT+CPBR=XY" - where XY = position
_cell.print("AT+CPBR=");
_cell.print((int)position);  
_cell.print("\r");

// 5000 msec. for initial comm tmout
// 50 msec. for inter character timeout
switch (WaitResp(5000, 50, "+CPBR"))
	{
	case RX_TMOUT_ERR:
		// response was not received in specific time
		ret_val = -2;
	break;

	case RX_FINISHED_STR_RECV:
		// response in case valid phone number stored:
		// <CR><LF>+CPBR: <index>,<number>,<type>,<text><CR><LF>
		// <CR><LF>OK<CR><LF>
		// \r\n+CPBR: 1,"+79012345678",145,"test1"\r\n\r\nOK\r\n

		// response in case there is not phone number:
		// <CR><LF>OK<CR><LF>
		p_char = strchr((char *)(comm_buf),'"');
		//"+79012345678",145,"test1"\r\n\r\nOK\r\n
		if (p_char != NULL)
			{
			sscanf(p_char, "\"%12[^\"]\",%*[^,],\"%20[^\"]\"", phone_number, phone_name);
			// output value = we have found out phone number string
			ret_val = 1;
			}
	break;

	case RX_FINISHED_STR_NOT_RECV:
		// only OK or ERROR => no phone number
		ret_val = 0; 
	break;
	}

SetCommLineStatus(CLS_FREE);
return (ret_val);
}



/**********************************************************
Method writes phone number string to the specified SIM position

position:     SMS position <1..20>
phone_number: phone number string for the writing

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0

        OK ret val:
        -----------
        0 - phone number was not written
        1 - phone number was written
**********************************************************/
char GSM::WritePhoneNumber(byte position, char *phone_number)
{
  char ret_val = -1;

  if (position == 0) return (-3);
  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  ret_val = 0; // phone number was not written yet
  
  //send: AT+CPBW=XY,"00420123456789"
  // where XY = position,
  //       "00420123456789" = phone number string
  _cell.print("AT+CPBW=");
  _cell.print((int)position);  
  _cell.print(",\"");
  _cell.print(phone_number);
  _cell.print("\"\r");

  // 5000 msec. for initial comm tmout
  // 50 msec. for inter character timeout
  switch (WaitResp(5000, 50, "OK")) {
    case RX_TMOUT_ERR:
      // response was not received in specific time
      break;

    case RX_FINISHED_STR_RECV:
      // response is OK = has been written
      ret_val = 1;
      break;

    case RX_FINISHED_STR_NOT_RECV:
      // other response: e.g. ERROR
      break;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}


/**********************************************************
Method del phone number from the specified SIM position

position:     SMS position <1..20>

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0

        OK ret val:
        -----------
        0 - phone number was not deleted
        1 - phone number was deleted
**********************************************************/
char GSM::DelPhoneNumber(byte position)
{
  char ret_val = -1;

  if (position == 0) return (-3);
  if (CLS_FREE != GetCommLineStatus()) return (ret_val);
  SetCommLineStatus(CLS_ATCMD);
  ret_val = 0; // phone number was not written yet
  
  //send: AT+CPBW=XY
  // where XY = position
  _cell.print("AT+CPBW=");
  _cell.print((int)position);  
  _cell.print("\r");

  // 5000 msec. for initial comm tmout
  // 50 msec. for inter character timeout
  switch (WaitResp(5000, 50, "OK")) {
    case RX_TMOUT_ERR:
      // response was not received in specific time
      break;

    case RX_FINISHED_STR_RECV:
      // response is OK = has been written
      ret_val = 1;
      break;

    case RX_FINISHED_STR_NOT_RECV:
      // other response: e.g. ERROR
      break;
  }

  SetCommLineStatus(CLS_FREE);
  return (ret_val);
}





/**********************************************************
Function compares specified phone number string 
with phone number stored at the specified SIM position

position:       SMS position <1..20>
phone_number:   phone number string which should be compare

return: 
        ERROR ret. val:
        ---------------
        -1 - comm. line to the GSM module is not free
        -2 - GSM module didn't answer in timeout
        -3 - position must be > 0

        OK ret val:
        -----------
        0 - phone numbers are different
        1 - phone numbers are the same


an example of usage:
        if (1 == gsm.ComparePhoneNumber(1, "123456789")) {
          // the phone num. "123456789" is stored on the SIM pos. #1
          // phone number string is copied to the phone_num array
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG phone numbers are the same", 1);
          #endif
        }
        else {
          #ifdef DEBUG_PRINT
            gsm.DebugPrint("DEBUG phone numbers are different", 1);
          #endif
        }
**********************************************************/
char GSM::ComparePhoneNumber(byte position, char *phone_number)
{
  char ret_val = -1;
  char sim_phone_number[20];


  ret_val = 0; // numbers are not the same so far
  if (position == 0) return (-3);
  if (1 == GetPhoneNumber(position, sim_phone_number)) {
    // there is a valid number at the spec. SIM position
    // -------------------------------------------------
    if (0 == strcmp(phone_number, sim_phone_number)) {
      // phone numbers are the same
      // --------------------------
      ret_val = 1;
    }
  }
  return (ret_val);
}

//-----------------------------------------------------



/*******************************************************************************
					������ ������ ��� ������
value:
	msg - ��������� �� ������������ ��������� �������
	msglength - ������ ������
return:
	true	 - ������ �������� �������
	false - ������ �� ��������
*******************************************************************************/
boolean SIMCOM900::Get_CSCA(char* msg, int msglength)
{
gsm.SetCommLineStatus(CLS_ATCMD);

_tf.setTimeout(3);

gsm.SimpleWriteln("AT+CSCA?");

if(_tf.find("+CSCA:"))
	{
	_tf.getString("\"+", "\",", msg, msglength);
	gsm.SetCommLineStatus(CLS_FREE);
	if (msg[0]==0) return false;
	else return true;
	};
gsm.SetCommLineStatus(CLS_FREE);
return false;
};



/*******************************************************************************
				������ ����� ��������� � ��� �����
value:
	msg - ��������� �� ������������ ��������� �������
	msglength - ������ ������
return:
	true	 - ������ �������� �������
	false - ������ �� ��������
*******************************************************************************/
boolean SIMCOM900::Get_OpSoS_name(char* msg, int msglength)
{
gsm.SetCommLineStatus(CLS_ATCMD);

_tf.setTimeout(1);

gsm.SimpleWriteln("AT+CSPN?");

if(_tf.find("+CSPN:"))
	{
	_tf.getString(" \"", "\",", msg, msglength);
	gsm.SetCommLineStatus(CLS_FREE);
	return true;
	};
gsm.SetCommLineStatus(CLS_FREE);
return false;
};



/*******************************************************************************
				������ �������� ������� � ������
value:
	msg - ��������� �� ������������ ��������� �������
	msglength - ������ ������
return:
	true	 - ������ �������� �������
	false - ������ �� ��������
*******************************************************************************/
boolean SIMCOM900::Get_GSM_time(char* msg, int msglength)
{
gsm.SetCommLineStatus(CLS_ATCMD);

_tf.setTimeout(1);

gsm.SimpleWriteln("AT+CCLK?");

if(_tf.find("+CCLK:"))
	{
	_tf.getString(" \"", "\"\r", msg, msglength);
	gsm.SetCommLineStatus(CLS_FREE);
	return true;
	};
gsm.SetCommLineStatus(CLS_FREE);
return false;
};