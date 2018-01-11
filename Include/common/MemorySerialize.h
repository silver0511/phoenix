// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: MemorySerialize.h
// 创 建 人: 史云杰
// 文件说明: 内存列化类
// $_FILEHEADER_END ******************************

#ifndef __MEMORYSERIALIZE_H
#define __MEMORYSERIALIZE_H


#include "base0/platform.h"
#include "ISerialize.h"

//用来序列化通讯包的类
class CMemorySerialize:public ISerialize
{
public:
	//构造和析构
	CMemorySerialize();
	CMemorySerialize(BYTE* apBuffer,ULONG alBufLen, ENUM_TYPE abyType);
	~CMemorySerialize();

public:
	
	virtual void Seek(uint32 adwPos,int8 aiOrigin = ios_base::beg)
    {
       mlCurPos = adwPos;
    };
	virtual uint32 GetPos() { return mlCurPos; }	// return current cursor pos

	//序列化bool类型的值	
	virtual LONG	Serialize(bool&	abValue);
	
	//序列化INT8类型的值
	virtual LONG	Serialize(int8&	abyValue);
	//序列化UINT8类型的值
	virtual LONG	Serialize(uint8& abyValue);

	//序列化INT16类型的值
	virtual LONG	Serialize(int16&	abyValue);
	//序列化uint16类型的值
	virtual LONG	Serialize(uint16& abyValue);

	//序列化INT32类型的值(INT32类型实际上也是INT型)	
	virtual LONG	Serialize(int32&	abValue);
	//序列化UINT类型的值(BOOL类型实际上也是UINT型)	
	virtual LONG	Serialize(uint32&	abValue);

	//序列化int64类型的值	
	virtual LONG	Serialize(int64&	ai64Value);
	//序列化uint64类型的值	
	virtual LONG	Serialize(uint64&	ai64Value);

	//序列化float32类型的值	
	virtual LONG	Serialize(float32&	afValue);
	//序列化float64类型的值	
	virtual LONG	Serialize(float64&	afValue);

    
	//序列化串类型的值(以\0结尾的字符串)	
	virtual LONG	Serialize( char* apValue,WORD awMaxLen);
	
	//序列化串类型的值(以\0结尾的字符串)	
	virtual LONG	Serialize( wchar_t* apValue,WORD awMaxLen);	

	//序列化字符数组类型的值	
	virtual LONG	Serialize( BYTE* apValue, WORD &awLen,WORD aiBufferLen);

	//序列化string类型的值
	virtual LONG	Serialize(string& astrValue);
	//序列化string类型的值
	virtual LONG	Serialize(wstring& astrValue);

	//序列化string类型的值
	virtual LONG	Serialize(GUID& aoValue);
	virtual LONG	getDataLen(){return mlDataLen;};
	
	virtual LONG getBufferLen(){return mlBufLen;}
public:
	BYTE *GetBuffer(){return mpBuffer;}

    //网络字序转本机字序(__int64)
	inline INT64  UCntohi64(INT64 ai64EndVal);
    //本机字序转网络字序(__int64)
    inline INT64 UChtoni64(INT64 ai64HostVal);

    //网络字序转本机字序(DWORD)
    inline DWORD  UCntohl(DWORD adwEndVal);
    //本机字序转网络字序(DWORD)
    inline DWORD UChtonl(DWORD adwHostVal);

    //网络字序转本机字序(WORD)
    inline WORD UCntohs(WORD awEndVal);
    //本机字序转网络字序(WORD)
    inline WORD UChtons(WORD awHostVal);
protected:	
	BYTE* mpBuffer;
    ULONG mlCurPos;
	ULONG mlDataLen;		//数据长度
	ULONG mlBufLen;			//buffer长度
	BOOL mbIsBigEndian;		//大头小头
};




//由于我们使用的大多少机器都是小头机，所以确定网络传输直接使用小头机的字节顺序
//只有遇到大头机时才进行转换。
//网络字序转本机字序(INT64)
INT64 CMemorySerialize::UCntohi64(INT64 ai64EndVal)
{
	if (FALSE == mbIsBigEndian)//小头机不需要转换
	{
		return ai64EndVal;
	}

	INT64 li64BigEndVal = 0;
	char *lpBigCharPtr = (char *)&li64BigEndVal;
	char *lpLittleCharPtr = (char *)&ai64EndVal;
	int liLen = sizeof(INT64);

	for (int i = 0; i < liLen; i++)
	{
		lpBigCharPtr[i] = lpLittleCharPtr[liLen - i - 1];
	}
	return li64BigEndVal;
}

//本机字序转网络字序(INT64)
INT64 CMemorySerialize::UChtoni64(INT64 ai64HostVal)
{
	if (FALSE == mbIsBigEndian)//小头机不需要转换
	{
		return ai64HostVal;
	}

	__int64 li64HostVal = 0;
	char *lpBigCharPtr = (char *)&ai64HostVal;
	char *lpLittleCharPtr = (char *)&li64HostVal;
	int liLen = sizeof(INT64);

	for (int i = 0; i < liLen; i++)
	{
		lpLittleCharPtr[i] = lpBigCharPtr[liLen - i - 1];        
	}
	return li64HostVal;
}

//网络字序转本机字序(DWORD)
DWORD CMemorySerialize::UCntohl(DWORD adwEndVal)
{
	if (FALSE == mbIsBigEndian)//小头机不需要转换
	{
		return adwEndVal;
	}

	DWORD ldwBigEndVal = 0;
	char *lpBigCharPtr = (char *)&ldwBigEndVal;
	char *lpLittleCharPtr = (char *)&adwEndVal;
	int liLen = sizeof(DWORD);

	for (int i = 0; i < liLen; i++)
	{
		lpBigCharPtr[i] = lpLittleCharPtr[liLen - i - 1];
	}
	return ldwBigEndVal;
}

//本机字序转网络字序(DWORD)
DWORD CMemorySerialize::UChtonl(DWORD adwHostVal)
{
	if (FALSE == mbIsBigEndian)//小头机不需要转换
	{
		return adwHostVal;
	}

	DWORD ldwHostVal = 0;
	char *lpBigCharPtr = (char *)&adwHostVal;
	char *lpLittleCharPtr = (char *)&ldwHostVal;
	int liLen = sizeof(DWORD);

	for (int i = 0; i < liLen; i++)
	{
		lpLittleCharPtr[i] = lpBigCharPtr[liLen - i - 1];        
	}
	return ldwHostVal;
}

//网络字序转本机字序(WORD)
WORD CMemorySerialize::UCntohs(WORD awEndVal)
{
	if (FALSE == mbIsBigEndian)//小头机不需要转换
	{
		return awEndVal;
	}

	WORD lwBigEndVal = 0;
	char *lpBigCharPtr = (char *)&lwBigEndVal;
	char *lpLittleCharPtr = (char *)&awEndVal;
	int liLen = sizeof(WORD);

	for (int i = 0; i < liLen; i++)
	{
		lpBigCharPtr[i] = lpLittleCharPtr[liLen - i - 1];
	}
	return lwBigEndVal;
}

//本机字序转网络字序(WORD)
WORD CMemorySerialize::UChtons(WORD awHostVal)
{
	if (FALSE == mbIsBigEndian)//小头机不需要转换
	{
		return awHostVal;
	}

	WORD lwHostVal = 0;
	char *lpBigCharPtr = (char *)&awHostVal;
	char *lpLittleCharPtr = (char *)&lwHostVal;
	int liLen = sizeof(WORD);

	for (int i = 0; i < liLen; i++)
	{
		lpLittleCharPtr[i] = lpBigCharPtr[liLen - i - 1];        
	}
	return lwHostVal;
}


#endif //__NETORDERSERIALIZE_H
