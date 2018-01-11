// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: U9Process.h
// 创 建 人: 史云杰
// 文件说明: 进程管理类,Linux,windows通用的程序使用进程时都应由此类来管理
// $_FILEHEADER_END ******************************

#ifndef __U9PROCESS_H
#define __U9PROCESS_H

#include "base0/platform.h"
#include "base0/u9_string.h"

#ifdef WIN32
	typedef HANDLE PID ;
#else
	typedef pid_t PID ;
#define STILL_ACTIVE ((DWORD )0x00000103L)  // 表示等待的进程仍在运行,只有当IsHang为false时,通过exitcode返回此值
#endif

enum ENMU_PROCESS_WAIT_RESULT
{
	PROCESS_WAIT_SUCCESS = 0,	// 表示等待的进程已经结束
	PROCESS_WAIT_FAIL,			// 等待出现错误
	PROCESS_GET_EXITCODE_ERROR,	// 表示等待成功,但是获得进程的返回值出现错误
};

// 进程管理类,Linux,windows通用的程序使用进程时都应由此类来管理
class CU9Process
{
public:
	CU9Process();
	~CU9Process();
	
	// 创建进程
	// $_FUNCTION_BEGIN ******************************
	// 方法名: CreateProcess
	// 访  问: public static 
	// 参  数: U9_CHAR * apszCmdLine	需要运行的程序
	// 参  数: PID * apPID	返回新创建进程的ID
	// 返回值: bool 是否创建成功
	// 说  明: 
	// $_FUNCTION_END ********************************
	static bool CreateProcess(const wchar_t* apszCmdLine, PID* apPID);
	static bool CreateProcess(const char* apszCmdLine, PID* apPID, const char* apArg = NULL);

	// $_FUNCTION_BEGIN ******************************
	// 方法名: WaitForProcessEnd
	// 访  问: public static 
	// 参  数: PID aProcessID 需要等待的进程ID
	// 参  数: bool IsHang 是否阻塞等待,即要等待的进程没有结束时是否一直等下去,
	//				为false时,就算等待的进程没有结束也立即返回,此时此函数相当于
	//				一个检测进程运行状态的函数.
	// 参  数: int* piExitCode 等待的函数的返回值
	// 返回值: DWORD	见ENMU_PROCESS_WAIT_RESULT
	// 说  明:
	// $_FUNCTION_END ********************************
	static DWORD WaitForProcessEnd(PID aPID, bool IsHang, int* piExitCode);

	static bool ClosePID(PID aPID);

};




#endif // __PROCESS_H_
