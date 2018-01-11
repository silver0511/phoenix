#include "stdafx.h"
#include "network/packet/ServerPack.h"


ServerPack::ServerPack()
{
    server_type_index = 0;
    server_statis_count = 0;
}

ServerPack::~ServerPack()
{
    clear();
}

void ServerPack::clear()
{
    std::map<EnumServerLogicType, std::vector<ITCPConnPoolMgr*> >::iterator map_iter;
    for(map_iter = mMapServerPooMgr.begin(); map_iter != mMapServerPooMgr.end(); ++map_iter)
    {
        std::vector<ITCPConnPoolMgr*> list_logic_server_pool = map_iter->second;
        for(int i = 0; i < list_logic_server_pool.size(); ++i)
        {
            if(list_logic_server_pool[i])
            {
                list_logic_server_pool[i]->Close();
                list_logic_server_pool[i]->Release();
                list_logic_server_pool[i] = NULL;
            }
        }

        list_logic_server_pool.clear();
    }

    mMapServerPooMgr.clear();
}


INT ServerPack::DisplayStatisticsInfo()
{
    if(server_statis_count < 60)
    {
        server_statis_count++;
        return 1;
    }

    if(server_type_vec.size() <= 0)
    {
        return 1;
    }

    server_statis_count = 0;

    server_type_index = server_type_index % server_type_vec.size();
    const std::vector<ITCPConnPoolMgr*> &list_logic_server_pool = mMapServerPooMgr[server_type_vec[server_type_index]];
    if(list_logic_server_pool.size() <= 0)
    {
        server_type_index++;
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, " server_type = " << server_type_vec[server_type_index] << " is invalid");
        return -1;
    }

    for(int index = 0; index < list_logic_server_pool.size(); ++index)
    {
        list_logic_server_pool[index]->GetNetStatisticsInfo(&m_stru_info);
        LOG_TRACE(LOG_CRIT, true, __FUNCTION__, "statis info "
                <<"server_type=" << server_type_vec[server_type_index] << "_" << index
                <<" t_send_count=" << m_stru_info.t_send_count << " t_recv_count="<< m_stru_info.t_recv_count
                <<" send_count_sec="<< m_stru_info.send_count_sec << " recv_count_sec="<< m_stru_info.recv_count_sec
                <<" cur_con="<< m_stru_info.cur_con << " t_lost_send="<< m_stru_info.t_lost_send
                <<" t_lost_recv="<< m_stru_info.t_lost_recv <<" s_start_time="<< m_stru_info.s_start_time
                << " s_end_time="<< m_stru_info.s_end_time);
    }
    server_type_index++;
    return 1;
}

bool ServerPack::OpenServerPoolByConfig(ITCPConnPoolMgrCallback *interface, STRU_TCP_POOL_OPT &tcp_pool_opt, EnumServerLogicType server_type)
{
    ITCPConnPoolMgr	*p_logic_pool;
    CreateNetTPool("NetTPool", (void**)&p_logic_pool);
    if(NULL == p_logic_pool)
    {
        LOG_TRACE(LOG_CRIT, false, __FUNCTION__,  "Create Error.");
        return false;
    }

    if(mMapServerPooMgr[server_type].size() <= 0)
    {
        mMapServerPooMgr[server_type].reserve(10);
        server_type_vec.push_back(server_type);
        LOG_TRACE(LOG_ALERT, false, __FUNCTION__, " init server length server_type = " << server_type);
    }

    mMapServerPooMgr[server_type].push_back(p_logic_pool);

    if(!p_logic_pool->Open(interface, tcp_pool_opt))
    {
        LOG_TRACE(LOG_CRIT,true,__FUNCTION__," Initialize logic tcp pool error.");
        return false;
    }

    return true;
}

INT ServerPack::PackComet2LogicHead(WORD package_type, const SESSIONID& comet_id, const USERID& user_id, BYTE *buffer)
{
    STRU_COMET_2_BACK_HEAD head_package;
    head_package.miCometID = comet_id;
    head_package.miUserID = user_id;
    head_package.mwPackType = package_type;

    return head_package.Pack(buffer, head_package.GetHeadLen());
}

INT ServerPack::PackComet2RouteHead(WORD package_type, const SESSIONID& comet_id, const USERID& user_id, BYTE *buffer)
{
    STRU_COMET_2_BACK_HEAD head_package;
    head_package.miCometID = comet_id;
    head_package.miUserID = user_id;
    head_package.mwPackType = package_type;

    return head_package.Pack(buffer, head_package.GetHeadLen());
}


INT ServerPack::PackKafkaHead(WORD package_type, const WORD &user_count, const USERID *user_id_list, BYTE *buffer)
{
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "enter enter");
    STRU_LOGIC_2_CONSUME_HEAD head_package;
    head_package.mwPackType = package_type;
    head_package.miUserCount = user_count;
    memcpy(head_package.moUserList, user_id_list, user_count * sizeof(USERID));
    return head_package.Pack(buffer, head_package.GetHeadLen());
}

INT ServerPack::PackScKafkaHead(WORD package_type, const USERID& user_id, BYTE *buffer)
{
    STRU_LOGIC_2_SC_CONSUMER_HEAD head_package;
    head_package.mwPackType = package_type;
    head_package.user_id = user_id;
    
    return head_package.Pack(buffer, head_package.GetHeadLen());
}

void ServerPack::AddRouter(WORD package_type, EnumServerLogicType server_type)
{
    package_to_router_map[package_type] = server_type;
}
