#include "stdafx.h"
#include "network/packet/ClientPack.h"


ClientPack::ClientPack()
{
    p_tcp_mgr = NULL;
}

ClientPack::~ClientPack()
{
    if(NULL != p_tcp_mgr)
    {
        p_tcp_mgr->Close();
        p_tcp_mgr->Release();
        p_tcp_mgr = NULL;
    }
}

void ClientPack::Close()
{
    if(NULL != p_tcp_mgr)
    {
        p_tcp_mgr->Close();
        p_tcp_mgr->Release();
        p_tcp_mgr = NULL;
    }
}

bool ClientPack::Open(ITransMgrCallBack *interface, STRU_OPTION_TCP &option_tcp, ENUM_CONNECT_MODE connect_mode)
{
    if(connect_mode == ENUM_CONNECT_MODE_WS)
    {
        CreateNetTCP("NETWS", (void**)&p_tcp_mgr);
    }
    else if(connect_mode == ENUM_CONNECT_MODE_TCP)
    {
        CreateNetTCP("NetTCP", (void**)&p_tcp_mgr);
    }
    else if(connect_mode == ENUM_CONNECT_MODE_HTTP)
    {
        CreateNetTCP("NetHttp", (void**)&p_tcp_mgr);
    }
    
    if(NULL == p_tcp_mgr)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__," Create TCP TransMgr Error.");
        return false;
    }
    p_tcp_mgr->Init(interface, &option_tcp);
    if(!p_tcp_mgr->Open())
    {
        LOG_TRACE(LOG_ERR,true,__FUNCTION__," Initialize tcp error.");
        return false;
    }

    return true;
}

INT ClientPack::DisConnectTo(const SOCKET &socket)
{
    return p_tcp_mgr->DisConnectTo(socket);
}

bool ClientPack::AddRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
    p_tcp_mgr->AddRelationPtr(apSocketInfo, apRelationPtr);
}

bool ClientPack::DelRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
    p_tcp_mgr->DelRelationPtr(apSocketInfo, apRelationPtr);
}

INT ClientPack::DisplayStatisticsInfo()
{
    static int satis_count = 0;
    static STRU_STATISTICS_TCP stru_statistics_tcp;
    if(satis_count < 60)
    {
        satis_count++;
        return 1;
    }
    satis_count = 0;
    p_tcp_mgr->GetNetStatisticsInfo(&stru_statistics_tcp);
    LOG_TRACE(LOG_CRIT, true, __FUNCTION__, "statis info "
            <<" t_send_count=" << stru_statistics_tcp.t_send_count << " t_recv_count="<< stru_statistics_tcp.t_recv_count
            <<" send_count_sec="<< stru_statistics_tcp.send_count_sec << " recv_count_sec="<< stru_statistics_tcp.recv_count_sec
            <<" cur_con="<< stru_statistics_tcp.cur_con << " t_lost_send="<< stru_statistics_tcp.t_lost_send
            <<" t_lost_recv="<< stru_statistics_tcp.t_lost_recv <<" s_start_time="<< stru_statistics_tcp.s_start_time
            << " s_end_time="<< stru_statistics_tcp.s_end_time);
}

INT ClientPack::PackHead(WORD package_type, BYTE *buffer)
{
    STRU_GENERAL_HEAD_PACKAGE head_package;
    head_package.mwPackType = package_type;
    return head_package.Pack(buffer, head_package.GetHeadLen());
}

INT ClientPack::PackBack2CometHead(WORD package_type, const WORD &user_count, const USERID *user_id_list, BYTE *buffer)
{
#ifdef _DEBUG
	std::stringstream userids;
    for(int i = 0; i < user_count; ++i)
    {
		userids  << user_id_list[i] << ","; 
    }
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__ , "type=" << package_type << " usercnt=" << user_count << " userids" << userids.str());
#endif

    STRU_BACK_2_COMET_HEAD_PACKAGE head_package;
    head_package.mwPackType = package_type;
    head_package.miUserCount = user_count;
    memcpy(head_package.moUserList, user_id_list, user_count * sizeof(USERID));
    return head_package.Pack(buffer, head_package.GetHeadLen());
}