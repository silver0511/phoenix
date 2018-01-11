#ifndef __ISERIALIZE_H
#define __ISERIALIZE_H

#include "base0/platform.h"
#include "base0/u9_string.h"

using namespace std;

#define DEF_SERIALIZE_ERR_NULL      (-1)
#define DEF_SERIALIZE_ERR_VER_OLD   (-2)
#define DEF_SERIALIZE_ERR_EXCEPTION (-3)
#define DEF_SERIALIZE_EFF_FORMAT    (-4)

class ISerialize
{
public:
	enum	ENUM_TYPE{LOAD=false, STORE=true};	
	enum	ENUM_SERIAL_TYPE{SERIAL_FILE, SERIAL_NET};

public:
	virtual uint32 GetPos()=0;	// return current cursor pos
	virtual void Seek(uint32 adwPos,int8 aiOrigin= ios_base::beg)=0;
	//序列化bool类型的值	
	virtual LONG	Serialize(bool&	abValue)=0;

	//序列化INT8类型的值
	virtual LONG	Serialize(int8&	abyValue)=0;
	//序列化UINT8类型的值
	virtual LONG	Serialize(uint8& abyValue)=0;

	//序列化INT16类型的值
	virtual LONG	Serialize(int16&	abyValue)=0;
	//序列化uint16类型的值
	virtual LONG	Serialize(uint16& abyValue)=0;

	//序列化INT32类型的值(INT32类型实际上也是INT型)	
	virtual LONG	Serialize(int32&	abValue)=0;
	//序列化UINT类型的值(BOOL类型实际上也是UINT型)	
	virtual LONG	Serialize(uint32&	abValue)=0;

	//序列化int64类型的值	
	virtual LONG	Serialize(int64&	ai64Value)=0;
	//序列化uint64类型的值	
	virtual LONG	Serialize(uint64&	ai64Value)=0;

	//序列化float32类型的值	
	virtual LONG	Serialize(float32&	afValue)=0;
	//序列化float64类型的值	
	virtual LONG	Serialize(float64&	afValue)=0;

	//序列化串类型的值(以\0结尾的字符串)	
	virtual LONG	Serialize( char* apValue,WORD awMaxLen)=0;

	//序列化串类型的值(以\0结尾的字符串)	
	virtual LONG	Serialize( wchar_t* apValue,WORD awMaxLen)=0;	

	//序列化字符数组类型的值	
	virtual LONG	Serialize( BYTE* apValue, WORD &awLen,WORD aiBufferLen)=0;

	//序列化string类型的值
	virtual LONG	Serialize(string& astrValue)=0;
	//序列化string类型的值
	virtual LONG	Serialize(wstring& astrValue)=0;
	//序列化string类型的值
	virtual LONG	Serialize(GUID& aoValue)=0;

	virtual LONG	getDataLen()=0;

	virtual LONG	getBufferLen()=0;

	virtual BYTE	*GetBuffer()=0;
public:
	inline ENUM_TYPE get_mbType() const { return mbyType; }

	// $_FUNCTION_BEGIN ******************************
	// 方法名: GetSerialType
	// 访  问: public 
	// 返回值: ISerialize::ENUM_SERIAL_TYPE
	// 说  明: 获得实现的序列化类型
	// $_FUNCTION_END ********************************
	
	inline ENUM_SERIAL_TYPE GetSerialType(){return mbySerialType;};
protected:
	//序列化类型：	//TRUE = 储存	//FALSE = 取出
	ENUM_TYPE			mbyType;	
	ENUM_SERIAL_TYPE	mbySerialType;
};

#endif //__ISERIALIZE_H
