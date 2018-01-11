// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: U9Assert.h
// 创 建 人: 史云杰
// 文件说明: 断言
// $_FILEHEADER_END ******************************
#ifndef __U9ASSERT_H_
#define __U9ASSERT_H_

#include "U9Exception.h"

namespace U9EXCEPTION
{

	const uint8 ENU_EXCEPTION_LV_CRITICAL		= 0;

	const uint8 ENU_EXCEPTION_LV_INNER_LOGIC	= 1;

	const uint8 ENU_EXCEPTION_LV_DATA_VALIDATE	= 2;

	const uint8 ENU_EXCEPTION_LV_HARDWARE		= 3;

	const uint8 ENU_EXCEPTION_LV_CLIENT_LOGIC	= 4;
	
	//...
	//...
	//...

	const uint8 ENU_EXCEPTION_LV_NOERROR		= 255;
}

#ifdef _DEBUG

const uint8 ENU_EXCEPTION_LV_FATAL = U9EXCEPTION::ENU_EXCEPTION_LV_INNER_LOGIC;

const uint8 ENU_EXCEPTION_LV_RECORD = U9EXCEPTION::ENU_EXCEPTION_LV_CLIENT_LOGIC;

#else

const uint8 ENU_EXCEPTION_LV_FATAL = U9EXCEPTION::ENU_EXCEPTION_LV_INNER_LOGIC;

const uint8 ENU_EXCEPTION_LV_RECORD = U9EXCEPTION::ENU_EXCEPTION_LV_HARDWARE;

#endif // _DEBUG

#ifdef WIN32
#define _U9ASSERT_V2(level, exp)\
	if (!(exp) && (U9LMEXCEPTION::level <=  ENU_EXCEPTION_LV_RECORD)){\
		sprintf_s(&goGobalException.mszFunctionName[0], DEF_RESULT_DESC_LEN, __FUNCTION__);\
		sprintf_s(&goGobalException.mszExceptionDesc[0],DEF_RESULT_DESC_LEN, "Assertion Failed at %s@Line%d: Content \"", __FILE__, __LINE__);{\
		size_t liStrLen = strlen(goGobalException.mszExceptionDesc);\
		sprintf_s(&goGobalException.mszExceptionDesc[liStrLen], DEF_RESULT_DESC_LEN - liStrLen, #exp);}{\
		size_t liStrLen = strlen(goGobalException.mszExceptionDesc);\
		sprintf_s(&goGobalException.mszExceptionDesc[liStrLen], DEF_RESULT_DESC_LEN - liStrLen, "\".\r\n");}\
		goGobalException.SetExceptionLv(U9LMEXCEPTION::level);\
		goGobalException.Dump();\
		if (U9LMEXCEPTION::level < ENU_EXCEPTION_LV_RECORD){\
			throw(goGobalException);\
		}\
	}
#define _U9ASSERT_V3(level, exp, ret)\
    if (!(exp) && (U9LMEXCEPTION::level <=  ENU_EXCEPTION_LV_RECORD)){\
        sprintf_s(&goGobalException.mszFunctionName[0], DEF_RESULT_DESC_LEN, __FUNCTION__);\
        sprintf_s(&goGobalException.mszExceptionDesc[0],DEF_RESULT_DESC_LEN, "Assertion Failed at %s@Line%d: Content \"", __FILE__, __LINE__);{\
        size_t liStrLen = strlen(goGobalException.mszExceptionDesc);\
        sprintf_s(&goGobalException.mszExceptionDesc[liStrLen], DEF_RESULT_DESC_LEN - liStrLen, #exp);}{\
        size_t liStrLen = strlen(goGobalException.mszExceptionDesc);\
        sprintf_s(&goGobalException.mszExceptionDesc[liStrLen], DEF_RESULT_DESC_LEN - liStrLen, "\".\r\n");}\
        goGobalException.SetExceptionLv(U9LMEXCEPTION::level);\
        goGobalException.Dump();\
        return ret;\
    }
#else
#define _U9ASSERT_V2(level, exp)\
	if (!(exp) && (U9LMEXCEPTION::level <=  ENU_EXCEPTION_LV_RECORD)){\
		snprintf(&goGobalException.mszFunctionName[0], DEF_RESULT_DESC_LEN, __FUNCTION__);\
		snprintf(&goGobalException.mszExceptionDesc[0],DEF_RESULT_DESC_LEN, "Assertion Failed at %s@Line%d: Content \"", __FILE__, __LINE__);{\
		size_t liStrLen = strlen(goGobalException.mszExceptionDesc);\
		snprintf(&goGobalException.mszExceptionDesc[liStrLen], DEF_RESULT_DESC_LEN - liStrLen, #exp);}{\
		size_t liStrLen = strlen(goGobalException.mszExceptionDesc);\
		snprintf(&goGobalException.mszExceptionDesc[liStrLen], DEF_RESULT_DESC_LEN - liStrLen, "\".\r\n");}\
		goGobalException.SetExceptionLv(U9LMEXCEPTION::level);\
		goGobalException.Dump();\
		if (U9LMEXCEPTION::level < ENU_EXCEPTION_LV_RECORD){\
			throw(goGobalException);\
		}\
	}

#define _U9ASSERT_V3(level, exp, ret)\
    if (!(exp) && (U9LMEXCEPTION::level <=  ENU_EXCEPTION_LV_RECORD)){\
        snprintf(&goGobalException.mszFunctionName[0], DEF_RESULT_DESC_LEN, __FUNCTION__);\
        snprintf(&goGobalException.mszExceptionDesc[0],DEF_RESULT_DESC_LEN, "Assertion Failed at %s@Line%d: Content \"", __FILE__, __LINE__);{\
        size_t liStrLen = strlen(goGobalException.mszExceptionDesc);\
        snprintf(&goGobalException.mszExceptionDesc[liStrLen], DEF_RESULT_DESC_LEN - liStrLen, #exp);}{\
        size_t liStrLen = strlen(goGobalException.mszExceptionDesc);\
        snprintf(&goGobalException.mszExceptionDesc[liStrLen], DEF_RESULT_DESC_LEN - liStrLen, "\".\r\n");}\
        goGobalException.SetExceptionLv(U9LMEXCEPTION::level);\
        goGobalException.Dump();\
        return ret;\
    }

#endif


#endif//__U9ASSERT_H_