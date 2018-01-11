#include "stdafx.h"
#include "ServerMgr.h"
#include "network/ITransProtocolMgr.h"
#include "network/ITransMgrCallBack.h"
#include "./ClientSide/ClientSide.h"
#include "common/md5.h"
#include "Config.h"

void CServerMgr::OnTimeCheck()
{	
	moConfig.CheckEvent();
	//以下添加你的服务器定时任务代码

	return;
}	
