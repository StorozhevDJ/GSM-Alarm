
#include <stdio.h>
#include <string.h>

#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8
#define GSM_AUTO_ENCODING 0x0f
#define GSM_FLASH		0x10
#define GSM_NO_FLASH	0x00

#define DATAMAX		320

#ifdef __cplusplus
 extern "C" {
#endif
 
 

typedef struct
	{
	char SCA[16];       //номер SMS-центра
	struct
		{
		char TP_MTI:2;	//Message Type indicator (TP-MTI)
		char TP_MMS:1;	//More messages to send (TP-MMS)
		char reserved:2;
		char TP_SRI:1;	//Status report indication (TP-SRI)
		char TP_UDHI:1;//User data header information (TP-UDHI)
		char TP_RP:1;	//Reply path (TP-RP)
		}TP_MTI_CO;	//TP-MTI & Co (Message Type indicator и компания)
	char TPA[16];       //TP-Destination-Address – Номер телефона получателя сообщения
	char TP_PID;        //TP-Protocol ID – Идентификатор протокола. Особого интереса не представляет. Тоже всегда равен «00»
	char TP_DCS;        //TP-Data-Coding-Scheme – Схема кодирования данных. Указывает, в каком формате представлено сообщение.
	char TP_SCTS[16];   //TimeStamp -  Время приема сервис центром СМС сообщения
	char TP_UDHL;
	struct
		{
		char IEI;		//Для отправки длинных сообщений всегда равен либо 0x00, либо 0x08. Если 0x00 – то IED1 занимает 1 байт, если 0x08 – то IED1 занимает 2 байта.
		char IEDL;	//0x03 или 0x04	Длина данных информационного элемента – 3 или 4 байта, в зависимости от длины поля IED1
		char IED1[2];	//1 или 2 байта	Номер-ссылка. Должен быть одинаков для всех частей сообщения
		char IED2;	//Количество частей в сообщении
		char IED3;	//Порядковый номер части сообщения
		}TP_UDH;
	unsigned char TP_UD[DATAMAX+1];    //TP-User-Data – непосредственно текст SMS-сообщения, закодированный согласно полю TP-DCS
	int  TP_UDlen;		//TP-User-Data-Length – длина сообщения.
	char index;		// ¶МПыПўРтєЕЈ¬ФЪ¶БИЎК±УГµЅ
	} SM_PARAM;



int gsmEncodeUcs2(const unsigned char* pSrc, unsigned char* pDst, int nSrcLength);
int gsmDecodeUcs2(const unsigned char* pSrc, unsigned char* pDst, int nSrcLength);
int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength);
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength);
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
int gsmString2Bytes(unsigned char* pSrc, unsigned char* pDst, int nSrcLength) ;
int gsmDecodePdu (unsigned char* pSrc, SM_PARAM* pDst);
int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength) ;
int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst);
int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength) ;
void StringToWideChar(char *source , char *dest) ;
int ASCII2UCS2(const unsigned char *Psource , unsigned char *Pdest , int nSrcLength ) ;

#ifdef __cplusplus
}
#endif


/*
Modem Revision:1137B09SIM900M64_ST
SMSC=+79037011111


07	//Length of the SMSC information (in this case 7 octets)
91	//Type-of-address of the SMSC. (91 means international format of the phone number)
9730071111F1	//Service center number(in decimal semi-octets). The length of the phone number is odd (11), so a trailing F has been added to form proper octets. The phone number of this service center is "+27381000015". See below.
40	//First octet of this SMS-DELIVER message.
0D	//Address-Length. Length of the sender number (0B hex = 11 dec)
D0	//Type-of-address of the sender number
C2629199741601	//Sender number (decimal semi-octets), with a trailing F
00	//TP-PID. Protocol identifier.
08	//TP-DCS Data coding scheme (7BIT=0, 8BIT=4, UCS2=8)
11119251024561	//TP-SCTS. Time stamp (semi-octets)
8B	//TP-UDL. User data length, length of message
06080490A703010412044B00200432043E0441043F043E043B044C0437043E04320430043B043804
41044C00200418043D044204350440043D04350442002F005700410050002F004D004D0053002004
410020041D0415043D0430044104420440043E0435043D043D043E0433043E002004420435043B04
350444043E043D0430002E0020041D0435043E

*/