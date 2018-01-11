// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: U9Process.cpp
// 创 建 人: 史云杰
// 文件说明: 进程管理类,Linux,windows通用的程序使用进程时都应由此类来管理
// $_FILEHEADER_END ******************************

#include "stdafx.h"
#include "U9Process.h"
#include <iostream>
using namespace std;
#ifndef WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#endif


CU9Process::CU9Process()
{

}

CU9Process::~CU9Process()
{

}

//-----------------------------------------------------------------------------
bool CU9Process::CreateProcess( const wchar_t* apszCmdLine, PID* apPID )
{
#ifdef WIN32
	PROCESS_INFORMATION loProcInfo;
	ZeroMemory( &loProcInfo, sizeof(loProcInfo) );
	STARTUPINFOW loStarInfo;
	ZeroMemory( &loStarInfo, sizeof(loStarInfo) );
	loStarInfo.cb = sizeof(loStarInfo);

	wchar_t lpszCommandLine[MAX_PATH+1] = {0};
	wcsncpy(lpszCommandLine, apszCmdLine, MAX_PATH);
	if(::CreateProcessW(NULL, lpszCommandLine,
		NULL, NULL, FALSE, 0, NULL, NULL, &loStarInfo, &loProcInfo))
	{
		/*::CloseHandle(loProcInfo.hProcess);*/
		::CloseHandle(loProcInfo.hThread);

		*apPID = loProcInfo.hProcess;
		return true;
	}
	else
	{
		*apPID = NULL;
		return false;
	}
#else
	pid_t pid;
	if((pid = fork()) < 0)
	{
	    LOG_TRACE(LOG_CRIT, true, __FUNCTION__,  "vfork error.\n");
            return false;
	}
	else if(0 == pid)
	{
	     LOG_TRACE(LOG_INFO, true, __FUNCTION__,  "sub Process running.\n");
		    
		char pCmd[(MAX_PATH+1) * 6] = {0};
		UNICODE2UTF8(apszCmdLine, pCmd, (MAX_PATH+1) * 6);

	    cout <<"pCmd:" <<pCmd <<endl;
	    if(system(pCmd) < 0)
	    {
		cout <<"execl Error." <<endl;
		exit(-1);
	    }
	}
	else
	{
		if(apPID)
		{
			*apPID = pid;
		}
	}
#endif
    return true;

}

bool CU9Process::CreateProcess( const char* apszCmdLine, PID* apPID, const char* apArg)
{
	cout <<"apszCmdLine:" <<apszCmdLine <<endl;
	cout <<"apArg:" <<apArg <<endl;
#ifdef WIN32
	PROCESS_INFORMATION loProcInfo;
	ZeroMemory( &loProcInfo, sizeof(loProcInfo) );
	STARTUPINFOA loStarInfo;
	ZeroMemory( &loStarInfo, sizeof(loStarInfo) );
	loStarInfo.cb = sizeof(loStarInfo);

	char lpszCommandLine[MAX_PATH+1] = {0};
	strncpy(lpszCommandLine, apszCmdLine, MAX_PATH);
	if(::CreateProcessA(NULL, lpszCommandLine,
		NULL, NULL, FALSE, 0, NULL, NULL, &loStarInfo, &loProcInfo))
	{
		/*::CloseHandle(loProcInfo.hProcess);*/
		::CloseHandle(loProcInfo.hThread);

		*apPID = loProcInfo.hProcess;
		return true;
	}
	else
	{
		if(apPID)
		{
			*apPID = NULL;
		}
		return false;
	}
#else
	pid_t pid;
	if((pid = fork()) < 0)
	{
		LOG_TRACE(LOG_CRIT, true, __FUNCTION__,  "vfork error.\n");
		return false;
	}
	else if(0 == pid)
	{
		LOG_TRACE(LOG_INFO, true, __FUNCTION__,  "sub Process running.\n");

		if(system(apszCmdLine) < 0)
		{
			cout <<"execl Error:" <<errno <<endl;
			exit(-1);
		}
	}
	else
	{
		if(apPID)
		{
			*apPID = pid;
		}
	}
#endif
	return true;

}


//-----------------------------------------------------------------------------
DWORD CU9Process::WaitForProcessEnd( PID aPID, bool IsHang, int* piExitCode)
{
#ifdef WIN32
	if(aPID == NULL)
	{
		return PROCESS_WAIT_FAIL;
	}
	else
	{
		if(IsHang)
		{
			// 无限等待
			DWORD ldwResult = ::WaitForSingleObject(aPID, INFINITE);
			if(WAIT_OBJECT_0 == ldwResult)
			{
				// 等待成功
				DWORD ldwExitCode;
				if(GetExitCodeProcess(aPID, &ldwExitCode))
				{
					// 获得返回值正确
					*piExitCode = ldwExitCode;
					return PROCESS_WAIT_SUCCESS;
				}
				else
				{
					return PROCESS_GET_EXITCODE_ERROR;
				}
				return PROCESS_WAIT_SUCCESS;
			}
			else 
			{
				return PROCESS_WAIT_FAIL;
			}
		}
		else
		{
			// 仅仅查状态
			DWORD ldwResult= ::WaitForSingleObject(aPID, 0);
			if(WAIT_OBJECT_0 == ldwResult)
			{
				// 进程已经结束
				return PROCESS_WAIT_SUCCESS;
			}
			else if(WAIT_TIMEOUT == ldwResult)
			{
				// 仍在运行
				*piExitCode = STILL_ACTIVE;
				return PROCESS_WAIT_SUCCESS;
			}
			else
			{
				return PROCESS_WAIT_FAIL;
			}
		}

	}
#else

	// 返回的状态值
	int liStatus;
	if(IsHang)
	{
	    if(waitpid(aPID, &liStatus, 0) != aPID)
	    {
		LOG_TRACE(LOG_CRIT, true, __FUNCTION__,  "waitpid error.\n");
		return PROCESS_WAIT_FAIL;
	    }
	    else if(WIFEXITED(liStatus))
	    {
		// 正常退出
		LOG_TRACE(LOG_INFO, true, __FUNCTION__,  "sub exit success.\n");
		*piExitCode = WEXITSTATUS(liStatus);
		return PROCESS_WAIT_SUCCESS;

	    }
	    else
	    {
		LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "sub process exit abnormal.\n");
		*piExitCode = -1;
		return PROCESS_WAIT_SUCCESS;
	    }

	}
	else
	{
	    PID lPID = waitpid(aPID, &liStatus, WNOHANG); 
	    if (0 == lPID)
	    {
		LOG_TRACE(LOG_INFO, true, __FUNCTION__,  "sub process Still Running.\n");
		// 还没有运行结束
		*piExitCode = STILL_ACTIVE;
		return PROCESS_WAIT_SUCCESS;
	    }
	    else if(aPID == lPID)
	    {
		if(WIFEXITED(liStatus))
		{
		    // 正常退出
		    LOG_TRACE(LOG_INFO, true, __FUNCTION__,  "sub process Exit normal.\n");
		    *piExitCode = WEXITSTATUS(liStatus);
		}
		else
		{
		    LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "sub process exit abnormal.\n");
		    *piExitCode = -1;
		    return PROCESS_WAIT_SUCCESS;
		}
	    }
	    else
	    {
		LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "Error Wait..\n");
		return PROCESS_WAIT_FAIL;
	    }
	}


#endif
}

//-----------------------------------------------------------------------------
bool CU9Process::ClosePID( PID aPID )
{
#ifdef WIN32
	if(NULL == aPID)
	{
		return false;
	}

	::CloseHandle(aPID);
#else

#endif

	return true;
}
