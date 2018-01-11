/***********************************************************
// 版权声明：xxxxx版权所有
//			  
文 件 名：	BlowFishEnrypt.h
创 建 人：	史云杰
说    明：	BlowFish加密类
***********************************************************/
#ifndef __BLOWFISHENCRYPT_H
#define __BLOWFISHENCRYPT_H

#include "./Encrypt.h"

class CBlowFishEncrypt : public IEncrypt
{
public:
	CBlowFishEncrypt();
	CBlowFishEncrypt( char* apKey, INT aiNum);
	virtual ~CBlowFishEncrypt();
	VOID			InitPassword( char* apKey, INT aiNum);
	virtual INT		Encrypt( char* apInData,unsigned short aiInDataLen,  char* apOutData,unsigned short& aiOutDataLen);
	virtual INT		Decrypt( char* apInData,unsigned short aiInDataLen,  char* apOutData,unsigned short& aiOutDataLen);
private:
	unsigned long Run_Funcion(unsigned long x);
	VOID			en_code(unsigned long *xl, unsigned long *xr);
	VOID			de_code(unsigned long *xl, unsigned long *xr);
private:
	const static INT N = 16;
	//成员变量
	unsigned long	P[N + 2];
	unsigned long	S[4][256];
};

#endif //__BLOWFISHENCRYPT_H