// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：debug_log.h
// 创建人： 史云杰
// 文件说明：调试日志 宏定义
// $_FILEHEADER_END ******************************
#ifndef __DEBUG_LOG_H
#define __DEBUG_LOG_H


//log display typle
#define		DEF_DISPLAY_TYPE_NONE			(0)			// 不显示日志
#define		DEF_DISPLAY_TYPE_FILE			(1)			// 以文件方式
#define		DEF_DISPLAY_TYPE_CONSOLE		(2)			// 控制台方式
#define		DEF_DISPLAY_TYPE_THREAD			(3)			// 背景线程写日志方式
#define		DEF_DISPLAY_TYPE_ALL			(4)			// 两种方式显示


#define LOG_EMERG       (0)     //系统不可用日志
#define LOG_ALERT       (1)     //危险错误，有可能危急系统
#define LOG_CRIT        (2)     //不能忽略的严重错误
#define LOG_ERR         (3)     //错误是不能忽略的
#define LOG_WARNING     (4)     //警告条件不是很重要,可以忽略的
#define LOG_NOTICE      (5)     //值得注意的条件
#define LOG_INFO        (6)     //普通日志，显示信息
#define LOG_DEBUG       (7)     //输出的是程序处理细节

#define LOG_DEBUG_1	(8)
#define LOG_DEBUG_2	(9)

//可记录选项
#define LOG_CAN_LEVEL       (1)
#define LOG_CAN_FILELINE    (2)
#define LOG_CAN_THREAD      (4)

//#ifndef TRACE_LOG
//#define TRACE_LOG
//#endif

#ifdef _DEBUG
#define TRACE_LOG_CURRENT_LEVEL (4)
#else
#define TRACE_LOG_CURRENT_LEVEL (3)
#endif

//分隔字符
#define LOG_SYMBOL endl
#define LOG_BEGIN  1119

#ifdef TRACE_LOG




#include "DebugTrace.h"
#include "DebugTraceThread.h"
#include "CriticalSection.h"
//定义调试宏 v0.1

using namespace U9;

extern U9::CCriticalSection gLogSection;
extern CDebugTraceThread gLogThread;
//extern CDebugTrace gLogThread;
// $_FUNCTION_BEGIN ******************************
// 函数名称：	INIT_LOG_V02
// 函数参数：	BYTE abyType			记录类型
//				BYTE abyLevel			记录的等级
//				String astrLogFileName	日志文件名
//				String astrAppName		应用名
// 返 回 值： 
// 函数说明： 初始化日志宏
// $_FUNCTION_END ********************************
//#define INIT_LOG_V02(abyType,abyLevel,astrLogFileName,astrAppName)  \
//	gLog.Init(abyType,abyLevel,astrLogFileName,astrAppName,FALSE);\
//	gLog.SetDisplaySourceFileInfo(FALSE);
	
// $_FUNCTION_BEGIN ******************************
// 函数名称： INIT_LOG_V03
// 函数参数：BYTE abyType          记录类型
//         BYTE abyLevel         记录的等级
//         String astrLogFileName 日志文件名
//         String astrAppName     应用名
//         BOOL DisplaySourceFileInfo     是否显示源文件信息。
// 返 回 值： 
// 函数说明： 初始化日志宏
// $_FUNCTION_END ********************************
//#define INIT_LOG_V03(abyType,abyLevel,astrLogFileName,astrAppName,DisplaySourceFileInfo) \
//	gLog.Init(abyType,abyLevel,astrLogFileName,astrAppName,DisplaySourceFileInfo);\

#define INIT_LOG_V04(abyType,abyLevel,astrLogFileName,astrAppName,DisplaySourceFileInfo,aiMaxCahceCount) \
	gLogThread.Init(abyType,abyLevel,astrLogFileName,astrAppName,DisplaySourceFileInfo,aiMaxCahceCount);\

// $_FUNCTION_BEGIN ******************************
// 函数名称： CLOSE_LOG_V02
// 函数参数：
// 返 回 值： 
// 函数说明： 关闭日志宏
// $_FUNCTION_END ********************************
#define CLOSE_LOG_V02()


// $_FUNCTION_BEGIN ******************************
// 函数名称： TRACE_LOG_V02
// 函数参数：
// 返 回 值： 
// 函数说明： 记录日志宏
// $_FUNCTION_END ********************************
/*
#define TRACE_LOG_V04(abyLevel,abySuccess,astrFuncName,astrValue) \
	if (DEF_DISPLAY_TYPE_NONE != gLogThread.GetType())\
{\
	if (gLogThread.GetLevel()>=abyLevel)\
    {\
	gLogSection.Enter();\
	gLogThread.WriteBegin(abyLevel,abySuccess,\
	astrFuncName,__FILE__,__LINE__)<<astrValue;\
    gLogThread.WriteEnd();\
	gLogSection.Leave();\
    }\
};
*/
#define TRACE_LOG_V04(abyLevel,abySuccess,astrFuncName,astrValue) \
	if (DEF_DISPLAY_TYPE_NONE != gLogThread.GetType())\
{\
	if (gLogThread.GetLevel()>=abyLevel)\
	{\
        STRU_LOG_BUFFER* lpBuffer = gLogThread.WriteBegin(abyLevel,abySuccess,\
                                    astrFuncName,__FILE__,__LINE__);\
        if(NULL != lpBuffer)\
        {\
            (*lpBuffer) <<astrValue;gLogThread.WriteEnd(lpBuffer);\
        }\
	}\
};

#define CLOSE_LOG_V04() {gLogThread.close();}

//
//#define SET_LOG_LEVEL_V02(abyLevel) \
//	gLog.SetLevel(abyLevel);		
//
//#define SET_SET_DISPLAYSOURCEFILEINFO_V02(abValue) \
//	gLog.SetDisplaySourceFileInfo(abValue)
//
//#define SET_LOG_SET_DISPLAY_OPTION_V02(aiValue) \
//	gLog.SetDisplayOption(aiValue);		


#define SET_LOG_LEVEL_V04(abyLevel) \
	if(gLogThread.GetLevel() != abyLevel) gLogThread.SetLevel(abyLevel);		

#define SET_SET_DISPLAYSOURCEFILEINFO_V04(abValue) \
	gLogThread.SetDisplaySourceFileInfo(abValue)

#define SET_LOG_SET_DISPLAY_OPTION_V04(aiValue) \
	gLogThread.SetDisplayOption(aiValue);		

#define LOG_INIT    INIT_LOG_V04
//#define LOG_INIT_NEW INIT_LOG_V03
#define LOG_TRACE   TRACE_LOG_V04
#define LOG_CLOSE   CLOSE_LOG_V04
#define LOG_SET_LEVEL	SET_LOG_LEVEL_V04


#define LOG_SET_DISPLAYSOURCEFILEINFO	SET_SET_DISPLAYSOURCEFILEINFO_V04
#define LOG_SET_DISPLAYOPTION	SET_LOG_SET_DISPLAY_OPTION_V04



#else      //写调试日志 为空  
//			BYTE abyType          记录类型
//         BYTE abyLevel         记录的等级
//         String astrLogFileName 日志文件名
//         String astrAppName     应用名
#define LOG_INIT(abyType,abyLevel,astrLogFileName,astrAppName)
#define LOG_INIT_NEW(abyType,abyLevel,astrLogFileName,astrAppName,DisplaySourceFileInfo)
#define LOG_TRACE(abyLevel,abySuccess,astrFuncName,astrValue)
#define LOG_SET_LEVEL(abyLevel)

#define LOG_SET_DISPLAYSOURCEFILEINFO(abValue)
#define LOG_SET_DISPLAYOPTION(abValue)

#define LOG_CLOSE()

#endif

#endif //__DEBUG_LOG_H





