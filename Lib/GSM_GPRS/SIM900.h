#ifndef SIMCOM900_H
#define SIMCOM900_H
#include "NewSoftSerial.h"
#include "GSM.h"



#if defined __cplusplus
  extern "C" {
#endif
int getCCI(char* cci);
int getIMEI(char* imei);
int sendSMS(const char* to, const char* msg);

#if defined __cplusplus
  }
#endif



class SIMCOM900 : public virtual GSM
{

  private:
    int configandwait(char* pin);
    int setPIN(char *pin);
    int changeNSIPmode(char);

  public:
    SIMCOM900();
    ~SIMCOM900();
    int getCCI(char* cci);
	int getIMEI(char* imei);
    int sendSMS(const char* to, const char* msg);
    boolean readSMS_text(char* msg, int msglength, char* number, int nlength);
    boolean readSMS_PDU(char* msg, int msglength, char number);
    boolean Get_CUSD(char* msg, int msglength, char number);
    boolean Send_CUSD(char* msg, int msglength, char* number);
    boolean Get_CSCA(char* msg, int msglength);
    boolean Get_OpSoS_name(char* msg, int msglength);
    boolean readCall(char* number, int nlength);
    boolean call(char* number, unsigned int milliseconds);
	char forceON();
    virtual int read(char* result, int resultlength);
	virtual uint8_t read();
    int readCellData(int &mcc, int &mnc, long &lac, long &cellid);
    void SimpleRead();
    uint8_t find(char *target);
    int getString(char *pre_string, char *post_string, char *buffer, int length);
    void WhileSimpleRead();
    void SimpleWrite(char *comm);
    void SimpleWrite(char const *comm);
    void SimpleWrite(int comm);
    void SimpleWriteln(char *comm);
    void SimpleWriteln(char const *comm);
    void SimpleWriteln(int comm);
    
    boolean Get_GSM_time(char* msg, int msglength);
};

extern SIMCOM900 gsm;

#endif

