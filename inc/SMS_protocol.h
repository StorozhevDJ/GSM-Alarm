

//extern SMSGSM sms;
extern SM_PARAM MySM_PARAM;
extern char smsbuffer[];

extern long sms_new_timeout;

char SendSms(SM_PARAM *MySM_PARAM);
void AlarmSendSMS(char * str, char flash_type, char sms_type);

void SendMajachok(char * phone_num, char majachok_number);
bool SMS_protocol_check (void);