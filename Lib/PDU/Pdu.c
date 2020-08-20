//ʵ��pdu���ŵı���ͽ���
//
//֧��7bit,8bit��/���룬��֧��UCS2����
//
//2007-5-31 7:30 	+��SM_PARAM��������TP_UDlen���ԣ�
//					���Ա����û����ݳ���
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
const char tab[]="0123456789ABCDEF";    // 0x0-0xf���ַ����ұ�
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



// 8-bit����
// pSrc: Դ���봮ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ���봮����
// ����: Ŀ���ַ�������
int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int kk;       
	
	if (nSrcLength < 1)return 0;
	for(kk = 0;kk < nSrcLength;kk++)
	{
		*pDst++ =	*pSrc++;		
	}
    *pDst = '\0';
    
    // ����Ŀ�괮����
    return nSrcLength;
}



// 8-bit����
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ����봮ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ����봮����
int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int kk;       
	
	if (nSrcLength < 1)return 0;
	for(kk = 0;kk < nSrcLength;kk++)
	{
		*pDst++ =	*pSrc++;		
	}
    *pDst = '\0';
    // ����Ŀ�괮����
    return nSrcLength;
}



// 7-bit����
// pSrc: Դ���봮ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ���봮����
// ����: Ŀ���ַ�������
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int nSrc;        // Դ�ַ����ļ���ֵ
    int nDst;        // Ŀ����봮�ļ���ֵ
    int nByte;       // ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6
    unsigned char nLeft;    // ��һ�ֽڲ��������
    
    // ����ֵ��ʼ��
    nSrc = 0;
    nDst = 0;
    
    // �����ֽ���źͲ������ݳ�ʼ��
    nByte = 0;
    nLeft = 0;
    
    // ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�
    // ѭ���ô�����̣�ֱ��Դ���ݱ�������
    // ������鲻��7�ֽڣ�Ҳ����ȷ����
    while(nSrc<nSrcLength)
    {
        // ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�
        *pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
        // �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
        nLeft = *pSrc >> (7-nByte);
    
        // �޸�Ŀ�괮��ָ��ͼ���ֵ
        pDst++;
        nDst++;
    
        // �޸��ֽڼ���ֵ
        nByte++;
    
        // ����һ������һ���ֽ�
        if(nByte == 7)
        {
            // ����õ�һ��Ŀ������ֽ�
            *pDst = nLeft;
    
            // �޸�Ŀ�괮��ָ��ͼ���ֵ
            pDst++;
            nDst++;
    
            // �����ֽ���źͲ������ݳ�ʼ��
            nByte = 0;
            nLeft = 0;
        }
    
        // �޸�Դ����ָ��ͼ���ֵ
        pSrc++;
        nSrc++;
    }
    
    *pDst = 0;
    
    // ����Ŀ�괮����
    return nDst;
}



// 7-bit����
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ����봮ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ����봮����
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int nSrc;        // Դ�ַ����ļ���ֵ
    int nDst;        // Ŀ����봮�ļ���ֵ
    int nChar;       // ��ǰ���ڴ���������ַ��ֽڵ���ţ���Χ��0-7
    unsigned char nLeft = 0;    // ��һ�ֽڲ��������
    
    // ����ֵ��ʼ��
    nSrc = 0;
    nDst = 0;
    
    //printf("src=%s" , pSrc ) ;
    // ��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
    // ѭ���ô�����̣�ֱ��Դ����������
    // ������鲻��8�ֽڣ�Ҳ����ȷ����
    while(nSrc<nSrcLength)
    {
        //printf("nSrc=%d\n" , nSrc );
        
        // ȡԴ�ַ����ļ���ֵ�����3λ
        nChar = nSrc & 7;
    
        // ����Դ����ÿ���ֽ�
        if(nChar == 0)
        {
            // ���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
            nLeft = *pSrc;
        }
        else
        {
            // ���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
            *pDst = (*pSrc << (8-nChar)) | nLeft;
    
            // �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
            nLeft = *pSrc >> nChar;
            // �޸�Ŀ�괮��ָ��ͼ���ֵ 
            pDst++;
            nDst++; 
        } 
        
        
        // �޸�Դ����ָ��ͼ���ֵ
        pSrc++; nSrc++;
    }
    
    // ����Ŀ�괮����
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
	if ((wchar>=0x410)&&(wchar<=0x44F)) pDst[i]=wchar-0x410+0xC0;	//�������� ������������ ��������
	else if(wchar==0x451) pDst[i]='�';							//����� ��
	else if(wchar==0x401) pDst[i]='�';
	else pDst[i]=wchar&0x7f;									//��������
	}
pDst[nSrcLength/2]=0;			//����� ������

return nSrcLength/2;
}



int gsmEncodeUcs2(const unsigned char* pSrc, unsigned char* pDst, int nSrcLength)
{
for(int i=0; i<nSrcLength; i++)
	{
	//�������� ������������ ��������
	if (*pSrc>0xC0)
		{
		*pDst++=(unsigned short)(*pSrc+0x410-0xC0)>>8;
		*pDst++=(unsigned short)(*pSrc+0x410-0xC0)&0xff;
		*pSrc++;
		}
	//����� ��
	else if (*pSrc=='�') {*pDst++=0x04; *pDst++=0x51; *pSrc++;}
	else if (*pSrc=='�') {*pDst++=0x04; *pDst++=0x01; *pSrc++;}
	//��������
	else
		{
		*pDst++=0;
		*pDst++=(*pSrc++&0x7f);
		}
	}
*pDst=0;			//����� ������

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

//������� ����� SMS-������ �����������
gsmString2Bytes(pSrc, &tmp, 2);//����������� ������ � �������� ���
tmp = (tmp - 1) * 2;    		// SMSC
pSrc += 4;

//����������� ������ � �������� ��� �������� ������ ������ ���
gsmSerializeNumbers((char const *)pSrc, pDst->SCA, tmp);
pSrc += tmp;        			// ָ�����

//TP-MTI & Co (Message Type indicator � ��������) � ������ ����, ��������� �� ��������� ������
gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_MTI_CO, 2);
pSrc += 2;

//������� ������ ������ �������� �����������
gsmString2Bytes(pSrc, &tmp, 2);
//���� ������ ������ ��������, �������� �� �������� (����� F)
if(tmp & 1) tmp += 1;
pSrc += 2;
//������� ��� �������� �����������
unsigned char address_type;
gsmString2Bytes(pSrc, &address_type, 2);
//������� ��� ������� ����������� (TP-Destination-Address)
pSrc += 2;
//��������� ������ ���� ������������� ��������
if ((address_type&0xF0)==(0x50|0x80))//0�50 - ��������-��������
	{
	//����������� �������� ������ � ������ ����
	nDstLength = gsmString2Bytes(pSrc, buf, tmp);
	//����������� �� 7������ ��������� � Win1251
	gsmDecode7bit(buf, (char *)pDst->TPA, nDstLength);
	}
else if ((address_type&0xF0)==(0x10|0x80))//0�10 - ������������� ������
	{
	//����������� ������ � �������� ��� ��������
	gsmSerializeNumbers((char const *)pSrc, pDst->TPA, tmp);
	}
pSrc += tmp;		//��������� ��������

//TP-Protocol ID � ������������� ���������.
gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_PID, 2);
pSrc += 2;

//TP-Data-Coding-Scheme � ����� ����������� ������. ���������, � ����� ������� ������������ ���������.
gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_DCS, 2);
pSrc += 2;

//TP-Service-Centre-Time-Stamp (TP_SCTS)
gsmSerializeNumbers((char const *)pSrc, pDst->TP_SCTS, 14);
pSrc += 14;

//TP-User-Data-Length � ����� ���������.
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
	//����������� �������� ������ � ������ ����
	nDstLength = gsmString2Bytes(pSrc, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);
	//����������� �� 7������ ��������� � Win1251
	gsmDecode7bit(buf, (char *)pDst->TP_UD, nDstLength);
	nDstLength = tmp;
	}
// UCS2
else if((pDst->TP_DCS&0x0f) == GSM_UCS2)
	{
	//����������� �������� ������ � ������ ����
	nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);
	//����������� �� Unicode � Win1251
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



// ����˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
// �磺"8613851872468" --> "683158812764F8"
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ���ַ�������
int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;   // Ŀ���ַ�������
    char ch;          // ���ڱ���һ���ַ�
    int i=0;
    // ���ƴ�����
    nDstLength = nSrcLength;
    
    // �����ߵ�
    for(i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;        // �����ȳ��ֵ��ַ�
        *pDst++ = *pSrc++;   // ���ƺ���ֵ��ַ�
        *pDst++ = ch;        // �����ȳ��ֵ��ַ�
    }
    
    // Դ��������������
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';     // ��'F'
        nDstLength++;        // Ŀ�괮���ȼ�1
    }
    
    // ����ַ����Ӹ�������
    *pDst = '\0';
    
    // ����Ŀ���ַ�������
    return nDstLength;
}



// PDU converter (from text to PDU)
// pSrc: source struct SM_PARAM data
// pDst: Output PDU data
int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst)
{
int nLength=0;				// �ڲ��õĴ�����
int nDstLength=0;			// Ŀ��PDU������
unsigned char buf[DATAMAX+1];	// �ڲ��õĻ�����

//SMSCenter (SCA)
nLength = strlen(pSrc->SCA);	//�������� ������ ������ ��� ������
if (nLength)				//���� ��� ����� ������ ����, ��������� ����� ����
	{
	buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	//���� ��� �� ������, ������� 1 (����� F)
	buf[1] = 0x91;			//��� ������ SMS-������ (������������� ������)
	nDstLength = gsmBytes2String(buf, pDst, 2);	//����������� �������� ������ � ������
	nDstLength += gsmInvertNumbers(pSrc->SCA, &pDst[nDstLength], nLength);	//������� ����� �������� ��� ������ � ������
	}
else
	{
	buf[0] = 0;
	nDstLength = gsmBytes2String(buf, pDst, 1);	//����������� �������� ������ � ������
	}

// TPDU (TPDU = �PDU-Type� + �TP-MR� + �TP-DA� + �TP-PID� + �TP-DCS� + �TP-VP� + �TP-UDL� + �TP-UD�)
nLength = strlen(pSrc->TPA);	//TP-DA - TP-Destination-Address � ����� �������� ���������� ���������
buf[0] = 0x11;				//PDU-Type � ��� ��������� ()
buf[1] = 0;				//TP-MR=0 - TP-Message-Reference
buf[2] = (char)nLength;		//TP-DA - ������ ������ �������� ����������
buf[3] = 0x91;				//��� ������ ���������� (������������� ������)
nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 4);	//��������� ���������� � buf ������ � ������
nDstLength += gsmInvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);	//������� ����� ���������� � ������

nLength = pSrc->TP_UDlen ;    		// ������ ������ ��� ��������� � ��������
buf[0] = 0 ;        				// TP-PID - TP-Protocol ID � ������������� ���������.
buf[1] = pSrc->TP_DCS ;       		// TP-DCS - TP-Data-Coding-Scheme � ����� ����������� ������.
buf[2] = 0;            				// TP-VP  - TP-Validity-Period � ����� �������� ���������

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
	buf[3] = nLength*2;				//TP-User-Data-Length � ����� ���������.
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

//����� �������
pDst[nDstLength]=0x1a;
pDst[nDstLength+1]='\0';
nLength = strlen(pSrc->SCA);
nLength = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;

return nDstLength / 2 - nLength - 1 ;
}



//��ASCIIת��ΪUCS2��
//���룺Psource=ASCII�ַ���
//		nSrcLength=ASCII�ַ����ĳ���
//�����Pdest=UCS2�ַ���		
//���أ�UCS2�ַ����ĳ���
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



//�����ı�㣬Ӣ�ı�㣬���ֵ�UCS2��ת��Ϊת��ΪASCII��
//���룺Psource=UCS2�ַ���
//		nSrcLength=UCS2�ַ����ĳ���
//�����Pdest=ASCII�ַ���		
//���أ�-1=���й涨֮����ַ���>-1=ASCII�ַ����ĳ���
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




