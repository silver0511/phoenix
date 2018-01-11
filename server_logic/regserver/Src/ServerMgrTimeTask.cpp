// $_FILEHEADER_BEGIN ****************************
//版权声明：xxxxx版权所有
//            
// 文件名称：ServerMgrTimeTask.cpp
// 创 建 人: 史云杰
// 文件说明: 负责定时的任务
// $_FILEHEADER_END *****************************

#include "stdafx.h"
#include "ServerMgr.h"
#include "Config.h"


// $_FUNCTION_BEGIN ******************************
// 函数名称： OnTimeCheck
// 函数参数：bool abIsNormal 是否到正常检测时间
// 返 回 值： 无
// 函数说明： 检查处理事件
// $_FUNCTION_END ********************************
void CServerMgr::OnTimeCheck()
{
	// 工程中只需要固定时间一次的Progress,利用liTimeSec来计算时间,目前定为60秒,不精确,因为不需要非常精确

	//以下添加你的服务器定时任务代码
	moUserConMgr.CheckEvent();
	moConfig.CheckEvent();
}
