//实现pdu短信的编码和解码
//
//支持7bit,8bit编/解码，不支持UCS2编码
//
//2007-5-31 7:30 	+在SM_PARAM中增加了TP_UDlen属性，
//					用以保存用户数据长度
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
const char tab[]="0123456789ABCDEF";    // 0x0-0xf的字符查找表
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



// 8-bit解码
// pSrc: 源编码串指针
// pDst: 目标字符串指针
// nSrcLength: 源编码串长度
// 返回: 目标字符串长度
int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int kk;       
	
	if (nSrcLength < 1)return 0;
	for(kk = 0;kk < nSrcLength;kk++)
	{
		*pDst++ =	*pSrc++;		
	}
    *pDst = '\0';
    
    // 返回目标串长度
    return nSrcLength;
}



// 8-bit编码
// pSrc: 源字符串指针
// pDst: 目标编码串指针
// nSrcLength: 源字符串长度
// 返回: 目标编码串长度
int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int kk;       
	
	if (nSrcLength < 1)return 0;
	for(kk = 0;kk < nSrcLength;kk++)
	{
		*pDst++ =	*pSrc++;		
	}
    *pDst = '\0';
    // 返回目标串长度
    return nSrcLength;
}



// 7-bit解码
// pSrc: 源编码串指针
// pDst: 目标字符串指针
// nSrcLength: 源编码串长度
// 返回: 目标字符串长度
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int nSrc;        // 源字符串的计数值
    int nDst;        // 目标解码串的计数值
    int nByte;       // 当前正在处理的组内字节的序号，范围是0-6
    unsigned char nLeft;    // 上一字节残余的数据
    
    // 计数值初始化
    nSrc = 0;
    nDst = 0;
    
    // 组内字节序号和残余数据初始化
    nByte = 0;
    nLeft = 0;
    
    // 将源数据每7个字节分为一组，解压缩成8个字节
    // 循环该处理过程，直至源数据被处理完
    // 如果分组不到7字节，也能正确处理
    while(nSrc<nSrcLength)
    {
        // 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
        *pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
        // 将该字节剩下的左边部分，作为残余数据保存起来
        nLeft = *pSrc >> (7-nByte);
    
        // 修改目标串的指针和计数值
        pDst++;
        nDst++;
    
        // 修改字节计数值
        nByte++;
    
        // 到了一组的最后一个字节
        if(nByte == 7)
        {
            // 额外得到一个目标解码字节
            *pDst = nLeft;
    
            // 修改目标串的指针和计数值
            pDst++;
            nDst++;
    
            // 组内字节序号和残余数据初始化
            nByte = 0;
            nLeft = 0;
        }
    
        // 修改源串的指针和计数值
        pSrc++;
        nSrc++;
    }
    
    *pDst = 0;
    
    // 返回目标串长度
    return nDst;
}



// 7-bit编码
// pSrc: 源字符串指针
// pDst: 目标编码串指针
// nSrcLength: 源字符串长度
// 返回: 目标编码串长度
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int nSrc;        // 源字符串的计数值
    int nDst;        // 目标编码串的计数值
    int nChar;       // 当前正在处理的组内字符字节的序号，范围是0-7
    unsigned char nLeft = 0;    // 上一字节残余的数据
    
    // 计数值初始化
    nSrc = 0;
    nDst = 0;
    
    //printf("src=%s" , pSrc ) ;
    // 将源串每8个字节分为一组，压缩成7个字节
    // 循环该处理过程，直至源串被处理完
    // 如果分组不到8字节，也能正确处理
    while(nSrc<nSrcLength)
    {
        //printf("nSrc=%d\n" , nSrc );
        
        // 取源字符串的计数值的最低3位
        nChar = nSrc & 7;
    
        // 处理源串的每个字节
        if(nChar == 0)
        {
            // 组内第一个字节，只是保存起来，待处理下一个字节时使用
            nLeft = *pSrc;
        }
        else
        {
            // 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
            *pDst = (*pSrc << (8-nChar)) | nLeft;
    
            // 将该字节剩下的左边部分，作为残余数据保存起来
            nLeft = *pSrc >> nChar;
            // 修改目标串的指针和计数值 
            pDst++;
            nDst++; 
        } 
        
        
        // 修改源串的指针和计数值
        pSrc++; nSrc++;
    }
    
    // 返回目标串长度
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
	if ((wchar>=0x410)&&(wchar<=0x44F)) pDst[i]=wchar-0x410+0xC0;	//蔫囡噻铐 觇痂腓麇耜桴 耔焘铍钼
	else if(wchar==0x451) pDst[i]='�';							//聋赈� 辅
	else if(wchar==0x401) pDst[i]='�';
	else pDst[i]=wchar&0x7f;									//肃蜩龛鲟
	}
pDst[nSrcLength/2]=0;			//暑礤� 耱痤觇

return nSrcLength/2;
}



int gsmEncodeUcs2(const unsigned char* pSrc, unsigned char* pDst, int nSrcLength)
{
for(int i=0; i<nSrcLength; i++)
	{
	//蔫囡噻铐 觇痂腓麇耜桴 耔焘铍钼
	if (*pSrc>0xC0)
		{
		*pDst++=(unsigned short)(*pSrc+0x410-0xC0)>>8;
		*pDst++=(unsigned short)(*pSrc+0x410-0xC0)&0xff;
		*pSrc++;
		}
	//聋赈� 辅
	else if (*pSrc=='�') {*pDst++=0x04; *pDst++=0x51; *pSrc++;}
	else if (*pSrc=='�') {*pDst++=0x04; *pDst++=0x01; *pSrc++;}
	//肃蜩龛鲟
	else
		{
		*pDst++=0;
		*pDst++=(*pSrc++&0x7f);
		}
	}
*pDst=0;			//暑礤� 耱痤觇

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

//项塍麒� 眍戾� SMS-鲥眚疣 铗镳噔栩咫�
gsmString2Bytes(pSrc, &tmp, 2);//橡孱狃噻箦� 耱痤牦 � 徼磬痦 忤�
tmp = (tmp - 1) * 2;    		// SMSC
pSrc += 4;

//橡孱狃噻箦� 耱痤牦 � 鲨麴钼铋 觐� 蝈脲纛磬 皴疴桉 鲥眚疣 烟�
gsmSerializeNumbers((char const *)pSrc, pDst->SCA, tmp);
pSrc += tmp;        			// 指针后移

//TP-MTI & Co (Message Type indicator � 觐祜囗��) � 忄骓 徉轵, 耦耱��� 桤 疣珉梓睇� 綦嚆钼
gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_MTI_CO, 2);
pSrc += 2;

//项塍麒� 潆桧眢 眍戾疣 蝈脲纛磬 铗镳噔栩咫�
gsmString2Bytes(pSrc, &tmp, 2);
//篷腓 潆桧磬 眍戾疣 礤麇蝽��, 漕镱腠桁 漕 麇蝽铖蜩 (篦蝈� F)
if(tmp & 1) tmp += 1;
pSrc += 2;
//项塍麒� 蜩� 蝈脲纛磬 铗镳噔栩咫�
unsigned char address_type;
gsmString2Bytes(pSrc, &address_type, 2);
//项塍麒� 襦� 蝈脲纛� 铗镳噔栩咫� (TP-Destination-Address)
pSrc += 2;
//物疱溴腓� 赅觐泐 蜩镟 桎屙蜩翳赅蝾� 蝈脲纛磬
if ((address_type&0xF0)==(0x50|0x80))//0�50 - 狍赈屙眍-鲨麴钼铋
	{
	//橡孱狃噻箦� 镳桧�蝮� 耱痤牦 � 爨耨桠 徉轵
	nDstLength = gsmString2Bytes(pSrc, buf, tmp);
	//橡孱狃噻箦� 桤 7徼蝽铋 觐滂痤怅� � Win1251
	gsmDecode7bit(buf, (char *)pDst->TPA, nDstLength);
	}
else if ((address_type&0xF0)==(0x10|0x80))//0�10 - 戾驿箜囵钿睇� 纛痨囹
	{
	//橡孱狃噻箦� 耱痤牦 � 鲨麴钼铋 觐� 蝈脲纛磬
	gsmSerializeNumbers((char const *)pSrc, pDst->TPA, tmp);
	}
pSrc += tmp;		//央邃簋� 镟疱戾蝠

//TP-Protocol ID � 蠕屙蜩翳赅蝾� 镳铗铌铍�.
gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_PID, 2);
pSrc += 2;

//TP-Data-Coding-Scheme � 氧屐� 觐滂痤忄龛� 溧眄. 雨噻噱�, � 赅觐� 纛痨囹� 镳邃耱噔脲眍 耦钺龛�.
gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_DCS, 2);
pSrc += 2;

//TP-Service-Centre-Time-Stamp (TP_SCTS)
gsmSerializeNumbers((char const *)pSrc, pDst->TP_SCTS, 14);
pSrc += 14;

//TP-User-Data-Length � 潆桧� 耦钺龛�.
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
	//橡孱狃噻箦� 镳桧�蝮� 耱痤牦 � 爨耨桠 徉轵
	nDstLength = gsmString2Bytes(pSrc, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);
	//橡孱狃噻箦� 桤 7徼蝽铋 觐滂痤怅� � Win1251
	gsmDecode7bit(buf, (char *)pDst->TP_UD, nDstLength);
	nDstLength = tmp;
	}
// UCS2
else if((pDst->TP_DCS&0x0f) == GSM_UCS2)
	{
	//橡孱狃噻箦� 镳桧�蝮� 耱痤牦 � 爨耨桠 徉轵
	nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);
	//橡孱狃噻箦� 桤 Unicode � Win1251
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



// 正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
// 如："8613851872468" --> "683158812764F8"
// pSrc: 源字符串指针
// pDst: 目标字符串指针
// nSrcLength: 源字符串长度
// 返回: 目标字符串长度
int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;   // 目标字符串长度
    char ch;          // 用于保存一个字符
    int i=0;
    // 复制串长度
    nDstLength = nSrcLength;
    
    // 两两颠倒
    for(i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;        // 保存先出现的字符
        *pDst++ = *pSrc++;   // 复制后出现的字符
        *pDst++ = ch;        // 复制先出现的字符
    }
    
    // 源串长度是奇数吗？
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';     // 补'F'
        nDstLength++;        // 目标串长度加1
    }
    
    // 输出字符串加个结束符
    *pDst = '\0';
    
    // 返回目标字符串长度
    return nDstLength;
}



// PDU converter (from text to PDU)
// pSrc: source struct SM_PARAM data
// pDst: Output PDU data
int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst)
{
int nLength=0;				// 内部用的串长度
int nDstLength=0;			// 目标PDU串长度
unsigned char buf[DATAMAX+1];	// 内部用的缓冲区

//SMSCenter (SCA)
nLength = strlen(pSrc->SCA);	//项塍鬣屐 潆桧眢 眍戾疣 烟� 鲥眚疣
if (nLength)				//篷腓 烟� 鲥眚� 箨噻囗 �忭�, 镥疱溧滂� 麇疱� 礤泐
	{
	buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	//篷腓 铐� 礤 麇蝽��, 漕徉忤� 1 (篦蝈� F)
	buf[1] = 0x91;			//诣� 眍戾疣 SMS-鲥眚疣 (戾驿箜囵钿睇� 纛痨囹)
	nDstLength = gsmBytes2String(buf, pDst, 2);	//橡孱狃噻箦� 溻铊黜 溧眄 � 耱痤牦
	nDstLength += gsmInvertNumbers(pSrc->SCA, &pDst[nDstLength], nLength);	//漕徉忤� 眍戾� 蝈脲纛磬 耢� 鲥眚疣 � 耱痤牦
	}
else
	{
	buf[0] = 0;
	nDstLength = gsmBytes2String(buf, pDst, 1);	//橡孱狃噻箦� 溻铊黜 溧眄 � 耱痤牦
	}

// TPDU (TPDU = 玃DU-Type� + 玊P-MR� + 玊P-DA� + 玊P-PID� + 玊P-DCS� + 玊P-VP� + 玊P-UDL� + 玊P-UD�)
nLength = strlen(pSrc->TPA);	//TP-DA - TP-Destination-Address � 皖戾� 蝈脲纛磬 镱塍鬣蝈�� 耦钺龛�
buf[0] = 0x11;				//PDU-Type � 诣� 耦钺龛� ()
buf[1] = 0;				//TP-MR=0 - TP-Message-Reference
buf[2] = (char)nLength;		//TP-DA - 潆桧磬 眍戾疣 蝈脲纛磬 镱塍鬣蝈��
buf[3] = 0x91;				//诣� 眍戾疣 镱塍鬣蝈�� (戾驿箜囵钿睇� 纛痨囹)
nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 4);	//漕徉怆�屐 玎礤皴眄 � buf 溧眄 � 耱痤牦
nDstLength += gsmInvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);	//玎眍耔� 眍戾� 镱塍鬣蝈�� � 耱痤牦

nLength = pSrc->TP_UDlen ;    		// 碾桧磬 蝈犟蜞 烟� 耦钺龛� � 耔焘铍圊
buf[0] = 0 ;        				// TP-PID - TP-Protocol ID � 蠕屙蜩翳赅蝾� 镳铗铌铍�.
buf[1] = pSrc->TP_DCS ;       		// TP-DCS - TP-Data-Coding-Scheme � 氧屐� 觐滂痤忄龛� 溧眄.
buf[2] = 0;            				// TP-VP  - TP-Validity-Period � 怵屐� 溴轳蜮�� 耦钺龛�

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
	buf[3] = nLength*2;				//TP-User-Data-Length � 潆桧� 耦钺龛�.
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

//暑礤� 觐爨礓�
pDst[nDstLength]=0x1a;
pDst[nDstLength+1]='\0';
nLength = strlen(pSrc->SCA);
nLength = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;

return nDstLength / 2 - nLength - 1 ;
}



//将ASCII转换为UCS2码
//输入：Psource=ASCII字符串
//		nSrcLength=ASCII字符串的长度
//输出：Pdest=UCS2字符串		
//返回：UCS2字符串的长度
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



//将中文标点，英文标点，数字的UCS2码转换为转换为ASCII码
//输入：Psource=UCS2字符串
//		nSrcLength=UCS2字符串的长度
//输出：Pdest=ASCII字符串		
//返回：-1=含有规定之外的字符，>-1=ASCII字符串的长度
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




