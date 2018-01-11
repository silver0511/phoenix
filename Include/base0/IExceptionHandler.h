// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: IExceptionHandler.h
// 创 建 人: 史云杰
// 文件说明: 异常处理类 
// 使用 在程序入口处加入 	InstallExceptionHandler(_T("test"),_T("2.11.111"));
// $_FILEHEADER_END *****************************
#ifndef __IEXCEPTIONHANDER_H
#define __IEXCEPTIONHANDER_H
#include "base0/platform.h"

DLL_EXPORT INT InstallExceptionHandler(U9_CHAR * aszModulName,
												 U9_CHAR * aszVer);
#ifdef _WIN32
DLL_EXPORT INT WriteMinidumpWithException(EXCEPTION_POINTERS* apExceptionInfo);
#endif
#endif //__IEXCEPTIONHANDER_H
