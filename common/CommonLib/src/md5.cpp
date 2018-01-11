// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: DebugTraceThread.h
// 创 建 人: 史云杰
// 文件说明: md5
// $_FILEHEADER_END ******************************
//---------------------------------------------------------------------------
#include "sys/types.h"
#include "sys/stat.h"
#include "common/md5.h"
#include "common/basefun.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/* F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

//---------------------------------------------------------------------------
/* ROTATE_LEFT rotates x left n bits.
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, an
d 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac)\
{ \
	(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac)\
{ \
	(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac)\
{ \
	(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define II(a, b, c, d, x, s, ac)\
{ \
	(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
BYTE PADDING[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

BYTE BIN_2_ASCII[16]={'0','1','2','3','4','5','6','7','8','9',
'a','b','c','d','e','f'};

wchar_t BIN_2_U16[16] = {L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8',
						L'9', L'a', L'b', L'c', L'd', L'e', L'f'};

//---------------------------------------------------------------------------
/* MD5 initialization. Begins an MD5 operation, writing a new mstruContext. */
void CMD5::Init(MD5_CTX *mstruContext)
{
	mstruContext->count[0] = mstruContext->count[1] = 0;
	/* Load magic initialization constants. */
	mstruContext->state[0] = 0x67452301;
	mstruContext->state[1] = 0xefcdab89;
	mstruContext->state[2] = 0x98badcfe;
	mstruContext->state[3] = 0x10325476;
}

//---------------------------------------------------------------------------
/* MD5 block update operation. Continues an MD5 message-digest
operation, processing another message block, and updating the
mstruContext.
*/
void CMD5::Update(MD5_CTX *mstruContext, const BYTE *input, unsigned int inputLen)
{
	unsigned int i, index, partLen;

	/* Compute number of bytes mod 64 */
	index = (unsigned int)((mstruContext->count[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ( (mstruContext->count[0] += ((UINT4)inputLen << 3))
		< ((UINT4)inputLen << 3) )
		mstruContext->count[1]++;

	mstruContext->count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	/* Transform as many times as possible.
	*/
	if (inputLen >= partLen)
	{
		memcpy((POINTER)&mstruContext->buffer[index], (POINTER)input, partLen);
		Transform (mstruContext->state, mstruContext->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
			Transform (mstruContext->state, &input[i]);

		index = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	memcpy((POINTER)&mstruContext->buffer[index], (POINTER)&input[i], inputLen-i);
}

//---------------------------------------------------------------------------
/* MD5 finalization. Ends an MD5 message-digest operation, writing the
the message digest and zeroizing the mstruContext.
*/
void CMD5::Final(BYTE digest[16], MD5_CTX *mstruContext)
{
	BYTE bits[8];
	unsigned int index, padLen;

	/* Save number of bits */
	memcpy(bits, mstruContext->count, 8);

	/* Pad out to 56 mod 64. */
	index = (unsigned int)((mstruContext->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	Update (mstruContext, PADDING, padLen);

	/* Append length (before padding) */
	Update(mstruContext, bits, 8);

	/* Store state in digest */
	memcpy(digest, mstruContext->state, 16);

	/* Zeroize sensitive information. */
	memset((POINTER)mstruContext, 0, sizeof (*mstruContext));
}
//---------------------------------------------------------------------------

/* MD5 basic transformation. Transforms state based on block. */
void CMD5::Transform(UINT4 state[4], const BYTE block[64])
{
	UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	memcpy (x, block, 64);

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	/* Zeroize sensitive information. */
	memset((POINTER)x, 0, sizeof (x));
}

//---------------------------------------------------------------------------
// $_FUNCTION_BEGIN ******************************
// 函数名称： MD5_2_Binary
// 函数参数：const unsigned char *apszInput,输入数据
//			unsigned int aiInputLen,输入数据长度
//			unsigned char apszOutput[16],输出数据，以二进制方式
//			int aiIteration=1
// 返 回 值： 1是成功 -1是失败，
// 函数说明： md5散列运算
// $_FUNCTION_END ********************************
void CMD5::MD5(const BYTE *apszInput, unsigned int aiInputLen,
			   BYTE apszOutput[16], int aiIteration)
{
	Init(&mstruContext);
	for (int i=0; i<aiIteration; i++)
	{
		Update(&mstruContext, apszInput, aiInputLen);
	}
	Final(apszOutput, &mstruContext);
}



//---------------------------------------------------------------------------
// $_FUNCTION_BEGIN ******************************
// 函数名称： MD5_2_Binary
// 函数参数：const unsigned char *apszInput,输入数据
//			unsigned int aiInputLen,输入数据长度
//			unsigned char apszOutput[16],输出数据，以二进制方式
//			int aiIteration=1
// 返 回 值： 1是成功 -1是失败，
// 函数说明： md5散列运算
// $_FUNCTION_END ********************************
void CMD5::MD5_2_Binary(const BYTE *apszInput, unsigned int aiInputLen,
						BYTE *apszOutput, int aiLen, int aiIteration)
{
	if(aiLen<16||NULL == apszInput)
		return;

	BYTE lpBinMD5[16];
	MD5(apszInput,aiInputLen,lpBinMD5,aiIteration);
	memcpy(apszOutput,lpBinMD5,16);
}
// $_FUNCTION_BEGIN ******************************
// 函数名称： MD5_2_ASCII
// 函数参数：const unsigned char *apszInput,输入数据
//			unsigned int aiInputLen,输入数据长度
//			unsigned char apszOutput[32],输出数据，以16进制ascii码方式输出
//			int aiIteration=1
// 返 回 值： 1是成功 -1是失败，
// 函数说明： md5散列运算得到ascii码
// $_FUNCTION_END ********************************
void CMD5::MD5_2_HEXASCII(const BYTE* apszInput, unsigned int aiInputLen,
						  BYTE* apszOutput, int aiLen,int aiIteration)

{
	if(aiLen<32||NULL == apszInput)
		return;

	BYTE lpBinMD5[16];
	int j=0;
	MD5(apszInput,aiInputLen,lpBinMD5,aiIteration);
	for (int i=0;i<16;i++)
	{
		apszOutput[j++]=BIN_2_ASCII[lpBinMD5[i]>>4];//低4位
		apszOutput[j++]=BIN_2_ASCII[lpBinMD5[i]&0xf];//高4位
	}
}

void CMD5::MD5_2_HEXU16(const BYTE* apszInput, unsigned int aiInputLen,
						wchar_t* apszOutput, int aiLen,int aiIteration)
{
	if(aiLen<32||NULL == apszInput)
		return;

	BYTE lpBinMD5[16];
	MD5(apszInput, aiInputLen, lpBinMD5, aiIteration);

	for (int i=0, j=0;i<16;i++)
	{
		apszOutput[j++]=BIN_2_U16[lpBinMD5[i]>>4];//低4位
		apszOutput[j++]=BIN_2_U16[lpBinMD5[i]&0xf];//高4位
	}

}

std::string MakeMD5(const char* apszStr, size_t aiStrlen)
{
	enum{
		ENU_MD5_LEN = 32
	};
	BYTE lpszBuffer[ENU_MD5_LEN + 1];
	ZeroMemory(&lpszBuffer[0], sizeof(lpszBuffer));
	if (aiStrlen <= 0)
	{
		return std::string("");
	}
	CMD5 loMd5;
	loMd5.MD5_2_HEXASCII(reinterpret_cast<const BYTE*>(apszStr), aiStrlen, &lpszBuffer[0], ENU_MD5_LEN);
	return std::string(reinterpret_cast<char*>(&lpszBuffer[0]));
}
