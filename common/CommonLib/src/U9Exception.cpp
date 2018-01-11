// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: U9Exception.cpp
// 创 建 人: 史云杰
// 文件说明: 异常处理
// $_FILEHEADER_END ******************************
#include "stdafx.h"

#include "common/U9Exception.h"
#include "common/U9Assert.h"

CU9Exception::CU9Exception( void )
{
	Reset();
	return;
}

void CU9Exception::SetExceptionLv( uint8 aiLevel )
{
	miExceptionLv = aiLevel;
}

void CU9Exception::Dump( void )
{
	LOG_TRACE(LOG_CRIT, false, mszFunctionName, mszExceptionDesc);
	return;
}

void CU9Exception::Reset()
{
	miExceptionLv = U9EXCEPTION::ENU_EXCEPTION_LV_NOERROR;
	memset(mszFunctionName, 0,  sizeof(mszFunctionName));
	memset(mszExceptionDesc, 0, sizeof(mszFunctionName));
}

void CU9Exception::Execute()
{
	if (miExceptionLv <= ENU_EXCEPTION_LV_FATAL)
	{
		throw;
	}
	Reset();
	return;
}