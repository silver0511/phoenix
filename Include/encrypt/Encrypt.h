/***********************************************************
//xxxxx版权所有
//  
文 件 名：	CEncrypt.h
创 建 人：	史云杰
说    明：	加密类
***********************************************************/
 #ifndef __CENCRYPT_H
 #define __CENCRYPT_H
 
#include "base0/platform.h"
#include "base0/u9_string.h"

class IEncrypt
{
public:
    virtual INT	Encrypt( char* apInData,unsigned short aiInDataLen,  char* apOutData,unsigned short& aiOutDataLen) = 0;
    virtual INT	Decrypt( char* apInData,unsigned short aiInDataLen,  char* apOutData,unsigned short& aiOutDataLen) = 0;
};

IEncrypt* CreateEncrypt(U9_CHAR* apEncryptName);
void ReleaseEncrypt(IEncrypt* apEncrypt);

#endif //__CENCRYPT_H
