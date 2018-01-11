// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: DebugTraceThread.h
// 创 建 人: 史云杰
// 文件说明: 日志系统线程
// $_FILEHEADER_END ******************************
#include "stdafx.h"
#include <stdio.h>
#include "common/DebugTrace.h"
#include "common/CriticalSection.h"
#include "common/DebugTraceThread.h"
//定义分隔号
#ifdef WIN32
#include <winsock.h>
#endif //WIN32
#ifdef TRACE_LOG
CDebugTraceThread gLogThread;
U9::CCriticalSection gLogSectionThread;

#endif

volatile bool gbLogThread;

STRU_LOG_BUFFER& STRU_LOG_BUFFER::operator<<(void *apValue)
{
	long liValue =(long)(apValue);
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"0x%x",(long)liValue);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"0x%x",(long)liValue);
#endif
	return OperateString(mbyTransBuffer);
}


CDebugTraceThread::CDebugTraceThread()
{
	miType=0;
	miLevel=0;
	miCount=0;
	mbDisplaySourceFileInfo=TRUE;
	miLogWriteWait = 1;
	ZeroMemory(mszFileName,sizeof(mszFileName));
	ZeroMemory(mszAppName,sizeof(mszAppName));
	ZeroMemory(mszTempFileName,sizeof(mszTempFileName));

	miTempFileNameLen = 0;
	mulRecordCount = 0;
#ifndef WIN32
	 mbIsInit = false;
#endif 	
}
CDebugTraceThread::~CDebugTraceThread()
{

}

void CDebugTraceThread::Init(INT aiType,INT aiLevel,
					   char *aszFileName,char * aszAppName,
					   BOOL abDisplaySrouceFileInfo, int aiLogCacheCount)
{
#ifndef WIN32
	if (mbIsInit)
	{
		return;
	}
	mbIsInit = true;
#endif
	miType = aiType;
	moLevelCS.Enter();
	miLevel=aiLevel;
	moLevelCS.Leave();

	mbIsRun = true;


	moFileInfo.Enter();
	mbDisplaySourceFileInfo=abDisplaySrouceFileInfo;
	moFileInfo.Leave();

	strncpy(mszFileName,aszFileName,FILENAMELENGTH);
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
	if(miType != DEF_DISPLAY_TYPE_CONSOLE)
	{
		CreateNewLogFile();
		mLogFile<<aszAppName<<endl<<endl;
	}
	else
	{
		cout<<aszAppName<<endl<<endl;
	}

	moLogUnusedQueue.init(aiLogCacheCount);
	moLogUsedQueue.init(aiLogCacheCount);

	BeginThread(0);

	mulRecordCount = 0;
}

#define DEF_LOG_MAX_LINE (999999)

void CDebugTraceThread::CreateNewLogFile()
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
	if(mLogFile.is_open())
	{
		mLogFile.close();
	}
	mLogFile.open(mszFileName);
}

void CDebugTraceThread::DirectDisplay(STRU_LOG_BUFFER &aoBuffer, int aiLevel,int  aiSuccess,
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

	snprintf(lszDate,512,"%d %d %d-%d-%d %d:%d:%d-%d ",
			 miCount,
			 CSystem::GetThreadID(),
			 lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			 lpstruTime->tm_hour,lpstruTime->tm_min,lpstruTime->tm_sec,loTime.millitm);
	aoBuffer << lszDate << " " << GetStrLevel(aiLevel) << " " << aszFuncName << " ";
	miCount++;
}

STRU_LOG_BUFFER* CDebugTraceThread::WriteBegin(int aiLevel,int  aiSuccess,
									 const char *aszFuncName,
									  const char *aszFileName,int aiLine)
{
	STRU_LOG_BUFFER* lpBuffer = moLogUnusedQueue.malloc();
	if(!lpBuffer)
	{
		return NULL;
	}

	lpBuffer->Init();

	miCount++;

	char lszDate[512+1];
	time_t	ltmNow;	
	struct timeb loTime;
	struct tm * lpstruTime;
	struct tm lostruTime;
	time(&ltmNow);

	lostruTime = *localtime(&ltmNow);
	lpstruTime = &lostruTime;
	ftime( &loTime ); 



#ifdef WIN32
		if(TRUE ==mbDisplaySourceFileInfo)
		{
			_snprintf(lszDate,512,"%d %04d%02d%02d %02d:%02d:%02d-%03d %s:%03d",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,
			lpstruTime->tm_sec,loTime.millitm,
			aszFileName,aiLine);
		}
		else
		{
			_snprintf(lszDate,512,"%d %04d%02d%02d %02d:%02d:%02d-%03d",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,
			lpstruTime->tm_sec,loTime.millitm);
		}
#else
		if(TRUE ==mbDisplaySourceFileInfo)
		{
			snprintf(lszDate,512,"%d %04d%02d%02d %02d:%02d:%02d-%03d %s:%03d",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,
			lpstruTime->tm_sec,loTime.millitm,
			aszFileName,aiLine);
		}
		else
		{
			snprintf(lszDate,512,"%d %04d%02d%02d %02d:%02d:%02d-%03d",
			CSystem::GetThreadID(),
			lpstruTime->tm_year+1900,lpstruTime->tm_mon+1,lpstruTime->tm_mday,
			lpstruTime->tm_hour,lpstruTime->tm_min,
			lpstruTime->tm_sec,loTime.millitm);	
		}
#endif //WIN32
		(*lpBuffer)<<miCount<<" "<<lszDate<<" " <<GetStrLevel(aiLevel)\
			<<" "<<aszFuncName<<" ";

	return lpBuffer;

} 
void CDebugTraceThread::WriteEnd(STRU_LOG_BUFFER* apBuffer)
{
	if(!apBuffer)
	{
		return;
	}
	(*apBuffer) <<"\n";
	moLogUsedQueue.AddTail(apBuffer);
}

void CDebugTraceThread::Run()
{
	STRU_LOG_BUFFER* lpBuffer =NULL;
	while(mbIsRun)
	{
		if(moLogUsedQueue.size() == 0)
		{
			CSystem::Sleep(10);
		}
		else
		{
			lpBuffer = moLogUsedQueue.GetandDelHead();
			if (lpBuffer != NULL)
			{

				if(miType != DEF_DISPLAY_TYPE_CONSOLE)
				{
					mLogFile.write(lpBuffer->mbyBuffer, lpBuffer->miCurrentSize);
					mLogFile.flush();
				}
				else
				{
					cout<<lpBuffer->mbyBuffer;
					cout.flush();
				}


				mulRecordCount++;
				moLogUnusedQueue.free(lpBuffer);
				if(miType != DEF_DISPLAY_TYPE_CONSOLE)
				{
					if( mulRecordCount >=DEF_LOG_MAX_LINE)
					{
						mulRecordCount = 0;
						//生成新的TRACE文件名
						CreateNewLogFile();
					}
				}
			}
		}
	}
}
void CDebugTraceThread::close()
{
	CSystem::Sleep(1000);

	mbIsRun = false;

	CSystem::Sleep(1000);
}


const char* CDebugTraceThread::GetStrLevel(INT aiLevel)
{
	switch (aiLevel)
	{
		case LOG_EMERG:
			return "[EMERG]";
		case LOG_ALERT:
			return "[ALERT]";
		case LOG_CRIT:
			return "[CRIT]";
		case LOG_ERR:
			return "[ERROR]";
		case LOG_WARNING:
			return "[WARNING]";
		case LOG_NOTICE:
			return "[NOTICE]";
		case LOG_INFO:
			return "[INFO]";
		case LOG_DEBUG:
			return "[DEBUG]";
		case LOG_DEBUG_1:
			return "[DEBUG_1]";
		case LOG_DEBUG_2:
			return "[DEBUG_2]";
		default:
			return "[UNKNOWN]";
	}
}
