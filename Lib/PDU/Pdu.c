//КµПЦpdu¶МРЕµД±аВлєНЅвВл
//
//Ц§іЦ7bit,8bit±а/ЅвВлЈ¬І»Ц§іЦUCS2±аВл
//
//2007-5-31 7:30 	+ФЪSM_PARAMЦРФцјУБЛTP_UDlenКфРФЈ¬
//					УГТФ±ЈґжУГ»§КэѕЭі¤¶И
//
//

#include "Pdu.h"




int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
int nDstLength;
char ch;
int i=0;

nDstLength = nSrcLength;

for(i=0; i<nSrcLength; i+=2)
	{
	ch = *pSrc++;
	*pDst++ = *pSrc++;
	*pDst++ = ch;
	}

if(*(pDst-1) == 'F')
	{
	pDst--;
	nDstLength--;
	}

*pDst = '\0';

return nDstLength;
}



// Binary To String converter
// Example {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
// pSrc: Input binary data
// pDst: Output string data
// nSrcLength: Length binary data
// return: Lenth string data
int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
const char tab[]="0123456789ABCDEF";    // 0x0-0xfµДЧЦ·ыІйХТ±н
int i=0;
for(i=0; i<nSrcLength; i++)
	{
	// Convert high tetrad
	*pDst++ = tab[*pSrc >> 4];
	// Convert low tetrad
	*pDst++ = tab[*pSrc & 0x0f];
	pSrc++;
	}
// Set end string
*pDst = '\0';
// return string len
return nSrcLength * 2;
}



// String To Binary converter
// Example {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
// pSrc: Input binary data
// pDst: Output string data
// nSrcLength: Length binary data
// return: Lenth string data
int gsmString2Bytes( unsigned char* pSrc, unsigned char* pDst, int nSrcLength)
{
int i=0;
for(i=0; i<nSrcLength; i+=2)
	{
	if(*pSrc>='0' && *pSrc<='9') *pDst = (*pSrc - '0') << 4;
	else *pDst = (*pSrc - 'A' + 10) << 4;
    
	pSrc++;
    
	if(*pSrc>='0' && *pSrc<='9') *pDst |= *pSrc - '0';
	else *pDst |= *pSrc - 'A' + 10;

	pSrc++;
	pDst++;
	}

return nSrcLength / 2;
}



// 8-bitЅвВл
// pSrc: Фґ±аВлґ®ЦёХл
// pDst: Дї±кЧЦ·ыґ®ЦёХл
// nSrcLength: Фґ±аВлґ®і¤¶И
// ·µ»Ш: Дї±кЧЦ·ыґ®і¤¶И
int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int kk;       
	
	if (nSrcLength < 1)return 0;
	for(kk = 0;kk < nSrcLength;kk++)
	{
		*pDst++ =	*pSrc++;		
	}
    *pDst = '\0';
    
    // ·µ»ШДї±кґ®і¤¶И
    return nSrcLength;
}



// 8-bit±аВл
// pSrc: ФґЧЦ·ыґ®ЦёХл
// pDst: Дї±к±аВлґ®ЦёХл
// nSrcLength: ФґЧЦ·ыґ®і¤¶И
// ·µ»Ш: Дї±к±аВлґ®і¤¶И
int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int kk;       
	
	if (nSrcLength < 1)return 0;
	for(kk = 0;kk < nSrcLength;kk++)
	{
		*pDst++ =	*pSrc++;		
	}
    *pDst = '\0';
    // ·µ»ШДї±кґ®і¤¶И
    return nSrcLength;
}



// 7-bitЅвВл
// pSrc: Фґ±аВлґ®ЦёХл
// pDst: Дї±кЧЦ·ыґ®ЦёХл
// nSrcLength: Фґ±аВлґ®і¤¶И
// ·µ»Ш: Дї±кЧЦ·ыґ®і¤¶И
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int nSrc;        // ФґЧЦ·ыґ®µДјЖКэЦµ
    int nDst;        // Дї±кЅвВлґ®µДјЖКэЦµ
    int nByte;       // µ±З°ХэФЪґ¦АнµДЧйДЪЧЦЅЪµДРтєЕЈ¬·¶О§КЗ0-6
    unsigned char nLeft;    // ЙПТ»ЧЦЅЪІРУаµДКэѕЭ
    
    // јЖКэЦµіхКј»Ї
    nSrc = 0;
    nDst = 0;
    
    // ЧйДЪЧЦЅЪРтєЕєНІРУаКэѕЭіхКј»Ї
    nByte = 0;
    nLeft = 0;
    
    // Ѕ«ФґКэѕЭГї7ёцЧЦЅЪ·ЦОЄТ»ЧйЈ¬ЅвС№ЛхіЙ8ёцЧЦЅЪ
    // С­»·ёГґ¦Ан№эіМЈ¬Ц±ЦБФґКэѕЭ±»ґ¦АнНк
    // Из№ы·ЦЧйІ»µЅ7ЧЦЅЪЈ¬ТІДЬХэИ·ґ¦Ан
    while(nSrc<nSrcLength)
    {
        // Ѕ«ФґЧЦЅЪУТ±ЯІї·ЦУлІРУаКэѕЭПајУЈ¬ИҐµфЧоёЯО»Ј¬µГµЅТ»ёцДї±кЅвВлЧЦЅЪ
        *pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
        // Ѕ«ёГЧЦЅЪКЈПВµДЧу±ЯІї·ЦЈ¬ЧчОЄІРУаКэѕЭ±ЈґжЖрАґ
        nLeft = *pSrc >> (7-nByte);
    
        // РЮёДДї±кґ®µДЦёХлєНјЖКэЦµ
        pDst++;
        nDst++;
    
        // РЮёДЧЦЅЪјЖКэЦµ
        nByte++;
    
        // µЅБЛТ»ЧйµДЧоєуТ»ёцЧЦЅЪ
        if(nByte == 7)
        {
            // ¶оНвµГµЅТ»ёцДї±кЅвВлЧЦЅЪ
            *pDst = nLeft;
    
            // РЮёДДї±кґ®µДЦёХлєНјЖКэЦµ
            pDst++;
            nDst++;
    
            // ЧйДЪЧЦЅЪРтєЕєНІРУаКэѕЭіхКј»Ї
            nByte = 0;
            nLeft = 0;
        }
    
        // РЮёДФґґ®µДЦёХлєНјЖКэЦµ
        pSrc++;
        nSrc++;
    }
    
    *pDst = 0;
    
    // ·µ»ШДї±кґ®і¤¶И
    return nDst;
}



// 7-bit±аВл
// pSrc: ФґЧЦ·ыґ®ЦёХл
// pDst: Дї±к±аВлґ®ЦёХл
// nSrcLength: ФґЧЦ·ыґ®і¤¶И
// ·µ»Ш: Дї±к±аВлґ®і¤¶И
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int nSrc;        // ФґЧЦ·ыґ®µДјЖКэЦµ
    int nDst;        // Дї±к±аВлґ®µДјЖКэЦµ
    int nChar;       // µ±З°ХэФЪґ¦АнµДЧйДЪЧЦ·ыЧЦЅЪµДРтєЕЈ¬·¶О§КЗ0-7
    unsigned char nLeft = 0;    // ЙПТ»ЧЦЅЪІРУаµДКэѕЭ
    
    // јЖКэЦµіхКј»Ї
    nSrc = 0;
    nDst = 0;
    
    //printf("src=%s" , pSrc ) ;
    // Ѕ«Фґґ®Гї8ёцЧЦЅЪ·ЦОЄТ»ЧйЈ¬С№ЛхіЙ7ёцЧЦЅЪ
    // С­»·ёГґ¦Ан№эіМЈ¬Ц±ЦБФґґ®±»ґ¦АнНк
    // Из№ы·ЦЧйІ»µЅ8ЧЦЅЪЈ¬ТІДЬХэИ·ґ¦Ан
    while(nSrc<nSrcLength)
    {
        //printf("nSrc=%d\n" , nSrc );
        
        // ИЎФґЧЦ·ыґ®µДјЖКэЦµµДЧоµН3О»
        nChar = nSrc & 7;
    
        // ґ¦АнФґґ®µДГїёцЧЦЅЪ
        if(nChar == 0)
        {
            // ЧйДЪµЪТ»ёцЧЦЅЪЈ¬Ц»КЗ±ЈґжЖрАґЈ¬ґэґ¦АнПВТ»ёцЧЦЅЪК±К№УГ
            nLeft = *pSrc;
        }
        else
        {
            // ЧйДЪЖдЛьЧЦЅЪЈ¬Ѕ«ЖдУТ±ЯІї·ЦУлІРУаКэѕЭПајУЈ¬µГµЅТ»ёцДї±к±аВлЧЦЅЪ
            *pDst = (*pSrc << (8-nChar)) | nLeft;
    
            // Ѕ«ёГЧЦЅЪКЈПВµДЧу±ЯІї·ЦЈ¬ЧчОЄІРУаКэѕЭ±ЈґжЖрАґ
            nLeft = *pSrc >> nChar;
            // РЮёДДї±кґ®µДЦёХлєНјЖКэЦµ 
            pDst++;
            nDst++; 
        } 
        
        
        // РЮёДФґґ®µДЦёХлєНјЖКэЦµ
        pSrc++; nSrc++;
    }
    
    // ·µ»ШДї±кґ®і¤¶И
    return nDst; 
}



int gsmDecodeUcs2(const unsigned char* pSrc, unsigned char* pDst, int nSrcLength)
{
//int nDstLength=0;
//wchar_t wchar[128];
short wchar;

for(int i=0; i<nSrcLength/2; i++)
	{
	wchar = *pSrc++ << 8;
	wchar|= *pSrc++;
	if ((wchar>=0x410)&&(wchar<=0x44F)) pDst[i]=wchar-0x410+0xC0;	//Диапазон кирилических символов
	else if(wchar==0x451) pDst[i]='ё';							//Быквы ёЁ
	else if(wchar==0x401) pDst[i]='Ё';
	else pDst[i]=wchar&0x7f;									//Латиница
	}
pDst[nSrcLength/2]=0;			//Конец строки

return nSrcLength/2;
}



int gsmEncodeUcs2(const unsigned char* pSrc, unsigned char* pDst, int nSrcLength)
{
for(int i=0; i<nSrcLength; i++)
	{
	//Диапазон кирилических символов
	if (*pSrc>0xC0)
		{
		*pDst++=(unsigned short)(*pSrc+0x410-0xC0)>>8;
		*pDst++=(unsigned short)(*pSrc+0x410-0xC0)&0xff;
		*pSrc++;
		}
	//Быквы ёЁ
	else if (*pSrc=='ё') {*pDst++=0x04; *pDst++=0x51; *pSrc++;}
	else if (*pSrc=='Ё') {*pDst++=0x04; *pDst++=0x01; *pSrc++;}
	//Латиница
	else
		{
		*pDst++=0;
		*pDst++=(*pSrc++&0x7f);
		}
	}
*pDst=0;			//Конец строки

return nSrcLength;
}



// PDU to struct converter
// pSrc: source PDU string
// pDst: output struct data
int gsmDecodePdu( unsigned char* pSrc, SM_PARAM* pDst) 
{
int nDstLength;
unsigned char tmp;
unsigned char buf[256];

//Получим номер SMS-центра отправителя
gsmString2Bytes(pSrc, &tmp, 2);//Преобразуем строку в бинарный вид
tmp = (tmp - 1) * 2;    		// SMSC
pSrc += 4;

//Преобразуем строку в цифровой код телефона сервис центра СМС
gsmSerializeNumbers((char const *)pSrc, pDst->SCA, tmp);
pSrc += tmp;        			// ЦёХлєуТЖ

//TP-MTI & Co (Message Type indicator и компания) – важный байт, состоящий из различных флагов
gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_MTI_CO, 2);
pSrc += 2;

//Получим длинну номера телефона отправителя
gsmString2Bytes(pSrc, &tmp, 2);
//Если длинна номера нечетная, дополним до четности (учтем F)
if(tmp & 1) tmp += 1;
pSrc += 2;
//Получим тип телефона отправителя
unsigned char address_type;
gsmString2Bytes(pSrc, &address_type, 2);
//Получим сам телефон отправителя (TP-Destination-Address)
pSrc += 2;
//Определим какого типа идентификатор телефона
if ((address_type&0xF0)==(0x50|0x80))//0х50 - буквенно-цифровой
	{
	//Преобразуем принятую строку в массив байт
	nDstLength = gsmString2Bytes(pSrc, buf, tmp);
	//Преобразуем из 7битной кодировки в Win1251
	gsmDecode7bit(buf, (char *)pDst->TPA, nDstLength);
	}
else if ((address_type&0xF0)==(0x10|0x80))//0х10 - международный формат
	{
	//Преобразуем строку в цифровой код телефона
	gsmSerializeNumbers((char const *)pSrc, pDst->TPA, tmp);
	}
pSrc += tmp;		//Следующий пареметр

//TP-Protocol ID – Идентификатор протокола.
gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_PID, 2);
pSrc += 2;

//TP-Data-Coding-Scheme – Схема кодирования данных. Указывает, в каком формате представлено сообщение.
gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_DCS, 2);
pSrc += 2;

//TP-Service-Centre-Time-Stamp (TP_SCTS)
gsmSerializeNumbers((char const *)pSrc, pDst->TP_SCTS, 14);
pSrc += 14;

//TP-User-Data-Length – длина сообщения.
gsmString2Bytes(pSrc, &tmp, 2);
pDst->TP_UDlen = tmp;
pSrc += 2;

if (pDst->TP_MTI_CO.TP_UDHI)
	{
	//TP-Service-Centre-Time-Stamp (TP_SCTS)
	gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_UDHL, 2);
	pSrc += 2;
	pSrc += pDst->TP_UDHL*2;
	tmp-=pDst->TP_UDHL;
	}

// 7-bit
if((pDst->TP_DCS&0x0f) == GSM_7BIT)
	{
	//Преобразуем принятую строку в массив байт
	nDstLength = gsmString2Bytes(pSrc, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);
	//Преобразуем из 7битной кодировки в Win1251
	gsmDecode7bit(buf, (char *)pDst->TP_UD, nDstLength);
	nDstLength = tmp;
	}
// UCS2
else if((pDst->TP_DCS&0x0f) == GSM_UCS2)
	{
	//Преобразуем принятую строку в массив байт
	nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);
	//Преобразуем из Unicode в Win1251
	nDstLength = gsmDecodeUcs2(buf, pDst->TP_UD, nDstLength);
	}
// 8-bit
else
	{
	nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);
	nDstLength = gsmDecode8bit(buf, (char *)pDst->TP_UD, nDstLength);
	}

pDst->TP_UDlen = nDstLength;
return nDstLength;
}



// ХэіЈЛіРтµДЧЦ·ыґ®ЧЄ»»ОЄБЅБЅµЯµ№µДЧЦ·ыґ®Ј¬Ифі¤¶ИОЄЖжКэЈ¬І№'F'ґХіЙЕјКэ
// ИзЈє"8613851872468" --> "683158812764F8"
// pSrc: ФґЧЦ·ыґ®ЦёХл
// pDst: Дї±кЧЦ·ыґ®ЦёХл
// nSrcLength: ФґЧЦ·ыґ®і¤¶И
// ·µ»Ш: Дї±кЧЦ·ыґ®і¤¶И
int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;   // Дї±кЧЦ·ыґ®і¤¶И
    char ch;          // УГУЪ±ЈґжТ»ёцЧЦ·ы
    int i=0;
    // ёґЦЖґ®і¤¶И
    nDstLength = nSrcLength;
    
    // БЅБЅµЯµ№
    for(i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;        // ±ЈґжПИіцПЦµДЧЦ·ы
        *pDst++ = *pSrc++;   // ёґЦЖєуіцПЦµДЧЦ·ы
        *pDst++ = ch;        // ёґЦЖПИіцПЦµДЧЦ·ы
    }
    
    // Фґґ®і¤¶ИКЗЖжКэВрЈї
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';     // І№'F'
        nDstLength++;        // Дї±кґ®і¤¶ИјУ1
    }
    
    // КдіцЧЦ·ыґ®јУёцЅбКш·ы
    *pDst = '\0';
    
    // ·µ»ШДї±кЧЦ·ыґ®і¤¶И
    return nDstLength;
}



// PDU converter (from text to PDU)
// pSrc: source struct SM_PARAM data
// pDst: Output PDU data
int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst)
{
int nLength=0;				// ДЪІїУГµДґ®і¤¶И
int nDstLength=0;			// Дї±кPDUґ®і¤¶И
unsigned char buf[DATAMAX+1];	// ДЪІїУГµД»єіеЗш

//SMSCenter (SCA)
nLength = strlen(pSrc->SCA);	//Получаем длинну номера СМС центра
if (nLength)				//Если СМС центр указан явно, передадим через него
	{
	buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	//Если она не четная, добавим 1 (учтем F)
	buf[1] = 0x91;			//Тип номера SMS-центра (международный формат)
	nDstLength = gsmBytes2String(buf, pDst, 2);	//Преобразуем двоичные данные в строку
	nDstLength += gsmInvertNumbers(pSrc->SCA, &pDst[nDstLength], nLength);	//добавим номер телефона смс центра в строку
	}
else
	{
	buf[0] = 0;
	nDstLength = gsmBytes2String(buf, pDst, 1);	//Преобразуем двоичные данные в строку
	}

// TPDU (TPDU = «PDU-Type» + «TP-MR» + «TP-DA» + «TP-PID» + «TP-DCS» + «TP-VP» + «TP-UDL» + «TP-UD»)
nLength = strlen(pSrc->TPA);	//TP-DA - TP-Destination-Address – Номер телефона получателя сообщения
buf[0] = 0x11;				//PDU-Type – Тип сообщения ()
buf[1] = 0;				//TP-MR=0 - TP-Message-Reference
buf[2] = (char)nLength;		//TP-DA - длинна номера телефона получателя
buf[3] = 0x91;				//Тип номера получателя (международный формат)
nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 4);	//добавляем занесенные в buf данные в строку
nDstLength += gsmInvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);	//заносим номер получателя в строку

nLength = pSrc->TP_UDlen ;    		// Длинна текста СМС сообщения в символах
buf[0] = 0 ;        				// TP-PID - TP-Protocol ID – Идентификатор протокола.
buf[1] = pSrc->TP_DCS ;       		// TP-DCS - TP-Data-Coding-Scheme – Схема кодирования данных.
buf[2] = 0;            				// TP-VP  - TP-Validity-Period — время действия сообщения

char encoding=pSrc->TP_DCS&0x0f;
//Auto encoding
if (encoding==GSM_AUTO_ENCODING)
	{
	encoding=GSM_7BIT;				//Default encoding (only latin)
	for (char i=0; i<nLength; i++)
		{
		if (pSrc->TP_UD[i]>0x7f)		//if cirilic symbol
			{
			encoding=GSM_UCS2;
			break;
			}
		}
	}
// UCS2 (unicode 16bit)
if (encoding==GSM_UCS2)
	{
	buf[3] = nLength*2;				//TP-User-Data-Length – длина сообщения.
	//unsigned short UCS2_msg[61];
	gsmBytes2String( buf, &pDst[nDstLength], 4);
	nDstLength += 8;
	gsmEncodeUcs2((unsigned char const *)pSrc->TP_UD, buf, nLength);
	//strcat( &pDst[nDstLength], (char const *) &buf[0]);
	nDstLength +=gsmBytes2String(buf, &pDst[nDstLength], nLength*2);
	//nDstLength += nLength;
	}
// 7-bit
else if (encoding==GSM_7BIT)
	{
	buf[3] = nLength;
	nLength = gsmEncode7bit((char const *)pSrc->TP_UD, &buf[4], nLength+1) + 4;
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], nLength);
	}
// 8-bit
else if (encoding==GSM_8BIT)
	{
	buf[3] = nLength;
	nLength = gsmEncode8bit((char const *)pSrc->TP_UD, &buf[4], nLength) + 4;   
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], nLength);	
	}

//Конец команды
pDst[nDstLength]=0x1a;
pDst[nDstLength+1]='\0';
nLength = strlen(pSrc->SCA);
nLength = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;

return nDstLength / 2 - nLength - 1 ;
}



//Ѕ«ASCIIЧЄ»»ОЄUCS2Вл
//КдИлЈєPsource=ASCIIЧЦ·ыґ®
//		nSrcLength=ASCIIЧЦ·ыґ®µДі¤¶И
//КдіцЈєPdest=UCS2ЧЦ·ыґ®		
//·µ»ШЈєUCS2ЧЦ·ыґ®µДі¤¶И
int ASCII2UCS2(const unsigned char *Psource , unsigned char *Pdest , int nSrcLength )
{
int i ;
unsigned char temp[3] ;

for(i=0; i<nSrcLength; i++, Psource++ )
	{
	*Pdest++='0' ;	
	*Pdest++='0' ;
	gsmBytes2String( Psource, (char *)temp, 1 ) ;		
	*Pdest++= temp[0];
	*Pdest++= temp[1];
	}
*Pdest = '\0' ;
return nSrcLength*4 ;
}



//Ѕ«ЦРОД±кµгЈ¬УўОД±кµгЈ¬КэЧЦµДUCS2ВлЧЄ»»ОЄЧЄ»»ОЄASCIIВл
//КдИлЈєPsource=UCS2ЧЦ·ыґ®
//		nSrcLength=UCS2ЧЦ·ыґ®µДі¤¶И
//КдіцЈєPdest=ASCIIЧЦ·ыґ®		
//·µ»ШЈє-1=є¬УР№ж¶ЁЦ®НвµДЧЦ·ыЈ¬>-1=ASCIIЧЦ·ыґ®µДі¤¶И
int UCS22ACSII(const unsigned char *Psource , unsigned char *Pdest , int nSrcLength ) 
{
int i;

for(i=0; i<nSrcLength; i+=4, Psource+=4, Pdest++ )
	{
	if      ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='0'))*Pdest = '0' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='1'))*Pdest = '1' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='2'))*Pdest = '2' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='3'))*Pdest = '3' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='4'))*Pdest = '4' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='5'))*Pdest = '5' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='6'))*Pdest = '6' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='7'))*Pdest = '7' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='8'))*Pdest = '8' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='9'))*Pdest = '9' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='A'))*Pdest = ':' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='B'))*Pdest = ';' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='2')&&(*(Psource+3)=='1'))*Pdest = '!' ;
	else if ((*Psource=='0')&&(*(Psource+1)=='0')&&(*(Psource+2)=='3')&&(*(Psource+3)=='F'))*Pdest = '?' ;
	else if ((*Psource=='F')&&(*(Psource+1)=='F')&&(*(Psource+2)=='1')&&(*(Psource+3)=='F'))*Pdest = '?' ;
	else if ((*Psource=='F')&&(*(Psource+1)=='F')&&(*(Psource+2)=='1')&&(*(Psource+3)=='A'))*Pdest = ':' ;
	else if ((*Psource=='F')&&(*(Psource+1)=='F')&&(*(Psource+2)=='0')&&(*(Psource+3)=='1'))*Pdest = '!' ;
	else if ((*Psource=='F')&&(*(Psource+1)=='F')&&(*(Psource+2)=='1')&&(*(Psource+3)=='B'))*Pdest = ';' ;
	else return -1 ;	
	}
*Pdest = '\0';
return nSrcLength / 4;
}




