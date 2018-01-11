/***********************************************************
// 版权声明：xxxxx版权所有
//			  
文 件 名：	ASCIIEnrypt.h
创 建 人：	史云杰
说    明：	字符串加密类
***********************************************************/
 #ifndef __ASCIIENRYPT_H
 #define __ASCIIENRYPT_H
//-----------------------------------------------------------------------------
#include "encrypt/Encrypt.h"
//-----------------------------------------------------------------------------
class CASCIIEnrypt : public IEncrypt
{
public:
    CASCIIEnrypt();
    CASCIIEnrypt( char* apKey, INT aiNum);
    virtual ~CASCIIEnrypt();
    void 			InitPassword( char* apKey, INT aiNum);
    virtual INT				Encrypt( char* apInData,unsigned short aiInDataLen,  char* apOutData,unsigned short& aiOutDataLen);
    virtual INT				Decrypt( char* apInData,unsigned short aiInDataLen,  char* apOutData,unsigned short& aiOutDataLen);
private:
    unsigned long	inline Run_Funcion(unsigned long x);
    void			en_code(unsigned long *xl, unsigned long *xr);
    void			de_code(unsigned long *xl, unsigned long *xr);

	//base编解码
	INT encode_base64(char *apDest,INT aiDestlen,char *apszStr,INT aiStrlen);
	INT decode_base64(char *apDest,INT apiDestlen,char *apszStr,INT aiStrlen);

	const static INT N = 16;
    //成员变量
    unsigned long	P[N + 2];
    unsigned long	S[4][256];
};

#endif //__ASCIIENRYPT_H
