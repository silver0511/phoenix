// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：DebugTrace.cpp
// 创建人： 史云杰
// 文件说明：常量定义
// $_FILEHEADER_END ******************************
#include "stdafx.h"
#include "common/DebugTrace.h"
#include "common/CriticalSection.h"
#include <stdio.h>
//定义分隔号
#ifdef WIN32
#include <winsock.h>
#endif //WIN32
#ifdef TRACE_LOG
CDebugTrace gLog;
U9::CCriticalSection gLogSection;

#endif
CDebugTrace::CDebugTrace()
{
	miType=0;
	miLevel=0;
	miCount=0;
	mbDisplaySourceFileInfo=TRUE;

	ZeroMemory(mszFileName,sizeof(mszFileName));
	ZeroMemory(mszAppName,sizeof(mszAppName));
	ZeroMemory(mszTempFileName,sizeof(mszTempFileName));

	miTempFileNameLen = 0;
	mulRecordCount = 0;
}
CDebugTrace::~CDebugTrace()
{

}

void CDebugTrace::Init(int aiType,int aiLevel,
					   char *aszFileName,char * aszAppName,
					   BOOL abDisplaySrouceFileInfo)
{
	if (aiType < DEF_DISPLAY_TYPE_NONE)
	{
		aiType = DEF_DISPLAY_TYPE_NONE;
	}
	else if (aiType > DEF_DISPLAY_TYPE_ALL)
	{
		aiType = DEF_DISPLAY_TYPE_ALL;
	}
	miType=aiType;
	miLevel=aiLevel;
	mbDisplaySourceFileInfo=abDisplaySrouceFileInfo;

	miTempFileNameLen = (int32)strlen(aszFileName);
	if(miTempFileNameLen<4)
	{
		U9_ASSERT(FALSE);
		memcpy(mszTempFileName,aszFileName,miTempFileNameLen);
	}
	else
	{
		memcpy(mszTempFileName,aszFileName,miTempFileNameLen-4);
	}
	
	strncpy(mszAppName,aszAppName,FILENAMELENGTH);
	CreateNewLogFile();
	if (DEF_DISPLAY_TYPE_FILE==miType||DEF_DISPLAY_TYPE_ALL==miType)
	{
		
		mLogFile<<aszAppName<<endl<<endl;
		//mLogFile.close();
	}
	

	mulRecordCount = 0;
}
void CDebugTrace::SetDisplayOption(INT aiType)
{
	if (aiType < DEF_DISPLAY_TYPE_NONE)
	{
		aiType = DEF_DISPLAY_TYPE_NONE;
	}
	else if (aiType > DEF_DISPLAY_TYPE_ALL)
	{
		aiType = DEF_DISPLAY_TYPE_ALL;
	}
	miType = aiType;
}
#define DEF_LOG_MAX_LINE (999999)

void CDebugTrace::CreateNewLogFile()
{
	static int32 liIndex=0;
	char lszFileName[FILENAMELENGTH+1]={0};
	
#ifdef WIN32
	_snprintf(mszFileName,FILENAMELENGTH, "%s_%d.log", 
		mszTempFileName,liIndex++);
#else //!WIN32
	snprintf(mszFileName, FILENAMELENGTH,"%s_%d.log", 
		mszTempFileName,liIndex++);
#endif //WIN32
	mLogFile.open(mszFileName);
}
CDebugTrace &CDebugTrace::WriteBegin(int aiLevel,int  aiSuccess,
									 const char *aszFuncName,
									 const char *aszFileName,int aiLine)
{
	
	char lszDate[512+1];
	time_t	ltmNow;	
	struct timeb loTime;
	struct tm * lpstruTime;
	struct tm lostruTime;
	time(&ltmNow);

	lostruTime = *localtime(&ltmNow);
	lpstruTime = &lostruTime;
	ftime( &loTime ); 
	//liTime = loTime.millitm;

	mulRecordCount++;
	if( mulRecordCount >=DEF_LOG_MAX_LINE)
	{
		mLogFile.close();
		mulRecordCount = 0;
		//生成新的TRACE文件名
		CreateNewLogFile();

	}
	
	if (DEF_DISPLAY_TYPE_FILE==miType||DEF_DISPLAY_TYPE_ALL==miType){
#ifdef WIN32

		if(TRUE ==mbDisplaySourceFileInfo)
			_snprintf(lszDate,512,"%d %04d%02d%02d %02d:%02d:%02d-%03d %s:%03d",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,
			lpstruTime->tm_sec,loTime.millitm,
			aszFileName,aiLine);
		else
			_snprintf(lszDate,512,"%d %04d%02d%02d %02d:%02d:%02d-%03d",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,
			lpstruTime->tm_sec,loTime.millitm);
#else
		if(TRUE ==mbDisplaySourceFileInfo)
			snprintf(lszDate,512,"%d %04d%02d%02d %02d:%02d:%02d-%03d %s:%03d",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,
			lpstruTime->tm_sec,loTime.millitm,
			aszFileName,aiLine);
		else
			snprintf(lszDate,512,"%d %04d%02d%02d %02d:%02d:%02d-%03d",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,
			lpstruTime->tm_sec,loTime.millitm);	
#endif //WIN32
		//mLogFile.clear();
		//mLogFile.open(mszFileName,ios::out|ios::app);
		//if(mLogFile.fail())
		//{//无法打开文件
		//	return *this;
		//}
		mLogFile<<miCount<<" "<<lszDate<<" "<<aiSuccess<<" " <<aiLevel\
			<<" "<<aszFuncName<<" ";

	}
	if (DEF_DISPLAY_TYPE_CONSOLE<=miType){

#ifdef WIN32
		_snprintf(lszDate,512,"%d %d-%d-%d %d:%d:%d-%d ",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,lpstruTime->tm_sec,loTime.millitm);
		printf("%s %d %s ",lszDate,aiSuccess,aszFuncName);
#else
		snprintf(lszDate,512,"%d %d-%d-%d %d:%d:%d-%d ",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,lpstruTime->tm_sec,loTime.millitm);
		printf("%s %d %s ",lszDate,aiSuccess,aszFuncName);

#endif //WIN32
	}
	miCount++;

	return *this;
} 
void CDebugTrace::WriteEnd()
{
	if (DEF_DISPLAY_TYPE_FILE==miType||DEF_DISPLAY_TYPE_ALL==miType){
		mLogFile<<endl;
		//mLogFile.flush();
		//mLogFile.close();
	}
	if (DEF_DISPLAY_TYPE_CONSOLE<=miType){
		cout<<"\n\r";
	}
}



