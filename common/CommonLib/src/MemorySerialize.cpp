// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: MemorySerialize.cpp
// 创 建 人: 史云杰
// 文件说明: 内存列化类
// $_FILEHEADER_END ******************************

#include "stdafx.h"
#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <string.h>
#ifdef __SERVER

#include "common/MemorySerialize.h"
#else
#include "common/MemorySerialize.h"
#endif

// $_FUNCTION_BEGIN ******************************
// 函数名称: CMemorySerialize
// 函数参数: char * apBuffer：缓冲buff
//           long alBufLen : buff的长度
//           ENUM_TYPE abyType：序列化类型( 保存/加载 )
// 返 回 值: 
// 函数说明: 构造
// $_FUNCTION_END ********************************
CMemorySerialize::CMemorySerialize(BYTE* apBuffer,ULONG alBufLen, ENUM_TYPE abyType )
{
	mbySerialType=SERIAL_NET;
	mpBuffer = apBuffer;
	mbyType  = abyType;
	mlBufLen = alBufLen;
	mlDataLen = 0;
    mlCurPos = 0;

	//判断本机是否是大头机
	mbIsBigEndian = FALSE;
	unsigned short lusTestVal = 0x1234;
	BYTE *lpFirstChar = (BYTE *)&lusTestVal;
	if (*lpFirstChar == 0x12)
	{
		mbIsBigEndian = TRUE;
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: ~CMemorySerialize
// 函数参数: 
// 返 回 值: 
// 函数说明: 析构函数
// $_FUNCTION_END ********************************
CMemorySerialize::~CMemorySerialize( )
{
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: Serialize
// 函数参数: bool&	abValue( in/out )要序列化的数据
// 返 回 值: void
// 函数说明: 对bool类型数据进行序列化
// $_FUNCTION_END ********************************
LONG CMemorySerialize::Serialize( bool&	abValue )
{
	if ( mlBufLen < ( mlCurPos + sizeof( bool ) ) )
		throw( -1 );

	if ( mbyType == LOAD )//读取
	{
		memcpy( &abValue,mpBuffer + mlCurPos, sizeof( bool ) );		        
	}
	else                //存储
	{
		memcpy( mpBuffer + mlCurPos, &abValue, sizeof( bool ) );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( bool );
    }
    mlCurPos += sizeof(bool);

	return (LONG) sizeof( bool );
}

LONG CMemorySerialize::Serialize( int8&	aiValue )
{
	if ( mlBufLen < ( mlCurPos + sizeof( int8 ) ) )
		throw( -1 );

	if ( mbyType == LOAD )//读取
	{
		memcpy( &aiValue,mpBuffer + mlCurPos, sizeof( int8 ) );		        
	}
	else                //存储
	{
		memcpy( mpBuffer + mlCurPos, &aiValue, sizeof( int8 ) );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( int8 );
    }
    mlCurPos += sizeof(int8);
	return (LONG) sizeof( int8 );
}
LONG CMemorySerialize::Serialize( uint8&	aiValue )
{
	if ( mlBufLen < ( mlCurPos + sizeof( uint8 ) ) )
		throw( -1 );

	if ( mbyType == LOAD )//读取
	{
		memcpy( &aiValue,mpBuffer + mlCurPos, sizeof( uint8 ) );		        
	}
	else                //存储
	{
		memcpy( mpBuffer + mlCurPos, &aiValue, sizeof( uint8 ) );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( uint8 );
    }
    mlCurPos += sizeof(uint8);
	return (LONG) sizeof( uint8 );
}

LONG CMemorySerialize::Serialize( int16&	aiValue )
{
	if ( mlBufLen < ( mlCurPos + sizeof( int16 ) ) )
		throw( -1 );

	WORD lwTempVal = 0;
	if ( mbyType == LOAD )//读取
	{        
		memcpy( &lwTempVal,mpBuffer + mlCurPos, sizeof( int16 ) );
		aiValue = UCntohs(lwTempVal);
	}
	else                //存储
	{
		lwTempVal = UChtons(aiValue);
		memcpy( mpBuffer + mlCurPos, &lwTempVal, sizeof( int16 ) );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( int16 );
    }
    mlCurPos += sizeof(int16);
	return (LONG) sizeof( int16 );
}
LONG CMemorySerialize::Serialize( uint16&	aiValue )
{
	if ( mlBufLen < ( mlCurPos + sizeof( uint16 ) ) )
		throw( -1 );

	WORD lwTempVal = 0;
	if ( mbyType == LOAD )//读取
	{        
		memcpy( &lwTempVal,mpBuffer + mlCurPos, sizeof( uint16 ) );
		aiValue = UCntohs(lwTempVal);
	}
	else                //存储
	{
		lwTempVal = UChtons(aiValue);
		memcpy( mpBuffer + mlCurPos, &lwTempVal, sizeof( uint16 ) );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( uint16 );
    }
    mlCurPos += sizeof(uint16);
	return (LONG) sizeof( uint16 );
}

LONG CMemorySerialize::Serialize( int32&	aiValue )
{
	if ( mlBufLen < ( mlCurPos + sizeof( int32 ) ) )
		throw( -1 );

	DWORD ldwTempVal = 0;
	if ( mbyType == LOAD )//读取
	{        
		memcpy( &ldwTempVal,mpBuffer + mlCurPos, sizeof( int32 ) );		        
		aiValue = UCntohl(ldwTempVal);
	}
	else                //存储
	{        
		ldwTempVal = UChtonl(aiValue);
		memcpy( mpBuffer + mlCurPos, &ldwTempVal, sizeof( int32 ) );        
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( int32 );
    }
    mlCurPos += sizeof(int32);
	return (LONG) sizeof( int32 );
}

LONG CMemorySerialize::Serialize( uint32&	aiValue )
{
	if ( mlBufLen < ( mlCurPos + sizeof( uint32 ) ) )
		throw( -1 );

	DWORD ldwTempVal = 0;
	if ( mbyType == LOAD )//读取
	{        
		memcpy( &ldwTempVal,mpBuffer + mlCurPos, sizeof( uint32 ) );		        
		aiValue = UCntohl(ldwTempVal);
	}
	else                //存储
	{        
		ldwTempVal = UChtonl(aiValue);
		memcpy( mpBuffer + mlCurPos, &ldwTempVal, sizeof( uint32 ) );        
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( uint32 );
    }
    mlCurPos += sizeof(uint32);
	return (LONG) sizeof( uint32 );
}

LONG CMemorySerialize::Serialize( int64&	ai64Value )
{
	if ( mlBufLen < ( mlCurPos + sizeof( int64 ) ) )
		throw( -1 );

	int64 li64TmpVal = 0;
	if ( mbyType == LOAD )//读取
	{
		memcpy( &li64TmpVal,mpBuffer + mlCurPos, sizeof( int64 ) );	
		ai64Value = UCntohi64(li64TmpVal);
	}
	else                //存储
	{
		li64TmpVal = UChtoni64(ai64Value);
		memcpy( mpBuffer + mlCurPos, &li64TmpVal, sizeof( int64 ) );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( int64 );
    }
    mlCurPos += sizeof(int64);
	return (LONG) sizeof( int64 );
}

LONG CMemorySerialize::Serialize( uint64&	ai64Value )
{
	if ( mlBufLen < ( mlCurPos + sizeof( uint64 ) ) )
		throw( -1 );

	int64 li64TmpVal = 0;
	if ( mbyType == LOAD )//读取
	{
		memcpy( &li64TmpVal,mpBuffer + mlCurPos, sizeof( uint64 ) );	
		ai64Value = UCntohi64(li64TmpVal);
	}
	else                //存储
	{
		li64TmpVal = UChtoni64(ai64Value);
		memcpy( mpBuffer + mlCurPos, &li64TmpVal, sizeof( uint64 ) );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( uint64 );
    }
    mlCurPos += sizeof(uint64);
	return (LONG) sizeof( uint64 );
}

LONG CMemorySerialize::Serialize( float32&	afValue )
{
	if ( mlBufLen < ( mlCurPos + sizeof( float32 ) ) )
		throw( -1 );

	float32 lfTmpVal = 0;
	if ( mbyType == LOAD )//读取
	{
		memcpy( &lfTmpVal,mpBuffer + mlCurPos, sizeof( float32 ) );	
		afValue = (float32)UCntohl(lfTmpVal);
	}
	else                //存储
	{
		lfTmpVal = UChtonl(afValue);
		memcpy( mpBuffer + mlCurPos, &lfTmpVal, sizeof( float32 ) );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( float32 );
    }
    mlCurPos += sizeof(float32);
	return (LONG) sizeof( float32 );
}

LONG CMemorySerialize::Serialize( float64&	afValue )
{
	if ( mlBufLen < ( mlCurPos + sizeof( float64 ) ) )
		throw( -1 );

	float64 lfTmpVal = 0;
	if ( mbyType == LOAD )//读取
	{
		memcpy( &lfTmpVal,mpBuffer + mlCurPos, sizeof( float64 ) );	
		afValue = UCntohi64(lfTmpVal);
	}
	else                //存储
	{
		lfTmpVal = UCntohi64(afValue);
		memcpy( mpBuffer + mlCurPos, &lfTmpVal, sizeof( float64 ) );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += sizeof( float64 );
    }
    mlCurPos += sizeof(float64);
	return (LONG) sizeof( float64 );
}

//
//LONG CMemorySerialize::Serialize( INT&	aiValue )
//{
//	if ( mlBufLen < ( mlCurPos + sizeof( INT ) ) )
//		throw( -1 );
//
//	DWORD ldwTmpVal = 0;
//	if ( mbyType == LOAD )//读取
//	{
//		memcpy( &ldwTmpVal,mpBuffer + mlCurPos, sizeof( INT ) );	
//		aiValue = UCntohl(ldwTmpVal);
//	}
//	else                //存储
//	{
//		ldwTmpVal = UChtonl(aiValue);
//		memcpy( mpBuffer + mlCurPos, &ldwTmpVal, sizeof( INT ) );
//	}
//    if (mlCurPos == mlDataLen)
//    {
//        mlDataLen += sizeof( INT );
//    }
//    mlCurPos += sizeof(INT);
//	return (LONG) sizeof( INT );
//}
//LONG CMemorySerialize::Serialize(UINT&	aiValue )
//{
//	if ( mlBufLen < ( mlCurPos + sizeof( UINT ) ) )
//		throw( -1 );
//
//	DWORD ldwTmpVal = 0;
//	if ( mbyType == LOAD )//读取
//	{
//		memcpy( &ldwTmpVal,mpBuffer + mlCurPos, sizeof( UINT ) );	
//		aiValue = UCntohl(ldwTmpVal);
//	}
//	else                //存储
//	{
//		ldwTmpVal = UChtonl(aiValue);
//		memcpy( mpBuffer + mlCurPos, &ldwTmpVal, sizeof( UINT ) );
//	}
//    if (mlCurPos == mlDataLen)
//    {
//        mlDataLen += sizeof( UINT );
//    }
//    mlCurPos += sizeof(UINT);
//	return (LONG) sizeof( UINT );
//}
// $_FUNCTION_BEGIN ******************************
// 函数名称: Serialize
// 函数参数: char * apValue		字符串数据 ( in/out )要序列化的数据
//			WORD awBufferLen	容纳此字符串数据的缓存区大小
// 返 回 值: void
// 函数说明: 序列化以0结尾的字符串数据
// $_FUNCTION_END ********************************
LONG CMemorySerialize::Serialize( char* apValue,WORD awMaxLen )
{
	if ( mlBufLen < ( mlCurPos+2 ) )
		throw( -1 );

	WORD	lwLen = 0;
	if ( mbyType == LOAD )    //读取
	{
		//首先读取长度
		memcpy( &lwLen,mpBuffer + mlCurPos, sizeof( uint16 ) );
        if (mlCurPos == mlDataLen)
        {
            mlDataLen += sizeof( uint16 );
        }
        mlCurPos += sizeof( uint16 );

		//读取数据本身
		if ( ( lwLen >= awMaxLen ) || ( ( mlCurPos + lwLen ) > mlBufLen ) )
		{
			throw( -1 );
		}
		memcpy( apValue, mpBuffer + mlDataLen, lwLen );
		apValue[lwLen] = 0;
	}
	else    //存储
	{
		//首先存储长度
		lwLen = (WORD)strlen( apValue );
		if ( ( lwLen >= awMaxLen ) || ( lwLen+mlCurPos+2 > mlBufLen ) )
			throw( -1 );

		memcpy( mpBuffer + mlCurPos, &lwLen, sizeof(uint16) );
        if (mlCurPos == mlDataLen)
        {
            mlDataLen += sizeof( uint16 );
        }
        mlCurPos += sizeof( uint16 );

		//存储数据本身
		memcpy( mpBuffer+mlDataLen, apValue, lwLen );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += lwLen;
    }
    mlCurPos += lwLen;
	return (LONG) (lwLen+sizeof(uint16));
}

LONG CMemorySerialize::Serialize(wchar_t* apValue,WORD awMaxLen)
{
	if ( mlBufLen < ( mlCurPos+2 ) )
		throw( -1 );

	WORD	lwLen = 0;
	if ( mbyType == LOAD )    //读取
	{
		//首先读取长度
		memcpy( &lwLen,mpBuffer + mlCurPos, sizeof(uint16) );
        if (mlCurPos == mlDataLen)
        {
            mlDataLen += sizeof( uint16 );
        }
        mlCurPos += sizeof( uint16 );
        //读取数据本身
		if ( lwLen >= awMaxLen || ( mlCurPos+lwLen*2 ) > mlBufLen  )
		{
			throw( -1 );
		}
		apValue[lwLen] = 0;
		lwLen =lwLen*sizeof(wchar_t);
		memcpy( (char *)apValue, mpBuffer + mlCurPos,lwLen);
	}
	else    //存储
	{
		//首先存储长度
		lwLen = (WORD)u9_wcslen( apValue );
		if ( ( lwLen >= awMaxLen ) || ( lwLen*sizeof(wchar_t) + mlCurPos+2 > mlBufLen ) )
			throw( -1 );

		memcpy( mpBuffer+mlCurPos, &lwLen, sizeof(uint16) );
        if (mlCurPos == mlDataLen)
        {
            mlDataLen += sizeof( uint16 );
        }
        mlCurPos += sizeof( uint16 );

		lwLen =lwLen*sizeof(wchar_t);
		//存储数据本身
		memcpy( mpBuffer + mlCurPos, (char*) apValue, lwLen );
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += lwLen;
    }
    mlCurPos += lwLen;
	return (LONG) ((lwLen*sizeof(wchar_t))+sizeof(uint16));
}

// $_FUNCTION_BEGIN ******************************
// 函数名称: Serialize
// 函数参数: char * apValue		数据 ( in/out )要序列化的数据
//			WORD& awLen			此数据的真正长度
//			WORD awBufferLen	容纳此数据的缓存区大小
// 返 回 值: void
// 函数说明: 序列化二进制数据
// $_FUNCTION_END ********************************
LONG CMemorySerialize::Serialize( BYTE* apValue, WORD  &awLen,WORD aiBufferLen )
{
	if ( ( awLen > aiBufferLen ) || ( mlBufLen < ( mlCurPos+awLen ) ) )
		throw ( -1 );

	if ( mbyType == LOAD )    //读取
	{
		//长度不够
		if ( ( awLen > aiBufferLen ) ||( mlBufLen == mlCurPos)  )
			throw ( -1 );

		if(0 == awLen)
			return 0 ;
		memcpy( apValue, mpBuffer + mlCurPos, awLen );            
	}
	else    //存储数据本身
	{
		if(0 == awLen)
			return 0;
		if ( mlBufLen < ( mlCurPos + awLen ))
			throw ( -1 );
		memcpy( mpBuffer + mlCurPos, apValue, awLen );                        
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += awLen;
    }
    mlCurPos += awLen;
	return (LONG) (awLen);
}

/****************************************************************
函 数 名：Serialize(string& astrValue);
功能描述：序列化string类型的值
输入参数：string alValue
输出参数：
返 回 值：int 1=成功；－1＝失败(throw，有外部捕获)
创 建 者：梅亮犬
****************************************************************/
LONG CMemorySerialize::Serialize(string& astrValue)
{
	WORD lwLen = (WORD)astrValue.size();    //字符串长度
	if ( ( mlBufLen < ( mlCurPos + lwLen ) ) )
		throw ( -1 );

	if ( mbyType == LOAD )    //读取
	{
		//因为外部制定了读取长度，所以不需要对数据长度进行序列化
		memcpy((char*)&lwLen, mpBuffer + mlCurPos, sizeof(uint16));
        if (mlCurPos == mlDataLen)
        {
            mlDataLen += sizeof(uint16);
        }
        mlCurPos += sizeof(uint16);

        if(lwLen>0)
		{
			char* lpString = NULL;
			lpString = new char[lwLen + 1];
			if (NULL == lpString)
				throw(-1);
			ZeroMemory(lpString, lwLen + 1);
			memcpy( lpString, mpBuffer + mlCurPos, lwLen );            
			astrValue = lpString;
			if (NULL != lpString )
			{
				delete [] lpString;
				lpString =NULL;
			}
		}

	}
	else    //存储数据本身
	{
		memcpy(mpBuffer + mlCurPos, (char*)&lwLen, sizeof(WORD));
        if (mlCurPos == mlDataLen)
        {
            mlDataLen += sizeof(uint16);
        }
        mlCurPos += sizeof(uint16);

        if(lwLen >0)
			memcpy( mpBuffer + mlCurPos, astrValue.c_str(), lwLen );                        
	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += lwLen;
    }
    mlCurPos += lwLen;

    return (LONG) (lwLen+sizeof(uint16));
}
/****************************************************************
函 数 名：Serialize(wstring& astrValue);
功能描述：序列化wstring类型的值
输入参数：long alValue
输出参数：
返 回 值：int 1=成功；－1＝失败(throw，有外部捕获)
创 建 者：梅亮犬
****************************************************************/
LONG CMemorySerialize::Serialize(wstring& astrValue)
{
	if ( mlBufLen < ( mlCurPos+2 ) )
		throw( -1 );
	uint16 lwLen = 0;
	if ( mbyType == LOAD )    //读取
	{
		//因为外部制定了读取长度，所以不需要对数据长度进行序列化
		memcpy((char*)&lwLen, mpBuffer + mlCurPos, sizeof(WORD));
        if (mlCurPos == mlDataLen)
        {
            mlDataLen += sizeof(uint16);
        }
        mlCurPos += sizeof(uint16);

        if(lwLen >0)
		{
			wchar_t* lpString = NULL;
			lpString = new wchar_t[lwLen + 1];
			if (NULL == lpString)
				throw(-1);
			lpString[lwLen]=0;

			memcpy( (char *)lpString, mpBuffer + mlCurPos, lwLen*2);
			astrValue = lpString;
			if (NULL != lpString )
			{
				delete [] lpString;
				lpString =NULL;
			}
		}
	}
	else    //存储数据本身
	{
		lwLen= (WORD)u9_strlen(astrValue.c_str());
		memcpy(mpBuffer + mlCurPos, (char*)&lwLen, sizeof(WORD));
        if (mlCurPos == mlDataLen)
        {
            mlDataLen += sizeof(uint16);
        }
        mlCurPos += sizeof(uint16);

        if(lwLen>0)
		{
			memcpy( mpBuffer + mlCurPos, (char *)astrValue.c_str(),lwLen*2); 
		}

	}
    if (mlCurPos == mlDataLen)
    {
        mlDataLen += lwLen*2;
    }
    mlCurPos += lwLen*2;
	return (LONG) ((lwLen*2) + sizeof(uint16));
}

LONG CMemorySerialize::Serialize(GUID& aoValue)
{
	if ( mlBufLen < ( mlCurPos + sizeof( GUID ) ) )
		throw( -1 );
	LONG llLen=0;
	llLen += Serialize(aoValue.Data1);
	llLen += Serialize(aoValue.Data2);
	llLen += Serialize(aoValue.Data3);
	WORD lwLen=8;
	llLen += Serialize((BYTE *)aoValue.Data4, lwLen, 8);
	return llLen;
}