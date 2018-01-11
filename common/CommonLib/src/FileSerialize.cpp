// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: FileSerialize.h
// 创 建 人: 史云杰
// 文件说明: 事件处理
// $_FILEHEADER_END ******************************


#include "stdafx.h"
#include "common/basefun.h"
#include "common/FileSerialize.h"
using namespace std;

//#define ENCRYPT
//-----------------------------------------------------------------------------
uint32 CFileSerialize::GetPos()
{
	return moFileStream.tellp();
}
//-----------------------------------------------------------------------------
void CFileSerialize::Seek(uint32 adwPos,int8 aiOrigin)
{
	moFileStream.seekp(adwPos, (ios_base::seekdir)aiOrigin);
}
//-----------------------------------------------------------------------------
CFileSerialize::CFileSerialize(ENUM_TYPE abType)
{
	mbyType = abType;
	mbySerialType = SERIAL_FILE;
};

CFileSerialize::CFileSerialize()
{
	mbyType = ISerialize::LOAD;
	mbySerialType = SERIAL_FILE;
};


CFileSerialize::~CFileSerialize()
{
	if (moFileStream.is_open())
	{
		moFileStream.close();
	}
}
void CFileSerialize::SetType(ENUM_TYPE abType)
{
	mbyType = abType;
	mbySerialType = SERIAL_FILE;
}
#ifdef WIN32
bool CFileSerialize::Open(const U9_CHAR* apFileName, ios_base::open_mode aiMode)
#else
bool CFileSerialize::Open(const U9_CHAR* apFileName, std::_Ios_Openmode aiMode)
#endif //WIN32
{
#ifdef WIN32 
	moFileStream.open(apFileName, aiMode);
#else
	std::locale::global(std::locale(""));
#ifndef UNICODE
	moFileStream.open(apFileName, aiMode);
#else
	char *lszFileName=NULL;
	if(0>=UNICODE2UTF8(apFileName,&lszFileName))
		return false;
	moFileStream.open(lszFileName, aiMode);
	if(NULL != lszFileName)
	{
		delete[] lszFileName;
	}
#endif UNICODE
#endif //WIN32
	if (moFileStream.is_open())
	{
		return true;
	}
	return false;
}
bool CFileSerialize::Open(const U9_CHAR* apFileName)
{
	if(get_mbType()==LOAD)
	{
		return Open(apFileName,ios::in);
	}
	else 
	{
		return Open(apFileName,ios::out);
	}
}



//-----------------------------------------------------------------------------

//// $_FUNCTION_BEGIN ******************************
//// 函数名称： 	CFileSerialize
//// 函数参数：	fstream&  aFileStream   存储流
////              bool abType  true 保存   false 取出
//// 返 回 值： 
//// 函数说明：   构造函数
//// $_FUNCTION_END ********************************
//
//CFileSerialize::CFileSerialize(fstream&  aFileStream, bool abType)
//{
//	mpFileStream = &aFileStream;
//	get_mbType()      = abType;
//}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	bool & astrVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(bool& abVal)
{
	try
	{
		if (get_mbType() ==STORE)
		{	//存储
			moFileStream.write((char*)&abVal, sizeof(bool));			
		}
		else
		{
			//取出
			moFileStream.read((char*)&abVal, sizeof(bool));
		}

		return sizeof(bool);
	}
	catch(...)
	{
		throw(-1);
	}
}
// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	int8 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(int8 & abyVal)
{
	try
	{

		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&abyVal, sizeof(int8));
		}
		else
		{
			//取出
			moFileStream.read((char*)&abyVal, sizeof(int8));
		}

		return sizeof(int8);
	}
	catch(...)
	{
		throw(-1);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	uint8 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(uint8 & abyVal)
{
	try
	{

		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&abyVal, sizeof(uint8));
		}
		else
		{
			//取出
			moFileStream.read((char*)&abyVal, sizeof(uint8));
		}

		return sizeof(uint8);
	}
	catch(...)
	{
		throw(-1);
	}
}
// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	int16 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(int16& anVal)
{
	try
	{

		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&anVal, sizeof(int16));
		}
		else
		{
			//取出
			moFileStream.read((char*)&anVal, sizeof(int16));
		}

		return sizeof(int16);
	}
	catch(...)
	{
		throw(-1);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	uint16 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(uint16& anVal)
{
	try
	{

		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&anVal, sizeof(uint16));
		}
		else
		{
			//取出
			moFileStream.read((char*)&anVal, sizeof(uint16));
		}

		return sizeof(uint16);
	}
	catch(...)
	{
		throw(-1);
	}
}


// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	int32 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(int32& anVal)
{
	try
	{

		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&anVal, sizeof(int32));
		}
		else
		{
			//取出
			moFileStream.read((char*)&anVal, sizeof(int32));
		}

		return sizeof(int32);
	}
	catch(...)
	{
		throw(-1);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	uint32 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(uint32& anVal)
{
	try
	{

		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&anVal, sizeof(uint32));
		}
		else
		{
			//取出
			moFileStream.read((char*)&anVal, sizeof(uint32));
		}

		return sizeof(uint32);
	}
	catch(...)
	{
		throw(-1);
	}
}
// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	int64 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(int64& anVal)
{
	try
	{

		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&anVal, sizeof(int64));
		}
		else
		{
			//取出
			moFileStream.read((char*)&anVal, sizeof(int64));
		}

		return sizeof(int64);
	}
	catch(...)
	{
		throw(-1);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	uint64 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(uint64& anVal)
{
	try
	{

		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&anVal, sizeof(uint64));
		}
		else
		{
			//取出
			moFileStream.read((char*)&anVal, sizeof(uint64));
		}

		return sizeof(uint64);
	}
	catch(...)
	{
		throw(-1);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	float32 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************

LONG CFileSerialize::Serialize(float32& afVal)
{
	try
	{
		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&afVal, sizeof(float32));
		}
		else
		{
			//取出
			moFileStream.read((char*)&afVal, sizeof(float32));
		}

		return sizeof(float32);
	}
	catch(...)
	{
		throw(-1);
	}
}
// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	float64 &aiVal  要保存的数据
// 返 回 值：	数据长度
// 函数说明：   保存数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(float64& adbVal)
{
	try
	{
		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)&adbVal, sizeof(float64));
		}
		else
		{
			//取出
			moFileStream.read((char*)&adbVal, sizeof(float64));
		}

		return sizeof(float64);
	}
	catch(...)
	{
		throw(-1);
	}
}

//
//// $_FUNCTION_BEGIN ******************************
//// 函数名称： 	Serialize
//// 函数参数：	INT &aiVal  要保存的数据
//// 返 回 值：	数据长度
//// 函数说明：   保存数据
//// $_FUNCTION_END ********************************
//LONG CFileSerialize::Serialize(INT& adbVal)
//{
//	try
//	{
//		if (get_mbType() == STORE)
//		{
//			//存入文件
//			moFileStream.write((char*)&adbVal, sizeof(INT));
//		}
//		else
//		{
//			//取出
//			moFileStream.read((char*)&adbVal, sizeof(INT));
//		}
//
//		return sizeof(INT);
//	}
//	catch(...)
//	{
//		throw(-1);
//	}
//}
//
//// $_FUNCTION_BEGIN ******************************
//// 函数名称： 	Serialize
//// 函数参数：	UINT &aiVal  要保存的数据
//// 返 回 值：	数据长度
//// 函数说明：   保存数据
//// $_FUNCTION_END ********************************
//LONG CFileSerialize::Serialize(UINT& adbVal)
//{
//	try
//	{
//		if (get_mbType() == STORE)
//		{
//			//存入文件
//			moFileStream.write((char*)&adbVal, sizeof(UINT));
//		}
//		else
//		{
//			//取出
//			moFileStream.read((char*)&adbVal, sizeof(UINT));
//		}
//
//		return sizeof(UINT);
//	}
//	catch(...)
//	{
//		throw(-1);
//	}
//}


// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	char* apValue  要保存的数据地址
// 参数：       WORD awMaxLen  字符串的最大长度
// 返 回 值：	数据长度
// 函数说明：   序列化串类型的值(以\0结尾的字符串)
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize( char* apValue,WORD awMaxLen)
{
	//U9_ASSERT(NULL != apValue);
	if (NULL == apValue)
	{
		return 0;
	}
	try
	{
		WORD lwLen=0;
		if (mbyType==ISerialize::STORE)
		{   //存储
			lwLen = (WORD)strlen( apValue );
			if (lwLen >= awMaxLen)
				throw(-1);
			moFileStream.write((char*)&lwLen, sizeof(uint16));
			if (0 != lwLen)
			{
				moFileStream.write((char*)apValue, lwLen);
			}
		}
		else
		{
			//取出
			moFileStream.read((char*)&lwLen, sizeof(uint16));
			if (lwLen > awMaxLen)
				throw(-1);
			if(lwLen != 0 )
			{
				ZeroMemory(apValue, lwLen);
				moFileStream.read((char*)apValue, lwLen);
			}
			//apValue[lwLen] = 0;
		}

		return (lwLen + sizeof(uint16));
	}
	catch(...)
	{
		throw(-1);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	wchar_t* apValue  要保存的数据地址
// 参数：       WORD awMaxLen  字符串的长度
// 返 回 值：	数据长度
// 函数说明：   序列化串类型的值(以\0结尾的字符串)
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize( wchar_t* apValue,WORD awMaxLen)
{
	//U9_ASSERT(NULL != apValue);
	if (NULL == apValue)
	{
		return 0;
	}
	try
	{
		WORD lwLen=0;
		if (mbyType==ISerialize::STORE)
		{   //存储
			lwLen = (WORD)u9_wcslen(apValue );
			if (lwLen >= awMaxLen)
				throw( -1 );
			lwLen = lwLen*sizeof(wchar_t);
			moFileStream.write((char*)&lwLen, sizeof(WORD));
			if (0 != lwLen)
				moFileStream.write((char*)apValue, lwLen);
		}
		else
		{
			//取出
			moFileStream.read((char*)&lwLen, sizeof(uint16));
			if ( lwLen+2 > (WORD) (awMaxLen*sizeof(wchar_t)))
			{
				throw( -1 );
			}
			ZeroMemory((char*)apValue, awMaxLen*sizeof(wchar_t));
			if(lwLen != 0 )
			{
				moFileStream.read((char*)apValue, lwLen);
			}
			//apValue[lwLen] = 0;
		}

		return (lwLen + sizeof(WORD));
	}
	catch(...)
	{
		throw(-1);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	BYTE* apValue   数据 ( in/out )要序列化的数据
// 参数:  　　　WORD &awLen      此数据的真正长度
// 参数:        WORD aiBufferLen 容纳此数据的缓存区大小
// 返 回 值：	数据长度
// 函数说明：   序列化二进制数据
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize( BYTE* apValue, WORD &awLen,WORD aiBufferLen)
{
	//U9_ASSERT(NULL !=apValue && awLen < aiBufferLen && awLen > 0 );
	if(NULL ==apValue || awLen > aiBufferLen || awLen <= 0 )
			throw ( -1 );
	try
	{
		if (get_mbType() == STORE)
		{
			//存入文件
			moFileStream.write((char*)apValue, awLen);
		}
		else
		{
			//取出
			moFileStream.read((char*)apValue, awLen);
		}

		return awLen;
	}
	catch(...)
	{
		throw(-1);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	string& astrVal  要序列化的数据
// 返 回 值：	数据长度
// 函数说明：   序列化string类型的值
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(string& astrVal)
{
	WORD lwLen = (WORD)astrVal.size(); 

	try
	{
		if (get_mbType() == STORE)
		{   //存储
			moFileStream.write((char*)&lwLen, sizeof(WORD ));
			if(0 != lwLen)
				moFileStream.write((char*)astrVal.c_str(), lwLen);
		}
		else
		{
			//取出
			moFileStream.read((char*)&lwLen, sizeof(WORD ));
			if (0 == lwLen )
				return sizeof(WORD );

			char* lpszString = NULL;
			lpszString = new char[lwLen+1];

			if (NULL == lpszString)
				throw(-1);
			moFileStream.read(lpszString, lwLen);
			lpszString[lwLen] = '\0';
			astrVal = lpszString;
			if (NULL != lpszString )
			{
				delete []lpszString;
				lpszString =NULL;
			}
		}

		return (lwLen+sizeof(WORD ));
	}
	catch(...)
	{
		throw(-1);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	wstring& astrVal  要序列化的数据
// 返 回 值：	数据长度
// 函数说明：   序列化wstring类型的值
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(wstring& astrVal)
{
#ifdef WIN32
	WORD lwLen = (WORD )astrVal.length() * sizeof(wchar_t);    //字符串长度
#else
	WORD lwLen = (WORD )u9_wcslen(astrVal.c_str()) * sizeof(wchar_t);    //字符串长度
#endif 

#ifndef ENCRYPT  //不加密
	try
	{
		if (get_mbType()==ISerialize::LOAD)
		{
			//取出
			moFileStream.read((char*)&lwLen, sizeof(uint16 ));
			if (0 == lwLen )
				return 0;

			char* lpszString = NULL;
			lpszString = new char[lwLen+sizeof(wchar_t)];
			//U9_ASSERT(NULL != lpszString);
			if (NULL == lpszString)
				throw(-1);
			memset(lpszString, 0, lwLen+sizeof(wchar_t));

			moFileStream.read(lpszString, lwLen);
			lpszString[lwLen] = _T('\0');
			astrVal = (wchar_t*)lpszString;
			if (NULL != lpszString )
			{
				delete []lpszString;
				lpszString =NULL;
			}
		}
		else
		{   //存储
			moFileStream.write((char*)&lwLen, sizeof(uint16 ));
			if (0 < lwLen )
				moFileStream.write((char*)astrVal.c_str(), lwLen);
		}

		return (long)(lwLen + sizeof(uint16 ));
	}
	catch(...)
	{
		throw(-1);
	}
#endif
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	Serialize
// 函数参数：	GUID& aotrVal  要序列化的数据
// 返 回 值：	数据长度
// 函数说明：   序列化GUID类型的值
// $_FUNCTION_END ********************************
LONG CFileSerialize::Serialize(GUID& aoValue)
{

	LONG llLen=0;
	llLen += Serialize(aoValue.Data1);
	llLen += Serialize(aoValue.Data2);
	llLen += Serialize(aoValue.Data3);
	WORD lwLen=8;
	llLen += Serialize((BYTE *)aoValue.Data4,lwLen,8);

	return llLen;
}
void CFileSerialize::flush()
{
	moFileStream.flush();
}

