// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: FileSerialize.h
// 创 建 人: 史云杰
// 文件说明: 创建文件
// $_FILEHEADER_END *****************************
#ifndef __FILESERIAIZE_H_
#define __FILESERIAIZE_H_

#include "base0/platform.h"
#include "ISerialize.h"
#include <fstream>

#ifndef WIN32
#include <bits/ios_base.h>
#endif

using namespace std;

//-----------------------------------------------------------------------------
class CFileSerialize :public ISerialize
{
public:
	CFileSerialize(ENUM_TYPE abType);
	CFileSerialize();
	~CFileSerialize();
	void SetType(ENUM_TYPE abType);
	
#ifdef WIN32
	bool Open(const U9_CHAR* apFileName, ios_base::open_mode aiMode);
#else
	bool Open(const U9_CHAR* apFileName, std::_Ios_Openmode aiMode);
#endif
	bool Open(const U9_CHAR* apFileName);

	virtual uint32 GetPos();	// return current cursor pos
	virtual void Seek(uint32 adwPos,int8 aiOrigin= ios_base::beg);

public:	
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
	//序列化uint32类型的值(uint32类型实际上也是UINT型)	
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

	//序列化GUID类型的值
	virtual LONG	Serialize(GUID& aoValue);

	virtual LONG	getDataLen(){return 0;};

	virtual LONG	getBufferLen(){return 0;};
	void flush();
protected:
	
	//文件流对象
	fstream moFileStream;
};

#endif // __CFILESERIAIZE_H_
