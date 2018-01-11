#ifndef __GCSERVERPACKDEF_H
#define __GCSERVERPACKDEF_H
#include "./CommonPackDef.h"

#include "Package/hfbs/group/fb_group_offline_msg_generated.h"
#include "Package/hfbs/group/fb_group_list_rq_generated.h"
#include "Package/hfbs/group/fb_group_list_rs_generated.h"
#include "Package/hfbs/group/fb_group_client_send_message_rq_generated.h"
#include "Package/hfbs/group/fb_group_client_send_message_rs_generated.h"
#include "Package/hfbs/group/fb_group_get_offline_msg_rq_generated.h"
#include "Package/hfbs/group/fb_group_get_offline_msg_rs_generated.h"
#include "Package/hfbs/group/fb_group_create_rq_generated.h"
#include "Package/hfbs/group/fb_group_create_rs_generated.h"
#include "Package/hfbs/group/fb_group_detail_info_rq_generated.h"
#include "Package/hfbs/group/fb_group_detail_info_rs_generated.h"
#include "Package/hfbs/group/fb_group_modify_change_rq_generated.h"
#include "Package/hfbs/group/fb_group_modify_change_rs_generated.h"
#include "Package/hfbs/group/fb_group_offline_msg_generated.h"
#include "Package/hfbs/group/fb_group_leader_change_rq_generated.h"
#include "Package/hfbs/group/fb_group_leader_change_rs_generated.h"
#include "Package/hfbs/group/fb_group_chat_notify_rq_generated.h"
#include "Package/hfbs/group/fb_group_remark_detail_rq_generated.h"
#include "Package/hfbs/group/fb_group_remark_detail_rs_generated.h"
#include "Package/hfbs/group/fb_group_list_ids_rq_generated.h"
#include "Package/hfbs/group/fb_group_list_ids_rs_generated.h"
#include "Package/hfbs/group/fb_group_message_status_rq_generated.h"
#include "Package/hfbs/group/fb_group_message_status_rs_generated.h"
#include "Package/hfbs/group/fb_group_type_list_rq_generated.h"
#include "Package/hfbs/group/fb_group_type_list_rs_generated.h"
#include "Package/hfbs/group/fb_get_user_remark_name_rq_generated.h"
#include "Package/hfbs/group/fb_get_user_remark_name_rs_generated.h"
#include "Package/hfbs/group/fb_group_save_change_rq_generated.h"
#include "Package/hfbs/group/fb_group_save_change_rs_generated.h"
#include "Package/hfbs/group/fb_group_scan_rq_generated.h"
#include "Package/hfbs/group/fb_group_scan_rs_generated.h"
#include "Package/hfbs/group/fb_get_batch_group_info_rq_generated.h"
#include "Package/hfbs/group/fb_get_batch_group_info_rs_generated.h"

using namespace grouppack;

struct STRU_GROUP_LIST_RQ : public STRU_PACKAGE_RQ<T_GROUP_LIST_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_LIST_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_LIST_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_LIST_RQ*)GetT_GROUP_LIST_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_LIST_RS : public STRU_PACKAGE_RS<T_GROUP_LIST_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GROUP_LIST_RS);

    STRU_GROUP_LIST_RS() : STRU_PACKAGE_RS<T_GROUP_LIST_RS>(NEW_DEF_GROUP_LIST_RS)
    {

    }
};

struct STRU_GROUP_REMARK_DETAIL_RQ : public STRU_PACKAGE_RQ<T_GROUP_REMARK_DETAIL_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_REMARK_DETAIL_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_REMARK_DETAIL_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_REMARK_DETAIL_RQ*)GetT_GROUP_REMARK_DETAIL_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_REMARK_DETAIL_RS : public STRU_PACKAGE_RS<T_GROUP_REMARK_DETAIL_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GROUP_REMARK_DETAIL_RS);

    STRU_GROUP_REMARK_DETAIL_RS() : STRU_PACKAGE_RS<T_GROUP_REMARK_DETAIL_RS>(NEW_DEF_GROUP_REMARK_DETAIL_RS)
    {

    }
};


struct STRU_GROUP_LIST_IDS_RQ : public STRU_PACKAGE_RQ<T_GROUP_LIST_IDS_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_LIST_IDS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_LIST_IDS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_LIST_IDS_RQ*)GetT_GROUP_LIST_IDS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_LIST_IDS_RS : public STRU_PACKAGE_RS<T_GROUP_LIST_IDS_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GROUP_LIST_IDS_RS);

    STRU_GROUP_LIST_IDS_RS() : STRU_PACKAGE_RS<T_GROUP_LIST_IDS_RS>(NEW_DEF_GROUP_LIST_IDS_RS)
    {

    }
};

struct STRU_GROUP_CLIENT_SEND_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_GROUP_CLIENT_SEND_MESSAGE_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_CLIENT_SEND_MESSAGE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_CLIENT_SEND_MESSAGE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_CLIENT_SEND_MESSAGE_RQ*)GetT_GROUP_CLIENT_SEND_MESSAGE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
    
    INT Check()
    {
        int result = STRU_PACKAGE_RQ<T_GROUP_CLIENT_SEND_MESSAGE_RQ>::Check();
        if(result <= 0)
        {
            return result;
        }

        if(!t_fb_data->s_msg())
        {
            return -1;
        }

        if(!t_fb_data->s_msg()->msg_content())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->msg_content()->c_str())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content str is nil");
            return -1;
        }

        std::string str_content = t_fb_data->s_msg()->msg_content()->c_str();
        if(str_content.empty())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content str is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->send_user_name())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->send_user_name()->c_str())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name str is nil");
            return -1;
        }

        string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
        if(send_user_name.empty())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name str is nil");
            return -1;
        }

        if(!t_fb_data->group_id())
        {
            return -1;
        }

        if(!t_fb_data->message_id())
        {
            return -1;
        }

        if(!t_fb_data->group_name() || !t_fb_data->group_name()->c_str())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " group_name str is nil");
            return -1;
        }

        string group_name = t_fb_data->group_name()->c_str();
        if(group_name.empty())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " group_name str is nil");
            return -1;
        }

        return 1;
    }
};

struct STRU_GROUP_CLIENT_SEND_MESSAGE_RS : public STRU_PACKAGE_RS<T_GROUP_CLIENT_SEND_MESSAGE_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GROUP_CLIENT_SEND_MESSAGE_RS);

    STRU_GROUP_CLIENT_SEND_MESSAGE_RS() : STRU_PACKAGE_RS<T_GROUP_CLIENT_SEND_MESSAGE_RS>(NEW_DEF_GROUP_CLIENT_SEND_MESSAGE_RS)
    {

    }
};

struct STRU_GROUP_GET_OFFLINE_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_GROUP_GET_OFFLINE_MESSAGE_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GROUP_GET_OFFLINE_MESSAGE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_GET_OFFLINE_MESSAGE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_GET_OFFLINE_MESSAGE_RQ*)GetT_GROUP_GET_OFFLINE_MESSAGE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_GET_OFFLINE_MESSAGE_RS : public STRU_PACKAGE_RS<T_GROUP_GET_OFFLINE_MESSAGE_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GROUP_GET_OFFLINE_MESSAGE_RS);

    STRU_GROUP_GET_OFFLINE_MESSAGE_RS() : STRU_PACKAGE_RS<T_GROUP_GET_OFFLINE_MESSAGE_RS>(NEW_DEF_GROUP_GET_OFFLINE_MESSAGE_RS)
    {

    }
};


struct STRU_GROUP_CREATE_RQ : public STRU_PACKAGE_RQ<T_GROUP_CREATE_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_CREATE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_CREATE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_CREATE_RQ*)GetT_GROUP_CREATE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "check failed");
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }

    INT Check()
    {
        int result = STRU_PACKAGE_RQ<T_GROUP_CREATE_RQ>::Check();
        if(result <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " base failed");
            return result;
        }

        if(!t_fb_data->group_name())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "group name is nil");
            return -1;
        }

        if(!t_fb_data->group_name()->c_str())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "group name str is nil");
            return -1;
        }

        string group_name = t_fb_data->group_name()->c_str();
        if(group_name.empty())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "group name str is nil");
            return -1;
        }

        return 1;
    }
};

struct STRU_GROUP_CREATE_RS : public STRU_PACKAGE_RS<T_GROUP_CREATE_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GROUP_CREATE_RS);

    STRU_GROUP_CREATE_RS() : STRU_PACKAGE_RS<T_GROUP_CREATE_RS>(NEW_DEF_GROUP_CREATE_RS)
    {

    }
};

struct STRU_GROUP_DETAIL_INFO_RQ : public STRU_PACKAGE_RQ<T_GROUP_DETAIL_INFO_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_DETAIL_INFO_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_DETAIL_INFO_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_DETAIL_INFO_RQ*)GetT_GROUP_DETAIL_INFO_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_DETAIL_INFO_RS : public STRU_PACKAGE_RS<T_GROUP_DETAIL_INFO_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GROUP_DETAIL_INFO_RS);

    STRU_GROUP_DETAIL_INFO_RS() : STRU_PACKAGE_RS<T_GROUP_DETAIL_INFO_RS>(NEW_DEF_GROUP_DETAIL_INFO_RS)
    {

    }
};

struct STRU_GROUP_MODIFY_ChANGE_RQ : public STRU_PACKAGE_RQ<T_GROUP_MODIFY_ChANGE_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_MODIFY_CHANGE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_MODIFY_ChANGE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_MODIFY_ChANGE_RQ*)GetT_GROUP_MODIFY_ChANGE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_MODIFY_CHANGE_RS : public STRU_PACKAGE_RS<T_GROUP_MODIFY_ChANGE_RS>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_MODIFY_CHANGE_RS);

    STRU_GROUP_MODIFY_CHANGE_RS() : STRU_PACKAGE_RS<T_GROUP_MODIFY_ChANGE_RS>(NEW_DEF_GROUP_MODIFY_CHANGE_RS)
    {
        
    }
};

struct STRU_GROUP_LEADER_CHANGE_RQ : public STRU_PACKAGE_RQ<T_GROUP_LEADER_CHANGE_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_LEADER_CHANGE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_LEADER_CHANGE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_LEADER_CHANGE_RQ*)GetT_GROUP_LEADER_CHANGE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_LEADER_CHANGE_RS : public STRU_PACKAGE_RS<T_GROUP_LEADER_CHANGE_RS>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_LEADER_CHANGE_RS);

    STRU_GROUP_LEADER_CHANGE_RS() : STRU_PACKAGE_RS<T_GROUP_LEADER_CHANGE_RS>(NEW_DEF_GROUP_LEADER_CHANGE_RS)
    {
        
    }
};

//时间更新ID包[tcpcomet]
struct STRU_GROUP_UPDATE_LOGOUT_ID
{
public:
    uint64	miUpdateTime;
    MAPPING_PACKTYPE(NEW_DEF_GROUP_UPDATE_LOGOUT_ID);

    STRU_GROUP_UPDATE_LOGOUT_ID()
    {
        miUpdateTime = 0;
    }

    INT Serialize(ISerialize & aoSerialize)
    {
        aoSerialize.Serialize(miUpdateTime);
        return 1;
    }
};

struct STRU_GROUP_MESSAGE_STATUS_RS : public STRU_PACKAGE_RS<T_GROUP_MESSAGE_STATUS_RS>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_MESSAGE_STATUS_RS);

    STRU_GROUP_MESSAGE_STATUS_RS() : STRU_PACKAGE_RS<T_GROUP_MESSAGE_STATUS_RS>(NEW_DEF_GROUP_MESSAGE_STATUS_RS)
    {
        
    }
};


struct STRU_GROUP_MESSAGE_STATUS_RQ : public STRU_PACKAGE_RQ<T_GROUP_MESSAGE_STATUS_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_MESSAGE_STATUS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_MESSAGE_STATUS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_MESSAGE_STATUS_RQ*)GetT_GROUP_MESSAGE_STATUS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_NOTIFY_SERVER_RQ : public STRU_SERVER_PACKAGE_RQ<T_GROUP_CHAT_NOTIFY_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GROUP_CHAT_NOTIFY_SIMPLE_RQ);

    STRU_GROUP_NOTIFY_SERVER_RQ() : STRU_SERVER_PACKAGE_RQ<T_GROUP_CHAT_NOTIFY_RQ>(NEW_DEF_GROUP_CHAT_NOTIFY_SIMPLE_RQ)
    {

    }
};


struct STRU_GROUP_TYPE_LIST_RQ : public STRU_PACKAGE_RQ<T_GROUP_TYPE_LIST_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_TYPE_LIST_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_TYPE_LIST_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_TYPE_LIST_RQ*)GetT_GROUP_TYPE_LIST_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_TYPE_LIST_RS : public STRU_PACKAGE_RS<T_GROUP_TYPE_LIST_RS>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_TYPE_LIST_RS);

    STRU_GROUP_TYPE_LIST_RS() : STRU_PACKAGE_RS<T_GROUP_TYPE_LIST_RS>(NEW_DEF_GROUP_TYPE_LIST_RS)
    {
        
    }
};

struct STRU_GROUP_GET_USER_REMARK_NAME_RQ : public STRU_PACKAGE_RQ<T_GROUP_GET_USER_REMARK_NAME_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_GET_USER_REMARK_NAME_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_GET_USER_REMARK_NAME_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_GET_USER_REMARK_NAME_RQ*)GetT_GROUP_GET_USER_REMARK_NAME_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_GET_USER_REMARK_NAME_RS : public STRU_PACKAGE_RS<T_GROUP_GET_USER_REMARK_NAME_RS>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_GET_USER_REMARK_NAME_RS);

    STRU_GROUP_GET_USER_REMARK_NAME_RS() : STRU_PACKAGE_RS<T_GROUP_GET_USER_REMARK_NAME_RS>(NEW_DEF_GROUP_GET_USER_REMARK_NAME_RS)
    {
        
    }
};

struct STRU_GROUP_SAVE_CHANGE_RQ : public STRU_PACKAGE_RQ<T_GROUP_SAVE_CHANGE_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_SAVE_CHANGE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_SAVE_CHANGE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_SAVE_CHANGE_RQ*)GetT_GROUP_SAVE_CHANGE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_SAVE_CHANGE_RS : public STRU_PACKAGE_RS<T_GROUP_SAVE_CHANGE_RS>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_SAVE_CHANGE_RS);

    STRU_GROUP_SAVE_CHANGE_RS() : STRU_PACKAGE_RS<T_GROUP_SAVE_CHANGE_RS>(NEW_DEF_GROUP_SAVE_CHANGE_RS)
    {
        
    }
};

struct STRU_GROUP_SCAN_RQ : public STRU_PACKAGE_RQ<T_GROUP_SCAN_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_SCAN_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GROUP_SCAN_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GROUP_SCAN_RQ*)GetT_GROUP_SCAN_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GROUP_SCAN_RS : public STRU_PACKAGE_RS<T_GROUP_SCAN_RS>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_SCAN_RS);

    STRU_GROUP_SCAN_RS() : STRU_PACKAGE_RS<T_GROUP_SCAN_RS>(NEW_DEF_GROUP_SCAN_RS)
    {
        
    }
};

struct STRU_GET_BATCH_GROUP_INFO_RQ : public STRU_PACKAGE_RQ<T_GET_BATCH_GROUP_INFO_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_BATCH_INFO_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GET_BATCH_GROUP_INFO_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GET_BATCH_GROUP_INFO_RQ*)GetT_GET_BATCH_GROUP_INFO_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GET_BATCH_GROUP_INFO_RS : public STRU_PACKAGE_RS<T_GET_BATCH_GROUP_INFO_RS>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GROUP_BATCH_INFO_RS);

    STRU_GET_BATCH_GROUP_INFO_RS() : STRU_PACKAGE_RS<T_GET_BATCH_GROUP_INFO_RS>(NEW_DEF_GROUP_BATCH_INFO_RS)
    {
        
    }
};

#endif //__GCSERVERPACKDEF_H
