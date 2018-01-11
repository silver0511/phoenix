// $_FILEHEADER_BEGIN ****************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称：BaseConfig.cpp
// 创建人： 史云杰
// 文件说明：配置信息
// $_FILEHEADER_END ******************************
#include "stdafx.h"
#include "BaseConfig.h"

CBaseConfig::CBaseConfig()
{
	miDynamicReadInterval = 30;
	
}

CBaseConfig::~CBaseConfig()
{

}

BOOL CBaseConfig::DynamicLoad()
{
	CRITICAL_SECTION_HELPER(moCriCS);
	if (moIniFile.Open(mstrIniFilename))
	{
		BOOL lbResult=DynamicRead();
		moIniFile.Close();
		return lbResult;
	}
	return FALSE;
}

BOOL CBaseConfig::Load(const char *aszFileName)
{
	if(NULL == aszFileName)
		return FALSE;

	if(strcmp(mstrIniFilename.c_str(),aszFileName))
	{
		mstrIniFilename = GetAppPathA() + aszFileName;
	}

	if (moIniFile.Open(mstrIniFilename))
	{
		BOOL lbResult=Read();
		if(!lbResult)
		{
			moIniFile.Close();
			return FALSE;
		}
		lbResult = DynamicRead();

		moIniFile.Close();
		return lbResult;
	}
	return FALSE;
}

void CBaseConfig::CheckEvent()
{
	static int siSecCount = 0;
	if(siSecCount++ > miDynamicReadInterval)
	{
		siSecCount = 0;
		DynamicLoad();
	}
}
