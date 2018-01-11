/***********************************************************
//xxxxx版权所有
//  
文 件 名：	debugtrace.h
创 建 人：	史云杰
说    明：	日志类
***********************************************************/

#ifndef __DEBUGTRACE_H
#define __DEBUGTRACE_H
 
#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include "CriticalSection.h"
#include "common/System.h"
#include "base0/platform.h"
using namespace std;


#pragma warning(disable: 4786)

#define FILENAMELENGTH	(255)			//文件名大小
#define TRACE_BUF_LEN	(500 * 1000)	//100K 缓存区
#define DEF_ONCE_LOG_BUFFER_SIZE (2046)	// 一条日志2K大小
#define LONGFILE _T("log.txt")



//日志类
class CDebugTrace
{
public:

	CDebugTrace();
	~CDebugTrace();

	void Init(INT aiType,INT aiLevel,
		char *aszFileName, char* aszAppName,
		BOOL abDisplaySrouceFileInfo=TRUE);
	CDebugTrace & WriteBegin(INT aiLevel,INT abSuccess,
		const char* aszFuncName, const char* aszFileName,INT Line);

	void WriteEnd();
	inline INT GetLevel();
	inline void SetLevel(INT aiLevel);
	inline BOOL GetDisplaySourceFileInfo();
	inline void SetDisplaySourceFileInfo(BOOL abDisplaySrouceFileInfo);
	void SetDisplayOption(INT aiType);

	inline INT GetType();
	inline CDebugTrace & operator <<(char *);
	inline CDebugTrace & operator <<(const char *);
	inline CDebugTrace & operator <<(wchar_t *);
	inline CDebugTrace & operator <<(const wchar_t *);
	inline CDebugTrace & operator <<(bool );

	inline CDebugTrace & operator <<(int8 );
	inline CDebugTrace & operator <<(int16 );
	inline CDebugTrace & operator <<(int32 );
	inline CDebugTrace & operator <<(int64 );

	inline CDebugTrace & operator <<(uint8 );
	inline CDebugTrace & operator <<(uint16 );
	inline CDebugTrace & operator <<(uint32 );
	inline CDebugTrace & operator <<(uint64 );


	inline CDebugTrace & operator <<(LONG );
	inline CDebugTrace & operator <<(ULONG );


	inline CDebugTrace & operator <<(float32);
	inline CDebugTrace & operator <<(float64 );

	inline CDebugTrace & operator <<(string  &);
	inline CDebugTrace & operator <<(const string  &);
	inline CDebugTrace & operator <<(wstring  &);
	inline CDebugTrace & operator <<(const wstring  &);
	inline CDebugTrace & operator <<(const GUID &);
private:
	//操作
	template<typename T>
	CDebugTrace& OperateValue(T aValue)
	{
		if (DEF_DISPLAY_TYPE_FILE==miType||DEF_DISPLAY_TYPE_ALL==miType)
			mLogFile<<aValue;

		if (DEF_DISPLAY_TYPE_CONSOLE<=miType)
			cout<<aValue;
		return *this;
	}
	CDebugTrace& OperateString(const char *apValue)
	{
		if (DEF_DISPLAY_TYPE_FILE==miType||DEF_DISPLAY_TYPE_ALL==miType)
			mLogFile<<apValue;

		if (DEF_DISPLAY_TYPE_CONSOLE<=miType)
			cout<<apValue;
		return *this;
	}
	CDebugTrace& OperateWString(const wchar_t *apValue)
	{
		if (DEF_DISPLAY_TYPE_FILE==miType||DEF_DISPLAY_TYPE_ALL==miType)
		{
			mLogFile.write((char *)apValue,(std::streamsize)u9_wcslen(apValue)*sizeof(wchar_t));
		}

		if (DEF_DISPLAY_TYPE_CONSOLE<=miType)
			wcout<<apValue;
		return *this;
	}
	void CreateNewLogFile();
private:
	int32 miType;
	int32 miLevel;
	BOOL mbDisplaySourceFileInfo;//是否显示源文件信息
	char mszFileName[FILENAMELENGTH+1];
	char mszAppName[FILENAMELENGTH+1];
	ofstream mLogFile;
	int32 miCount;
	ULONG mulRecordCount;

	char mszTempFileName[FILENAMELENGTH+1];
	int32 miTempFileNameLen;
};
//////////////////////////////////////////////////////////////////////////
//inline 

void CDebugTrace::SetLevel(INT aiLevel)
{
	miLevel = aiLevel;
}
int CDebugTrace::GetLevel()
{
	return miLevel; 
}
int CDebugTrace::GetType()
{
	return miType; 
}
BOOL CDebugTrace::GetDisplaySourceFileInfo()
{
	return mbDisplaySourceFileInfo;
}
void CDebugTrace::SetDisplaySourceFileInfo(BOOL abDisplaySrouceFileInfo)
{
	mbDisplaySourceFileInfo=abDisplaySrouceFileInfo; 
}
#ifndef WIN32

#endif 


CDebugTrace&  CDebugTrace::operator <<(char * aszValue)
{
	if (aszValue != NULL)
	{
		return OperateString((const char*)aszValue);
	}
	return *this;
}
CDebugTrace&  CDebugTrace::operator <<(const char * aszValue)
{
	if (aszValue != NULL)
	{
		return OperateString(aszValue);
	}
	return *this;
}
CDebugTrace&  CDebugTrace::operator <<(wchar_t * aszValue)
{
	if (aszValue != NULL)
	{
		return OperateWString((const wchar_t*)aszValue);
	}
	return *this;
}
CDebugTrace&  CDebugTrace::operator <<(const wchar_t * aszValue)
{
	if (aszValue != NULL)
	{
		return OperateWString(aszValue);
	}
	return *this;
}
CDebugTrace&  CDebugTrace::operator <<(bool abValue)
{
	if (0 ==abValue){

		if (DEF_DISPLAY_TYPE_FILE==miType||DEF_DISPLAY_TYPE_ALL==miType)
			mLogFile<<"false";

		if (DEF_DISPLAY_TYPE_CONSOLE<=miType)
			cout<<"false";
	}
	else{
		if (DEF_DISPLAY_TYPE_FILE==miType||DEF_DISPLAY_TYPE_ALL==miType)
			mLogFile<<"true";

		if (DEF_DISPLAY_TYPE_CONSOLE<=miType)
			cout<<"true";
	}
	return *this;
}

CDebugTrace&  CDebugTrace::operator <<(int8 aValue)
{
	return OperateValue<int32>(aValue);
}
CDebugTrace&  CDebugTrace::operator <<(int16 aValue)
{
	return OperateValue<int16>(aValue);
}
CDebugTrace&  CDebugTrace::operator <<(int32 aValue)
{
	return OperateValue<int32>(aValue);
}
CDebugTrace&  CDebugTrace::operator <<(int64 aValue)
{
	return OperateValue<int64>(aValue);
}
CDebugTrace&  CDebugTrace::operator <<(uint8 aValue)
{
	return OperateValue<uint16>(aValue);
}
CDebugTrace&  CDebugTrace::operator <<(uint16 aValue)
{
	return OperateValue<uint16>(aValue);
}
CDebugTrace&  CDebugTrace::operator <<(uint32 aValue)
{
	return OperateValue<uint32>(aValue);
}
CDebugTrace&  CDebugTrace::operator <<(uint64 aValue)
{
	return OperateValue<uint64>(aValue);
}

CDebugTrace&  CDebugTrace::operator <<(LONG aiValue)
{
	return OperateValue<LONG>(aiValue);
}
CDebugTrace&  CDebugTrace::operator <<(ULONG aiValue)
{
	return OperateValue<ULONG>(aiValue);
}
CDebugTrace&  CDebugTrace::operator <<(float32 afValue)
{
	return OperateValue<float32>(afValue);
}
CDebugTrace&  CDebugTrace::operator <<(float64 afValue)
{
	return OperateValue<float64>(afValue);
}
CDebugTrace&  CDebugTrace::operator <<(string  &astrValue)
{
	return OperateString(astrValue.c_str());
}
CDebugTrace&  CDebugTrace::operator <<(const string  &astrValue)
{
	return OperateString(astrValue.c_str());
}
CDebugTrace&  CDebugTrace::operator <<(wstring  &astrValue)
{
	return OperateWString(astrValue.c_str());
}
CDebugTrace&  CDebugTrace::operator <<(const wstring  &astrValue)
{
	return OperateWString(astrValue.c_str());
}
CDebugTrace&  CDebugTrace::operator <<(const GUID &aValue)
{
	//{00000000-0000-0000-0002-000000000000}
	char lszTemp[64+1];
	lszTemp[64]=0;
#ifdef WIN32	
	_snprintf(lszTemp,64,"{%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}",aValue.Data1,
						 aValue.Data2,aValue.Data3,
						 aValue.Data4[0],aValue.Data4[1],
						 aValue.Data4[2],aValue.Data4[3],
						 aValue.Data4[4],aValue.Data4[5],
						 aValue.Data4[6],aValue.Data4[7]);
#else
	snprintf(lszTemp,64,"{%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}",aValue.Data1,
						aValue.Data2,aValue.Data3,
						aValue.Data4[0],aValue.Data4[1],
						aValue.Data4[2],aValue.Data4[3],
						aValue.Data4[4],aValue.Data4[5],
						aValue.Data4[6],aValue.Data4[7]);

#endif
	return OperateString(lszTemp);
}



#endif //__DEBUGTRACE_H
