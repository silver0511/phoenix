#ifndef QBIM_SERVERPACK_H
#define QBIM_SERVERPACK_H

#include <map>
#include <vector>
#include <struct/SvrBaseStruct.h>
#include "base0/platform.h"
#include "common/CriticalSection.h"
#include "network/ITCPPoolTransMgr.h"
#include "network/packet/BasePackDef.h"
#include "common/ServerTypeEnum.h"

//通用包头
#define GENERAL_SERVER_UNPACK_HEAD(packagetype) \
    packagetype loHeadSvrPacket; \
    INT liUnPackLen = loHeadSvrPacket.UnPack(apData, loHeadSvrPacket.GetHeadLen()); \
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " unpack svr 2 comet head len: " << liUnPackLen); \
    if(liUnPackLen < 0) \
    { \
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, " unpack comet head fault"); \
    	return 0; \
    } \
    apData += liUnPackLen; \
    awLen -= liUnPackLen; \

#define SERVER_UNPACK_BACK_2_COMET_HEAD() GENERAL_SERVER_UNPACK_HEAD(STRU_BACK_2_COMET_HEAD_PACKAGE)



class ServerPack
{
private:
    ServerPack();
    ServerPack(const ServerPack &);
    ServerPack & operator = (const ServerPack &);
    
public:
    ~ServerPack();
public:
    static ServerPack & GetInstance()
    {
        static ServerPack instance;
        return instance;
    }
public:
    INT DisplayStatisticsInfo();
public:
    bool OpenServerPoolByConfig(ITCPConnPoolMgrCallback *interface, STRU_TCP_POOL_OPT &tcp_pool_opt, EnumServerLogicType server_type);
    void AddRouter(WORD package_type, EnumServerLogicType server_type);

    template <class T>
    INT PackComet2Logic(WORD package_type, const SESSIONID& comet_id, const USERID& user_id, T &send_struct, BYTE *buffer)
    {
        INT head_len = PackComet2LogicHead(package_type, comet_id, user_id, buffer);
        buffer += head_len;
        INT body_len = PackBody(send_struct, buffer);
        return head_len + body_len;
    }

    template <class T>
    INT PackComet2Route(WORD package_type, const SESSIONID& comet_id, const USERID& user_id, T &send_struct, BYTE *buffer)
    {
        INT head_len = PackComet2RouteHead(package_type, comet_id, user_id, buffer);
        buffer += head_len;
        INT body_len = PackBody(send_struct, buffer);
        return head_len + body_len;
    }

    template <class T>
    INT Send2AllLogicServer(WORD package_type,  T &send_struct, const SESSIONID& comet_id, const USERID& user_id)
    {
        const EnumServerLogicType &server_type = package_to_router_map[package_type];
        const std::vector<ITCPConnPoolMgr*> &list_logic_server_pool = mMapServerPooMgr[server_type];
        if(list_logic_server_pool.size() <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " server_type = " << server_type << " is invalid");
            return -1;
        }

        BYTE send_buffer[DEF_MAX_TCP_PACKET_LEN];
        INT pack_len  = PackComet2Logic(package_type, comet_id, user_id, send_struct, send_buffer);

        if(pack_len <= 0)
        {
            LOG_TRACE(LOG_ERR, TRUE, __FUNCTION__, "invalid pack_len: " << pack_len
                                                                        << "pack_type" << package_type);
            return -1;
        }

        STRU_SENDDATA_TCP send_data;
        send_data.mhSocket = 0;
        send_data.mpData = send_buffer;
        send_data.mwLen = pack_len;
        send_data.mbNeedClose = FALSE;

        for(int index = 0; index < list_logic_server_pool.size(); ++index)
        {
            list_logic_server_pool[index]->SendDataBySession(user_id, &send_data);
        }

        return 1;
    }

    template <class T>
    INT Send2LogicServer(WORD package_type,  T &send_struct, const SESSIONID& comet_id, const USERID& user_id)
    {
        const EnumServerLogicType &server_type = package_to_router_map[package_type];
        const std::vector<ITCPConnPoolMgr*> &list_logic_server_pool = mMapServerPooMgr[server_type];
        if(list_logic_server_pool.size() <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " server_type = " << server_type << " is invalid");
            return -1;
        }

        BYTE send_buffer[DEF_MAX_TCP_PACKET_LEN];
        INT pack_len  = PackComet2Logic(package_type, comet_id, user_id, send_struct, send_buffer);

        if(pack_len <= 0)
        {
            LOG_TRACE(LOG_ERR, TRUE, __FUNCTION__, "invalid pack_len: " << pack_len
                                                                  << "pack_type" << package_type);
            return -1;
        }

        STRU_SENDDATA_TCP send_data;
        send_data.mhSocket = 0;
        send_data.mpData = send_buffer;
        send_data.mwLen = pack_len;
        send_data.mbNeedClose = FALSE;

        INT index = user_id %  list_logic_server_pool.size();
        return list_logic_server_pool[index]->SendDataBySession(user_id, &send_data);
    }

    template <class T>
    INT Send2RouteServer(WORD package_type, EnumServerLogicType server_type, T &send_struct, const SESSIONID& comet_id, const USERID& user_id)
    {
        const std::vector<ITCPConnPoolMgr*> &list_logic_server_pool = mMapServerPooMgr[server_type];
        if(list_logic_server_pool.size() <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " server_type = " << server_type << " is invalid");
            return -1;
        }

        BYTE send_buffer[DEF_MAX_TCP_PACKET_LEN];
        INT pack_len  = PackComet2Route(package_type, comet_id, user_id, send_struct, send_buffer);

        if(pack_len <= 0)
        {
            LOG_TRACE(LOG_ERR, TRUE, __FUNCTION__, "invalid pack_len: " << pack_len
                                                                        << "pack_type" << package_type);
            return -1;
        }

        STRU_SENDDATA_TCP send_data;
        send_data.mhSocket = 0;
        send_data.mpData = send_buffer;
        send_data.mwLen = pack_len;
        send_data.mbNeedClose = FALSE;

        for(int index = 0; index < list_logic_server_pool.size(); ++index)
        {
            list_logic_server_pool[index]->SendDataBySession(user_id, &send_data);
        }
        return 1;
    }
	
    void clear();
public:
    INT PackComet2LogicHead(WORD package_type, const SESSIONID& comet_id, const USERID& user_id, BYTE *buffer);
    INT PackComet2RouteHead(WORD package_type, const SESSIONID& comet_id, const USERID& user_id, BYTE *buffer);
    template <class T>
    INT PackBody(T &send_struct, BYTE *buffer)
    {
        STRU_GENERAL_BODY_PACKAGE_P<T> body_package_p;
        body_package_p.mpoUserLoad = &send_struct;
        return body_package_p.Pack(buffer, DEF_MAX_TCP_PACKET_LEN);
    }

    INT PackKafkaHead(WORD package_type, const WORD &user_count, const USERID *user_id_list, BYTE *buffer);

    template <class T>
    INT GetPackKafka(const WORD &package_type, const WORD &user_count, const USERID *user_id_list, T &send_struct, BYTE* buffer)
    {
        INT head_len = PackKafkaHead(package_type, user_count, user_id_list, buffer);
        buffer += head_len;

        INT body_len = PackBody(send_struct, buffer);

        LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " head len = " << head_len << " body len = " << body_len);
        return head_len + body_len;
    }

    INT PackScKafkaHead(WORD package_type, const USERID& user_id, BYTE *buffer);
    template <class T>
    INT GetPackScKafkaByUserId(const WORD &package_type, const USERID& user_id, T &send_struct, BYTE* buffer)
    {
        INT head_len = PackScKafkaHead(package_type, user_id, buffer);
        buffer += head_len;

        INT body_len = PackBody(send_struct, buffer);

        LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " head len = " << head_len << " body len = " << body_len);
        return head_len + body_len;
    }

    template<class T>
    INT SendAckToBackServer(SOCKET ahSocket, T &aoSendStruct, const SESSIONID& comet_id, IPTYPE aiIP,WORD awPort)
    {
        ITCPConnPoolMgr* pTcpConnPoolMgr = GetSendTcpConnPool(aiIP, awPort);
        if(NULL == pTcpConnPoolMgr)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " failed_aiIP = " << aiIP << "awPort" << awPort << " is failed");
            return -1;
        }

        BYTE send_buffer[DEF_MAX_TCP_PACKET_LEN];
        INT head_len = PackComet2LogicHead(T::ENUM_TYPE, comet_id, 0, send_buffer);

        STRU_SENDDATA_TCP send_data;
        send_data.mhSocket = 0;
        send_data.mpData = send_buffer;
        send_data.mwLen = head_len;
        send_data.mbNeedClose = FALSE;

        LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " send_ack_len = " << head_len << " comet_id = " << comet_id);
        return pTcpConnPoolMgr->SendDataBySocket(ahSocket, &send_data);
    }

    ITCPConnPoolMgr* GetSendTcpConnPool(IPTYPE aiIP, WORD awPort)
    {
        std::map<EnumServerLogicType, std::vector<ITCPConnPoolMgr*> >::iterator map_iter;
        for(map_iter = mMapServerPooMgr.begin(); map_iter != mMapServerPooMgr.end(); ++map_iter)
        {
            std::vector<ITCPConnPoolMgr*> list_logic_server_pool = map_iter->second;
            for(int index = 0; index < list_logic_server_pool.size(); index++)
            {
                bool isServerUser =  list_logic_server_pool[index]->JudgeIsConnectServer(aiIP, awPort);
                if(!!isServerUser)
                {
                    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " server find success enum_type = " << map_iter->first);
                    return list_logic_server_pool[index];
                }
            }
        }

        return NULL;
    }
    
private:
    std::map<WORD, EnumServerLogicType>    package_to_router_map;

    std::map<EnumServerLogicType, std::vector<ITCPConnPoolMgr*> > mMapServerPooMgr;

    std::vector<EnumServerLogicType> server_type_vec;
    int server_type_index;
    int server_statis_count;
    STRU_STATISTICS_TCP m_stru_info;
};

#endif //QBIM_SERVERPACK_H
