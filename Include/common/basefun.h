// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: basefun.h
// 创 建 人: 史云杰
// 文件说明: 基本函数集
// $_FILEHEADER_END ******************************
#ifndef __BASEFUN_H
#define __BASEFUN_H
#include <stdio.h>
#include <string.h>
#include <string>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include "base0/platform.h"
using namespace std;
#ifdef WIN32
#include <assert.h>
#endif
string GetAppPathA();

#ifdef WIN32
u9_string GetAppPathU9();
#define GetAppPath GetAppPathU9
#else

#define GetAppPath GetAppPathA
#endif //__SERVER

u9_string& ReplaceAll(u9_string& str,const u9_string& old_value,
				   const u9_string& new_value);

void ReparePath(u9_string& u9_string);

int PublicRandom();
BOOL IsInRange(INT aiValue, INT aiStart, INT aiEnd);

#ifdef TRACE_LOG
//extern VOID CreateDebugInfo(const char* apAppName,int aiDispayType);
#endif

string WStr2Str(wstring astr);

// $_FUNCTION_BEGIN ******************************
// 方法名: u9_ahtoi
// 访  问: public 
// 参  数: char * aszStr
// 参  数: int32 &aiValue out
// 返回值: BOOL
// 说  明: 字符串（16进制字符字符串转换成数字）
// $_FUNCTION_END ********************************
BOOL u9_hexatoi(char * aszStr,int32 &aiValue);


// $_FUNCTION_BEGIN ******************************
// 方法名: u9_macatoi
// 访  问: public 
// 参  数: char * aszMac
// 参  数: int64 & aiMac
// 返回值: BOOL
// 说  明: 字符串mac地址转int64值
// $_FUNCTION_END ********************************
BOOL u9_macatoi(char* aszMac,int64& aiMac);

//BOOL u9_macitoa(const int64&aiMac,char* aszMac,int32 aiMaxLen);

INT GetLocalMacStr(char* apMac, INT aiBufLen);
INT GetLocalMacINT64(int64& ai64Mac);
int64 GetLocalMac(BOOL abRandom = FALSE);

/********************************************************************
*函数名:ELFHash
*参数:	 char*apszData  字符串
UINT aiLen 长度
*返回值:UINT  hash值
*功能：	hash值
*创建人:史云杰
********************************************************************/
UINT   ELFHash(const  char*apszData,UINT aiLen);

/********************************************************************
*函数名:get_RandPackID
*参数:	
*返回值:WORD 一个随机的包序号
*功能：	得到一个随机的包序号
*创建人:史云杰
********************************************************************/
WORD   get_RandPackID();

char *stristr(const char *s1, const char *s2);
char *trimstr(char *s);

/********************************************************************
*函数名:Strcmp
*参数:	const  char*apszStr1  第一个字符串
const  char* apSzStr2 第二个
*返回值:INT	小于0－apszStr1 小于apszStr2 
等于0－apszStr1等于apszStr2 
大于0－apszStr1 大于apszStr2

*功能：	字符窜比较
*创建人:史云杰
********************************************************************/
INT   Strcmp(const char *apszStr1,const  char* apszStr2);

/********************************************************************
*函数名:Strcmp
*参数:	const char *apszStr1  第一个字符串
const  char* apszStr2 第二个
*返回值:INT	小于0－apszStr1 小于apszStr2 
等于0－apszStr1等于apszStr2 
大于0－apszStr1 大于apszStr2

*功能：	字符窜比较 可以大小写忽略
*创建人:史云杰
********************************************************************/
INT   CompareStringM(const char *apszStr1,const char *apszStr2);//
/********************************************************************
*函数名:Tolower_String
*参数:	char *apszDest
const char *apszSrc
*返回值:INT

*功能：	1成功，0失败
*创建人:史云杰
********************************************************************/
INT   Tolower_String(char *apszDest,const char *apszSrc);


/********************************************************************
函数名:Strstr_Slur
参数:	unsigned char *apData 需要判断的字符串，不一定以零结尾
INT aiDataLen  字符串长度
char* apszKey 匹配字符字符串
返回值:INT 1是数组中有符合的单词，0是没有。
功能：	判断一个单词是否在一个句子中。可以模糊匹配，大小写不敏感。
创建人:史云杰
********************************************************************/
INT   Strstr_Slur(unsigned char *apData,INT aiDataLen,char *apszKey);


/********************************************************************
函数名:HasWordinString
参数:	char *apData 需要判断的字符串
INT aiDataLen  字符串长度
char ** appWord 单词列表。一个字符串数组
返回值:INT 1是数组中有符合的单词，0是没有。
功能：	判断一些单词是否在一个句子中。可以模糊匹配。
创建人:史云杰
********************************************************************/
INT   HasWordinString(unsigned char *apData,INT aiDataLen,char ** appWord,INT aiWordCount);

/********************************************************************
函数名:KeyInText
参数:	char *apData 需要判断的字符串
INT aiDataLen  字符串长度
char* apKey  一个字符串
返回值:INT 1是数组中有符合的单词，0是没有。<0有错
功能：	判断一个字字符串是否在一段短文中。可以模糊匹配。
创建人:史云杰
********************************************************************/
INT   KeyInText(unsigned char *apData,INT aiDataLen, char* apKey);
/********************************************************************
函数名:WordsInText
参数:	char *apData 需要判断的字符串
INT aiDataLen  字符串长度
char ** appWord 单词列表。一个字符串数组
INT aiWordCount 单词数
返回值:INT 1是数组中有符合的单词，0是没有。<0有错
功能：	判断一些单词是否在一段短文中。可以模糊匹配。
创建人:史云杰
********************************************************************/
INT   WordsInText(unsigned char *apData,INT aiDataLen,char ** appWord,INT aiWordCount);

/********************************************************************
函数名:CountWordinString
参数:	char *apData 需要判断的字符串
INT aiDataLen  字符串长度
char ** appWord 单词列表。一个字符串数组
INT *apiCountList 单词重复数
返回值:void 
功能：	判断一些单词在一个句子中出现的个数。可以模糊匹配。
创建人:史云杰
********************************************************************/
void   CountWordinString(unsigned char *apData,INT aiDataLen,char ** appWord,INT aiWordCount,INT *apiCountList);
/********************************************************************
函数名:StrCountinText
参数:	unsigned char  *apData 需要判断的字符串
INT aiDataLen  字符串长度
char ** appWord 单词列表。一个字符串数组
INT aiWordCount 单词数
INT *apiCountList 单词重复数
INT *aiLetterCount 字数
返回值:INT 小于等于零为有异常
功能：	判断一些单词在一个文本中出现的个数。可以模糊匹配，大小写不敏感。
创建人:史云杰
********************************************************************/
INT   StrCountinText(unsigned char  *apData,INT aiDataLen,char ** appWord,
					 INT aiWordCount,INT *apiCountList,INT *aiLetterCount);

/********************************************************************
函数名:IgnoreBlank
参数:	 char* apData 字符串
INT aiDataLen  字符串长度
INT *apiIndex 当前列表

返回值:void
功能：	跳过空格。
创建人:史云杰
********************************************************************/
void   IgnoreBlank( char* apData,INT aiDataLen,INT *apiIndex);
/********************************************************************
函数名:CompareStringbyList
参数:	char *apData 需要判断的字符串
char ** appWord 单词列表。一个字符串数组
INT aiWordCount 单词数
返回值:INT 1是数组中有符合的单词，0是没有。<0有错
功能：	判断一些单词是否在一段短文中。可以模糊匹配。
创建人:史云杰
********************************************************************/
INT  CompareStringbyList(char *apData,char **apWordList,INT aiCount);
/********************************************************************
函数名:ReadLineData
参数:	FILE *fp,文件句柄
char *apBuffer,缓存
UINT aiBufferLen,缓存大小
UINT* apiLen,读到的大小
返回值:INT 1是数组中有符合的单词，0是没有。<0有错
功能：	
********************************************************************/
INT  ReadLineData(FILE *fp,char *apBuffer,
				  UINT aiBufferLen,UINT* apiLen);

/********************************************************************
函数名:GetIPString
参数:	UINT aiIP ip网络
char *apBuffer,缓存
UINT aiBufferLen,缓存大小
UINT* apiLen,读到的大小
返回值:INT 1是数组中有符合的单词，0是没有。<0有错
功能：	
********************************************************************/
string  GetIPString(UINT aiIP);

/********************************************************************
函数名:FormatTime
参数:	ULONG aulTime, 时间
BOOL abDataTime = 是否带年月日。
返回值:string
功能：	
********************************************************************/
string FormatTime(ULONG aulTime, BOOL abDataTime = FALSE);

string GetTimeString(ULONG aulTime);

char* StrReplace(char* aSrcStr);

int base64WordToChars(char *out,const char *input);
inline uint16 get_le16(const void* p) 
{
	uint16 luiResult =*((uint16*)p);
	CHECK_ENDIAN16(luiResult);
	return luiResult;

}
inline uint8* set_le16(void* p, uint16 v) 
{
	uint16*r=(uint16*)p;
	*r=v;
	CHECK_ENDIAN16(*r);
	return (uint8*)r;
}

inline uint32 get_le32(const void* p) 
{
	uint32 luiResult =*((uint32*)p);
	CHECK_ENDIAN32(luiResult);
	return luiResult;
}

inline uint8* set_le32(void* p, uint32 v) {
	uint32*r=(uint32*)p;
	*r=v;
	CHECK_ENDIAN32(*r);
	return (uint8*)r;
}

inline uint64 get_le64(const void* p) 
{
	uint64 luiResult =*((uint64*)p);
	CHECK_ENDIAN64(luiResult);
	return luiResult;

}
inline uint8* set_le64(void* p, uint64 v)
{
	uint64*r=(uint64*)p;
	*r=v;
	CHECK_ENDIAN64(*r);
	return (uint8*)r;
}

inline uint16 get_be16(const void* p) {
#if _BIG_ENDIAN
	return *(const uint16_t*)p;
#else
	const uint8* c = (const uint8*) p;
	return c[0] << 8 | c[1];
#endif 
}
inline uint8* set_be16(void* p, uint16 v) {
	uint8* b = (uint8*) p;
	b[0] = static_cast<uint8>((v >> 8) & 0xff);
	b[1] = static_cast<uint8>(v & 0xff);
	return b;
}

inline uint32 get_be32(const void* p) {
#if _BIG_ENDIAN
	return *(const uint32*)p;
#else
	const uint8* c = (const uint8*) p;
	return c[0] << 24 | c[1] << 16 | c[2] << 8 | c[3];
#endif 
}

inline uint8* set_be32(void* p, uint32 v) 
{
	uint8* b = (uint8*) p;
	b[0] = uint8((v >> 24) & 0xff);
	b[1] = uint8((v >> 16) & 0xff);
	b[2] = uint8((v >> 8) & 0xff);
	b[3] = uint8(v & 0xff);
	return b;
}

inline uint64 get_be64(const void* p) {
#if _BIG_ENDIAN
	return *(const uint64*)p;
#else
	const uint8* c = (const uint8*) p;
	return ((uint64) get_be32(c)) << 32 | get_le32(c + 4);
#endif 

}

inline void get_leGUID(const void* p,GUID &v)
{
	GUID* b = (GUID*) p;
	v.Data1=b->Data1;
	CHECK_ENDIAN32(v.Data1);
	v.Data2=b->Data2;
	CHECK_ENDIAN16(v.Data2);
	v.Data3=b->Data3;
	CHECK_ENDIAN16(v.Data3);
	memcpy(v.Data4,b->Data4,sizeof(v.Data4));
}
inline void set_leGUID(char* p ,GUID &v)
{
	GUID* b = (GUID*) p;
	b->Data1=v.Data1;
	CHECK_ENDIAN32(b->Data1);
	b->Data2=v.Data2;
	CHECK_ENDIAN16(b->Data2);
	b->Data3=v.Data3;
	CHECK_ENDIAN16(b->Data3);
	memcpy(b->Data4,v.Data4,sizeof(v.Data4));
}
int itoa_IP(IPTYPE aIP, char* apszIPAddr,INT aiLen);
IPTYPE atoi_IP(const char* apszIPAddr);
IPTYPE GetIPAddress(const char *apszIPAddr);
string GetIntranetIP();
//::Cpp2Lua::Lua::FunWrapImpl::GetFunctionWraper(__VA_ARGS__).get<__VA_ARGS__>()


// $_FUNCTION_BEGIN ******************************
// 函数名称： 	ClearItem
// 函数参数：	
// 返 回 值：	
// 函数说明： 	删除指针模板
// $_FUNCTION_END ********************************

template<class T>
void ClearItem(T* apItem)
{
	if (NULL != apItem)
	{
		delete apItem;
		apItem = NULL;
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	ClearItem
// 函数参数：	
// 返 回 值：	
// 函数说明： 	删除指针模板
// $_FUNCTION_END ********************************

template<class T>
void ReleaseItem(T* apItem)
{
	if (NULL != apItem)
	{
		apItem->Release();
		apItem = NULL;
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	ConvertConstToNo
// 函数参数：	const T&	const型变量
// 返 回 值：	T			非Const型变量
// 函数说明： 	将const型转换为非const型
// $_FUNCTION_END ********************************

template<class T>
T ConvertConstToNo(const T& aoItem)
{
	return (*((T*)&aoItem));
}


// $_FUNCTION_BEGIN ******************************
// 方法名: Double2Int
// 访  问: public 
// 参  数: double adValue
// 返回值: int32
// 说  明: double转int
// $_FUNCTION_END ********************************
int32 Double2Int(double adValue);


inline uint32 Roll16(uint32 aiMaxRange, uint32 aiMinRange)
{
    const uint32 RANGE_MAX = static_cast<uint32>(RAND_MAX) + 1;
    uint32 liDicePoint = rand();
    return static_cast<uint32>(((float)liDicePoint / (float)RANGE_MAX) * aiMaxRange) + aiMinRange;
}

inline uint32 Roll32(uint32 aiMaxRange, uint32 aiMinRange)
{
    const __int64 RANGE_MAX = (static_cast<uint32>(RAND_MAX) << 16) + static_cast<uint32>(RAND_MAX) + 1;
    __int64 liDicePoint = rand();
    liDicePoint <<= 16;
    liDicePoint += rand();

	return static_cast<uint32>(((float)liDicePoint / (float)RANGE_MAX) * aiMaxRange) + aiMinRange;
}

// $_FUNCTION_BEGIN ******************************
// 方法名: Roll
// 参  数: uint32 aiMaxDicePoint
// 参  数: uint32 aiMinDicePoint
// 返回值: int32
// 说  明: 得到一个随机值，该值为[aiMinDicePoint, aiMaxDicePoint]区间上的间隔为1的一致随机正整数
// $_FUNCTION_END ********************************
inline uint32 Roll(uint32 aiMaxDicePoint, uint32 aiMinDicePoint = 1)
{
    if (aiMaxDicePoint == aiMinDicePoint)
    {
        return aiMaxDicePoint;
    }
    uint32 RANGE_MIN = aiMinDicePoint;
    uint32 RANGE_MAX = aiMaxDicePoint - aiMinDicePoint + 1;
    if (RANGE_MAX > RAND_MAX)
    {
        return Roll32(RANGE_MAX, RANGE_MIN);
    }
    return Roll16(RANGE_MAX, RANGE_MIN);
}
inline uint32 GameRoll()
{
	return Roll(999999, 0);
}
//判断物理文件是否存在
#ifdef WIN32
bool IsFileExist(const U9_CHAR* apszFile);
#else
bool IsFileExist(const char* apszFile);
#endif // WIN32



// $_FUNCTION_BEGIN ******************************
// 方法名: UNICODE2UTF8
// 访  问: public 
// 参  数: const wchar_t * awszInput_UNICODE unicode字符串
// 参  数: char * * appszOutput_UTF8  utf8字符串。里面分配空间
// 返回值: int  大于零是成功
// 说  明: unicode16 转换成 utf8 格式
// $_FUNCTION_END ********************************
int UNICODE2UTF8(const wchar_t *awszInput_UNICODE,char ** appszOutput_UTF8);

// $_FUNCTION_BEGIN ******************************
// 方法名: UNICODE2UTF8
// 访  问: public 
// 参  数: const wchar_t * awszInput_UNICODE unicode字符串
// 参  数: char * appszOutput_UTF8  utf8字符串
// 参  数: uint32 auiBufferLen  utf8字符串最大长度
// 返回值: int  大于零是成功
// 说  明: unicode16 转换成 utf8 格式
// $_FUNCTION_END ********************************
int UNICODE2UTF8(const wchar_t *awszInput_UNICODE,char * appszOutput_UTF8,uint32 auiBufferLen);

// $_FUNCTION_BEGIN ******************************
// 方法名: UNICODE2UTF8
// 访  问: public 
// 参  数: const char * apszInput_UTF8 utf8字符串
// 参  数: char * appszOutput_UNICODE  unicode字符串
// 参  数: uint32 aiUnicodeLen  unicode字符串最大长度
// 返回值: int  大于零是成功
// 说  明: utf8  转换成  unicode16 格式
// $_FUNCTION_END ********************************
int UTF82UNICODE(const char *apszInput_UTF8,wchar_t * appszOutput_UNICODE, int32 aiUnicodeLen);

// $_FUNCTION_BEGIN ******************************
// 方法名: TimeToString
// 访  问: public 
// 参  数: u9_string apszString
// 参  数: int aiMaxCharsIncludingTerminator
// 参  数: long alFileTime
// 返回值: void
// 说  明: 时间转换成字符串
// $_FUNCTION_END ********************************
u9_string TimeToString(long alTime);

#define DEF_VERSION_LEN              (10)		// 版本信息最大长度
#define DEF_EXT_VERSION_LEN          (16)		// 版本信息最大长度
#define DEF_MIN_VERSION_LEN			 (7)		// 版本信息最小长度
#define DEF_MAJOR_VERSION_LEN		 (2)		// 主版本长度
#define DEF_MINOR_VERSION_LEN		 (2)		// 副版本长度
#define DEF_MINI_VERSION_LEN		 (3)		// 子版本长度长度
bool GetVer(const U9_CHAR* astrFullVer, int32 aiLen,
			U9_CHAR *astrMajorVer,U9_CHAR *astrMinorVer,U9_CHAR *astrMiniVer);

bool GetVer(const char* astrFullVer, int32 aiLen,
	char *astrMajorVer,char *astrMinorVer,char *astrMiniVer);

//创建刷新时间
enum ENUM_TIME_TYPE{
	ENUM_SECONDS_PER_MIN	= 60,			//1分钟 60秒
	ENUM_SECONDS_PER_HOUR	= 3600,			//1小时 60*60=3600秒
	ENUM_SECONDS_PER_DAY	= 86400,		//1天  3600*24=86400秒
	ENUM_SECONDS_PER_WEEK	= 604800		//1周  86400*7=604800
};
time_t MakeHourRefreshTime(int32 aiTime);//00:00
time_t MakeDailiyRefreshTime(int32 aiTime);//按天的00:00:00
time_t MakeWeeklyRefreshTime(int32 aiTime);//按周一的00:00:00

uint64 atoull(const char* pszSrc) ;

uint64 PhoneNumer2PhoneID(const char* pszPhoneNumber) ;

bool utf8cpy_ex(char* des,const char *src, int32 aiLen);

#define SHA256_ROTL(a,b) (((a>>(32-b))&(0x7fffffff>>(31-b)))|(a<<b))
#define SHA256_SR(a,b) ((a>>b)&(0x7fffffff>>(b-1)))
#define SHA256_Ch(x,y,z) ((x&y)^((~x)&z))
#define SHA256_Maj(x,y,z) ((x&y)^(x&z)^(y&z))
#define SHA256_E0(x) (SHA256_ROTL(x,30)^SHA256_ROTL(x,19)^SHA256_ROTL(x,10))
#define SHA256_E1(x) (SHA256_ROTL(x,26)^SHA256_ROTL(x,21)^SHA256_ROTL(x,7))
#define SHA256_O0(x) (SHA256_ROTL(x,25)^SHA256_ROTL(x,14)^SHA256_SR(x,3))
#define SHA256_O1(x) (SHA256_ROTL(x,15)^SHA256_ROTL(x,13)^SHA256_SR(x,10))
inline string sha_256(const char* str, long long length)
{
    char sha256[256] = {0};
	char *pp, *ppend;
	int l, i, W[64], T1, T2, A, B, C, D, E, F, G, H, H0, H1, H2, H3, H4, H5, H6, H7;
	H0 = 0x6a09e667, H1 = 0xbb67ae85, H2 = 0x3c6ef372, H3 = 0xa54ff53a;
	H4 = 0x510e527f, H5 = 0x9b05688c, H6 = 0x1f83d9ab, H7 = 0x5be0cd19;
	int K[64] = {
			0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
			0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
			0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
			0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
			0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
			0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
			0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
			0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
	};
	l = length + ((length % 64 > 56) ? (128 - length % 64) : (64 - length % 64));
	if (!(pp = (char*)malloc((unsigned int)l))) return 0;
	for (i = 0; i < length; pp[i + 3 - 2 * (i % 4)] = str[i], i++);
	for (pp[i + 3 - 2 * (i % 4)] = 128, i++; i < l; pp[i + 3 - 2 * (i % 4)] = 0, i++);
	*((int*)(pp + l - 4)) = length << 3;
	*((int*)(pp + l - 8)) = length >> 29;
	for (ppend = pp + l; pp < ppend; pp += 64){
		for (i = 0; i < 16; W[i] = ((int*)pp)[i], i++);
		for (i = 16; i < 64; W[i] = (SHA256_O1(W[i - 2]) + W[i - 7] + SHA256_O0(W[i - 15]) + W[i - 16]), i++);
		A = H0, B = H1, C = H2, D = H3, E = H4, F = H5, G = H6, H = H7;
		for (i = 0; i < 64; i++){
			T1 = H + SHA256_E1(E) + SHA256_Ch(E, F, G) + K[i] + W[i];
			T2 = SHA256_E0(A) + SHA256_Maj(A, B, C);
			H = G, G = F, F = E, E = D + T1, D = C, C = B, B = A, A = T1 + T2;
		}
		H0 += A, H1 += B, H2 += C, H3 += D, H4 += E, H5 += F, H6 += G, H7 += H;
	}
	free(pp - l);
	sprintf(sha256, "%08x%08x%08x%08x%08x%08x%08x%08x", H0, H1, H2, H3, H4, H5, H6, H7);
    string str_sha256 = sha256;
	return str_sha256;
}

inline string random_str_keys(int length)
{
    string key = "";
    string pattern = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLOMNOPQRSTUVWXYZ";
    if(length <= 0)
    {
        return key;
    }
    for(int index = 0; index < length; index++)
    {
        key += pattern.at(rand() % 62);
    }
    return key;
}

inline int get_asc_mod(string target_data, int mod_count)
{
    if(mod_count <= 0 || target_data.empty())
    {
        return 0;
    }

    int sum_asc = 0;
    for(int index = 0; index < target_data.length(); index++)
    {
        int asc = (int)target_data.at(index);
        sum_asc += asc;
    }

    return sum_asc % mod_count;
}
#endif //__BASEFUN_H
