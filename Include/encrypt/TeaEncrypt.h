/***********************************************************
// 版权声明：xxxxx版权所有
//			  
文 件 名：	BlowFishEnrypt.h
创 建 人：	史云杰
说    明：	TeaFish加密类
***********************************************************/
#ifndef __TEAENCRYPT_H
#define __TEAENCRYPT_H
#include "encrypt/Encrypt.h"

class CTeaEncrypt : public IEncrypt
{
public:
	CTeaEncrypt();
	virtual ~CTeaEncrypt();
	INT				Encrypt( char* apInData,unsigned short aiInDataLen,  char* apOutData,unsigned short& aiOutDataLen);
	INT				Decrypt( char* apInData,unsigned short aiInDataLen,  char* apOutData,unsigned short& aiOutDataLen);
private:
	// xl是需要加密的数据，xr是key,以数组形式保存，xrLen是Key的长度，按目前TEA算法的实现来说，一定是4
	// 用于加密的数据一定要是8的倍数，由算法需求决定
	VOID			en_code(uint32 *xl, uint32 *xr, uint32 xrLen);

	// xl是需要解密的数据，xr是key,以数组形式保存，xrLen是Key的长度，按目前TEA算法的实现来说，一定是4
	// 用于解密的数据一定是8的倍数，加密后的数据一定符合要求
	VOID			de_code(uint32 *xl, uint32 *xr, uint32 xrLen);
private:
	uint32 moEncryptKey[4];
};

#endif // __TEAENCRYPT_H__