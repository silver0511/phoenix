/***********************************************************
// 版权声明：xxxxx版权所有
//			  
文 件 名：	TeaEnrypt.h
创 建 人：	史云杰
说    明：	tea加密类
***********************************************************/


#include "encrypt/TeaEncrypt.h"


CTeaEncrypt::CTeaEncrypt()
{
	moEncryptKey[0] = 2345;
	moEncryptKey[1] = 1234;
	moEncryptKey[2] = 3456;
	moEncryptKey[3] = 6789;
}

CTeaEncrypt::~CTeaEncrypt()
{
	moEncryptKey[0] = 0;
	moEncryptKey[1] = 0;
	moEncryptKey[2] = 0;
	moEncryptKey[3] = 0;
}

void CTeaEncrypt::en_code(uint32 *v, uint32 *k, uint32 xrLen)
{	
	//LMASSERT(xrLen == 4);
	if( xrLen != 4)
	{
		throw -1;
	}
	uint32 v0=v[0], v1=v[1], sum=0, i; /* set up */ 
	uint32 delta=0x9e3779b9; /* a key schedule constant */ 
	uint32 k0=k[0], k1=k[1], k2=k[2], k3=k[3]; /* cache key */ 
	for (i=0; i < 32; i++) { /* basic cycle start */ 
		sum += delta; 
		v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1); 
		v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3); 
	} /* end cycle */ 
	v[0]=v0; v[1]=v1; 
}

void CTeaEncrypt::de_code(uint32 *v, uint32 *k, uint32 xrLen)
{
	//LMASSERT(xrLen == 4);
	if( xrLen != 4)
	{
		throw -1;
	}

	uint32 v0=v[0], v1=v[1], sum=0xC6EF3720, i; /* set up */ 
	uint32 delta=0x9e3779b9; /* a key schedule constant */ 
	uint32 k0=k[0], k1=k[1], k2=k[2], k3=k[3]; /* cache key */ 
	for (i=0; i < 32; i++) { /* basic cycle start */ 
		v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3); 
		v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1); 
		sum -= delta; 
	} /* end cycle */ 
	v[0]=v0; v[1]=v1; 
}

#define DEF_TMP_STACK_BUF_SIZE 4096 //临时buffer处理大小
int CTeaEncrypt::Encrypt(char * apInData,unsigned short aiInDataLen, char * apOutData,unsigned short & aiOutDataLen)
{
	try
	{
		unsigned short liLen;
		char lpBuffer[DEF_TMP_STACK_BUF_SIZE];
		int liIsStackBuffer;
		char *lpBuf ;
		//最头两个字节用来保存数据的实际长度
		liLen = aiInDataLen+2;
		//数据需要按8字节对齐（补0对齐）
		if(liLen%8!=0)
			liLen=liLen-liLen%8+8;
		if (DEF_TMP_STACK_BUF_SIZE >= liLen){
			lpBuf=lpBuffer;
			liIsStackBuffer=1;
		}
		else{
			lpBuf = new char[liLen];
			if (lpBuf == NULL)
				return -1;
			liIsStackBuffer=0;
		}
		memcpy(lpBuf+2,apInData,aiInDataLen);
		((unsigned short *)lpBuf)[0] = aiInDataLen;
		unsigned short lsTempLen=liLen>>3;
		for(int i=0;i<lsTempLen;i++)
			en_code((uint32 *)(lpBuf+i*8), moEncryptKey, 4);
		aiOutDataLen=liLen;
		memcpy(apOutData,lpBuf,liLen);

		if (0 == liIsStackBuffer){
			if (NULL != lpBuf){
				delete []lpBuf;
				lpBuf = NULL;
			}
		}
	}
	catch(...){
		return -1;
	}
	return 1;
}

int CTeaEncrypt::Decrypt(char * apInData,unsigned short aiInDataLen, char * apOutData,unsigned short & aiOutDataLen)
{
	try{
		//判断是否为8的整数倍
		if (aiInDataLen % 8 != 0){
			//不是由此加密算法加密出的数据
			return -1;
		}
		char lpBuffer[DEF_TMP_STACK_BUF_SIZE];
		int liIsStackBuffer;
		char *lpBuf = NULL;

		if (DEF_TMP_STACK_BUF_SIZE >= aiInDataLen){
			lpBuf=lpBuffer;
			liIsStackBuffer=1;
		}
		else{
			lpBuf = new char[aiInDataLen];
			if (lpBuf == NULL)
				return -1;
			liIsStackBuffer=0;
		}

		memcpy(lpBuf,apInData,aiInDataLen);
		unsigned short lsTempLen=aiInDataLen>>3;
		for(int i=0;i<lsTempLen;i++)
			de_code((uint32 *)(lpBuf+i*8), moEncryptKey, 4);

		aiOutDataLen = ((unsigned short *)lpBuf)[0];

		if (aiOutDataLen > aiInDataLen){
			//输出数据长度不能大于输入数据长度
			if (0 == liIsStackBuffer){
				if (lpBuf != NULL)
				{
					delete []lpBuf;
					lpBuf = NULL;
				}
			}
			return -1;
		}
		memcpy(apOutData,lpBuf+2,aiOutDataLen);
		if (0 == liIsStackBuffer){
			if (lpBuf != NULL)
			{
				delete []lpBuf;
				lpBuf = NULL;
			}
		}
		return 1;
	}
	catch(...){
		return -1;
	}
}
