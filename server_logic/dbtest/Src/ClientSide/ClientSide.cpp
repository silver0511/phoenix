#include "stdafx.h"
#include "const/ErrorCodeDef.h"
#include "GlobalResource.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/client/UserPackDef.h"
#include "Config.h"
#include "ClientSide.h"
#include "../ServerMgr.h"
#include "jsoncmdpacket/JsonCmdPacket.h"

SERVERID transforServerid(std::string szserverid)
{
	SERVERID dstserverid = 0;
	if (szserverid.empty())
	{
		return 0;
	}
	std::stringstream ss;
	
	ss << szserverid;

	ss >> dstserverid;

	return dstserverid;
}


CClientSide::CClientSide(CServerMgr *apoServerMgr)
{
	mpServerMgr = apoServerMgr;
	mbOpen = false;
}

CClientSide::~CClientSide()
{
	mbOpen = false;
}

bool CClientSide::open()
{
	printf("CClientSide::open begin\n");
	
	STRU_OPTION_TCP lstruTcpOpt;
	strncpy(lstruTcpOpt.mszAppName, g_server_name.c_str(),DEF_MAX_APP_NAME);
	lstruTcpOpt.miLogLevel = mpServerMgr->moConfig.miTCPLogLevel;
	lstruTcpOpt.mbIsServer = TRUE;
	lstruTcpOpt.muiMaxConnCount =   mpServerMgr->moConfig.miMaxConnCount;
	lstruTcpOpt.mulIPAddr = goLocalServerInfo.mlServerIP;
	lstruTcpOpt.mwPort = goLocalServerInfo.mwServerPort;
	lstruTcpOpt.mbyDealThreadCount =  mpServerMgr->moConfig.miDealThreadCount;
	lstruTcpOpt.mbyRecvThreadCount = mpServerMgr->moConfig.miRecvThreadCount;

	lstruTcpOpt.miMaxRecvPackageCount = mpServerMgr->moConfig.miMaxRecvPackageCount;
	lstruTcpOpt.miMaxSendPackageCount = mpServerMgr->moConfig.miMaxSendPackageCount;
	lstruTcpOpt.mbUseRelationPtr = false;
	lstruTcpOpt.mbyEpollCount = mpServerMgr->moConfig.miEpollCount;
	lstruTcpOpt.muiKeepAlive = 0;

	if(!ClientPack::GetInstance().Open(this, lstruTcpOpt, ENUM_CONNECT_MODE_TCP))
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," Initialize tcp error.");
		return false;
	}

	if (1 != moDatabaseMgr.open( mpServerMgr->moConfig))
	{
			LOG_TRACE(2,false,__FUNCTION__," initialize dbmgr  error.\n");
			return -1;
	}	

	printf("CClientSide::open end ip:%s, port:%d\n", GetIPString(lstruTcpOpt.mulIPAddr).c_str(), ntohs(lstruTcpOpt.mwPort));
	
	mbOpen = true;
	return true;
}


INT CClientSide::OnSendDataError(IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen)
{
	return 0;
}

INT CClientSide::OnError(UINT aiErrType)
{
	return 0;
}
 
VOID CClientSide::CheckEvent(bool abIsNormal)
{
	if(abIsNormal != true)
	{
		return ;
	}

	if(NULL != mpServerMgr)
	{
		mpServerMgr->OnTimeCheck();
	}

	static uint32 siSecCount = 0;

	if (0 != mpServerMgr->moConfig.m_statinterval)
	{
		if (siSecCount > mpServerMgr->moConfig.m_statinterval)
		{
			Statistics();
			siSecCount = 0;
		}
		else
		{
			siSecCount++;
		}
	}


	ClientPack::GetInstance().DisplayStatisticsInfo();
}

VOID CClientSide::Statistics()
{	
	return;
}

//TCP连接建立
void CClientSide::OnSessionConnected(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Connected " 
                                   << " IP=" << GetIPString(aiIP)
                                   << ":" << ntohs(awPort)
	                               << " Socket=" << ahSocket);

}

//TCP连接建立
void CClientSide::OnSessionConnectedEx(const SESSIONID& aiPeerID,void* apSocketInfo, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	return;	
}

void CClientSide::OnSessionConnectFailure(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Connected " 
		                               <<" IP=" << GetIPString(aiIP)
                                       << ":"<< ntohs(awPort)
		                               <<" Socket="<< ahSocket);
}

//客户端关闭连接
void CClientSide::OnSessionClose(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Disconnected "
		                               <<" Errcode="<<aiPeerID
		                               <<" IP=" << GetIPString(aiIP)
                                       << ":"<< ntohs(awPort)
		                               << " Socket="<< ahSocket);	
}
void CClientSide::OnSessionCloseEx(const SESSIONID & aiPeerID,void*apRelationPtr, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	return;	
}

INT CClientSide::OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
{	
	CJsonCmdPacket jsoncmd((char *)apData, awLen);
	std::string szmsg = jsoncmd.GetString();
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "send to db msg:" << szmsg);

	std::string cmd = jsoncmd.GetAttrib("cmd");

	if (cmd == "FDLISTRQ")
	{
		return OnFriendListRq(ahSocket, aiIP, awPort, jsoncmd);
	}
	else if (cmd == "FDREMARKRQ")
	{
		return OnFdRemarkRq(ahSocket, aiIP, awPort, jsoncmd);
	}
	else if (cmd == "FDDELRQ")
	{
		return OnFdDelRq(ahSocket, aiIP, awPort, jsoncmd);
	}
	else if (cmd == "FDCONFIRMRQ")
	{
		return OnFriendCONFIRMRq(ahSocket, aiIP, awPort, jsoncmd);
	}	

	return 1;
}

INT CClientSide::OnNewRecvData(SOCKET ahSocket,  void* apRelationPtr, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
{
	LOG_TRACE(LOG_ERR, false, __FUNCTION__, "OnNewRecvData failed");		
	return -1;
}

INT CClientSide::OnFriendListRq(SOCKET ahSocket, IPTYPE aiIP, WORD awPort, CJsonCmdPacket& cmd)
{
	std::string userid = cmd.GetAttrib("userid");
	std::string cometid = cmd.GetAttrib("cometid");
	std::string serverid = cmd.GetAttrib("serverid");
	unsigned int sessionid = cmd.GetAttribUN("sessionid");
	unsigned short paltform = cmd.GetAttribUS("platform");
	
	std::vector<STRFRIENDINFO>friendlist;
	moDatabaseMgr.GetFriendList(userid, friendlist);
	
	CJsonCmdPacket Response;
	Response.PutAttrib("cmd","FDLISTRS");
	Response.PutAttrib("userid",userid);
	Response.PutAttrib("cometid",cometid);
	Response.PutAttrib("serverid",serverid);
	Response.PutAttribUN("sessionid",sessionid);
	Response.PutAttribUS("platform",paltform);

	CJsonCmdPacket Jsonfriendlist;
	for (int i = 0; i < friendlist.size(); i++)
	{
		CJsonCmdPacket Jsonfriendinfo;
		STRFRIENDINFO friendinfo = friendlist[i];
		Jsonfriendinfo.PutAttrib("friendid",friendinfo.szuserid);
		Jsonfriendinfo.PutAttrib("remarkname",friendinfo.szremarkname);	
		Jsonfriendinfo.PutAttrib("sourcetype",friendinfo.szsourcetype);
		Jsonfriendlist.root.append(Jsonfriendinfo.root);
	}

	Response.root["friendinfo"] = Jsonfriendlist.root;

	std::string szRsponse = Response.GetString();
	
	ClientPack::GetInstance().Send2Comet(ahSocket, (BYTE *)(const_cast<char *> (szRsponse.c_str())), szRsponse.size());
	
	return 1;
}

INT CClientSide::OnFdRemarkRq(SOCKET ahSocket, IPTYPE aiIP, WORD awPort, CJsonCmdPacket& cmd)
{
	std::string userid = cmd.GetAttrib("userid");
	std::string cometid = cmd.GetAttrib("cometid");
	std::string serverid = cmd.GetAttrib("serverid");
	unsigned int sessionid = cmd.GetAttribUN("sessionid");
	unsigned short paltform = cmd.GetAttribUS("platform");
	std::string peeruserid = cmd.GetAttrib("peeruserid");
	std::string remarkname  = cmd.GetAttrib("remarkname");

	moDatabaseMgr.GetFriendRemark(userid, peeruserid, remarkname);	
	return 1;
}

INT CClientSide::OnFdDelRq(SOCKET ahSocket, IPTYPE aiIP, WORD awPort, CJsonCmdPacket& cmd)
{
	std::string userid = cmd.GetAttrib("userid");
	std::string cometid = cmd.GetAttrib("cometid");
	std::string serverid = cmd.GetAttrib("serverid");
	unsigned int sessionid = cmd.GetAttribUN("sessionid");
	unsigned short paltform = cmd.GetAttribUS("platform");
	std::string peeruserid = cmd.GetAttrib("peeruserid");

	moDatabaseMgr.GetFriendDel(userid, peeruserid);
	return 1;
}

INT CClientSide::OnFriendCONFIRMRq(SOCKET ahSocket, IPTYPE aiIP, WORD awPort, CJsonCmdPacket& cmd)
{
	std::string userid = cmd.GetAttrib("ownuserid");
	std::string cometid = cmd.GetAttrib("cometid");
	std::string serverid = cmd.GetAttrib("serverid");
	unsigned int sessionid = cmd.GetAttribUN("sessionid");
	unsigned short paltform = cmd.GetAttribUS("platform");	
    std::string peeruserid = cmd.GetAttrib("peeruserid");

    STRFRIENDINFO peerinfo;
    peerinfo.szuserid = peeruserid;
    peerinfo.szremarkname = cmd.GetAttrib("peerremark");
    peerinfo.szsourcetype = cmd.GetAttrib("sourcetype");
    
    STRFRIENDINFO owninfo;
    owninfo.szuserid = userid;
    owninfo.szremarkname = "";
    owninfo.szsourcetype = cmd.GetAttrib("sourcetype");
	
	moDatabaseMgr.GetFriendAdd(userid,peerinfo);
	moDatabaseMgr.GetFriendAdd(peeruserid,owninfo);
	return 1;
}

