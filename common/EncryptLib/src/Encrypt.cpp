/***********************************************************
// 版权声明：xxxxx版权所有
//			  
//文 件 名：	Enrypt.cpp
//创 建 人：	史云杰
//说    明：	加密解密
***********************************************************/
#include "encrypt/ASCIIEnrypt.h"
#include "encrypt/TeaEncrypt.h"
#include "encrypt/BlowFishEncrypt.h"


IEncrypt* CreateEncrypt(U9_CHAR* apEncryptName)
{
	if( 0 == u9_strcmp(apEncryptName, _T("ASC")))
	{
		return new CASCIIEnrypt;
	}
	else if( 0 == u9_strcmp(apEncryptName, _T("BLOWFISH")))
	{
		return new CBlowFishEncrypt;
	}
	else if( 0 == u9_strcmp(apEncryptName, _T("TEA")))
	{
		return new CTeaEncrypt;
	}

	return NULL;
}

void ReleaseEncrypt(IEncrypt* apEncrypt)
{
	SAFE_DELETE(apEncrypt);
}