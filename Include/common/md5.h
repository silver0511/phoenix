// $_FILEHEADER_BEGIN ****************************
//xxxxx版权所有
// 
// 文件名称：platform.h
// 创建人：史云杰
// 说明： md5加密类
// $_FILEHEADER_END ******************************

#ifndef __CMD5_H_
#define __CMD5_H_

#include "base0/u9_string.h"
#include "base0/platform.h"

//---------------------------------------------------------------------------
/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;


/* Constants for MD5Transform routine.
*/
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

//---------------------------------------------------------------------------
/* MD5 context. */
typedef struct
{
	UINT4 state[4];           /* state (ABCD) */
	UINT4 count[2];           /* number of bits, modulo 2^64 (lsb first) */
	BYTE  buffer[64]; /* input buffer */
} MD5_CTX;

//---------------------------------------------------------------------------
class CMD5
{
public :


	// $_FUNCTION_BEGIN ******************************
	// 函数名称： MD5_2_Binary
	// 函数参数：const unsigned char *apszInput,输入数据
	//			unsigned int aiInputLen,输入数据长度
	//			unsigned char apszOutput[16],输出数据，以二进制方式
	//			int aiIteration=1
	// 返 回 值： 
	// 函数说明： md5散列运算
	// $_FUNCTION_END ********************************
	void MD5_2_Binary(const BYTE *apszInput, unsigned int aiInputLen,
		BYTE* apszOutput, int aiLen,int aiIteration=1);

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： MD5_2_HEXASCII
	// 函数参数：const unsigned char *apszInput,输入数据
	//			unsigned int aiInputLen,输入数据长度
	//			unsigned char apszOutput[32],输出数据，以16进制ascii码方式输出
	//			int aiIteration=1
	// 返 回 值： 
	// 函数说明： md5散列运算
	// $_FUNCTION_END ********************************
	void MD5_2_HEXASCII(const BYTE *apszInput, unsigned int aiInputLen,
		BYTE * apszOutput, int aiLen, int aiIteration=1);

	// $_FUNCTION_BEGIN ******************************
	// 方法名: MD5_2_HEXU16
	// 访  问: public 
	// 参  数: const BYTE * apszInput	输入数据
	// 参  数: unsigned int aiInputLen	输入数据长度
	// 参  数: wchar_t * apszOutput	输出数据，以16进制ascii码方式输出
	// 参  数: int aiLen
	// 参  数: int aiIteration
	// 返回值: void
	// 说  明: md5散列运算 到unicode16,以宽字节输出
	// $_FUNCTION_END ********************************
	void MD5_2_HEXU16(const BYTE* apszInput, unsigned int aiInputLen,
		wchar_t* apszOutput, int aiLen,int aiIteration=1);

private:
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： MD5_2_Binary
	// 函数参数：const unsigned char *apszInput,输入数据
	//			unsigned int aiInputLen,输入数据长度
	//			unsigned char apszOutput[16],输出数据，以二进制方式
	//			int aiIteration=1
	// 返 回 值： 1是成功 -1是失败，
	// 函数说明： md5散列运算
	// $_FUNCTION_END ********************************
	void MD5(const BYTE *apszInput, unsigned int aiInputLen,
		BYTE apszOutput[16], int aiIteration=1);

protected :
	MD5_CTX mstruContext;
	void Init(MD5_CTX *context);
	void Update(MD5_CTX *context, const BYTE *input, unsigned int inputLen);
	void Final(BYTE digest[16], MD5_CTX *context);
	void Transform(UINT4 state[4], const BYTE block[64]);
};

std::string MakeMD5(const char* apszStr, size_t aiStrlen);

//---------------------------------------------------------------------------

#endif