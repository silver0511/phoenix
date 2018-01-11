#ifndef __SCSERVERPACKDEF_H
#define __SCSERVERPACKDEF_H

#include "./CommonPackDef.h"

#include "network/packet/PackTypeDef.h"
#include "network/packet/BasePackDef.h"
#include "Package/hfbs/sc/fb_client_send_message_rq_generated.h"
#include "Package/hfbs/sc/fb_client_send_message_rs_generated.h"
#include "Package/hfbs/sc/fb_server_send_message_rq_generated.h"
#include "Package/hfbs/sc/fb_server_send_message_rs_generated.h"
#include "Package/hfbs/sc/fb_offline_msg_generated.h"
#include "Package/hfbs/sc/fb_client_get_offline_message_rq_generated.h"
#include "Package/hfbs/sc/fb_client_get_offline_message_rs_generated.h"
#include "Package/hfbs/sc/fb_get_user_status_rq_generated.h"
#include "Package/hfbs/sc/fb_get_user_status_rs_generated.h"
#include "Package/hfbs/sc/fb_single_chat_status_rq_generated.h"
#include "Package/hfbs/sc/fb_single_chat_status_rs_generated.h"
#include "Package/hfbs/sc/fb_get_msg_unread_rq_generated.h"
#include "Package/hfbs/sc/fb_get_msg_unread_rs_generated.h"

using namespace scpack;

struct STRU_CHAT_CLIENT_SEND_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CHAT_CLIENT_SEND_MESSAGE_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_CHAT_CLIENT_SEND_MESSAGE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_CHAT_CLIENT_SEND_MESSAGE_RQBuffer(verifier);
        if(!is_fbs)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " is not fbs");
            return -2;
        }

        t_fb_data = (T_CHAT_CLIENT_SEND_MESSAGE_RQ*)GetT_CHAT_CLIENT_SEND_MESSAGE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " check failed ");
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
    
    INT Check()
    {
        int result = STRU_PACKAGE_RQ::Check();
        if(result < 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "base check failed result = " << result);
            return result;
        }

        if(!t_fb_data->s_msg())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "s_msg is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->msg_content())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->msg_content()->c_str())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
            return -1;
        }

        string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
        if(msg_content.empty())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->send_user_name())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->send_user_name()->c_str())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name is nil");
            return -1;
        }

        string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
        if(send_user_name.empty())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name is nil");
            return -1;
        }

        if(!t_fb_data->op_user_id())
        {
            return -1;
        }

        return 1;
    }
};

struct STRU_CHAT_CLIENT_SEND_MESSAGE_RS : public STRU_PACKAGE_RS<T_CHAT_CLIENT_SEND_MESSAGE_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_CHAT_CLIENT_SEND_MESSAGE_RS);

    STRU_CHAT_CLIENT_SEND_MESSAGE_RS() : STRU_PACKAGE_RS<T_CHAT_CLIENT_SEND_MESSAGE_RS>(NEW_DEF_CHAT_CLIENT_SEND_MESSAGE_RS)
    {

    }
};

struct STRU_CHAT_SERVER_SEND_MESSAGE_RS : public STRU_SERVER_PACKAGE_RS<T_CHAT_SERVER_SEND_MESSAGE_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_CHAT_SERVER_SEND_MESSAGE_RS);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_CHAT_SERVER_SEND_MESSAGE_RSBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_CHAT_SERVER_SEND_MESSAGE_RS*)GetT_CHAT_SERVER_SEND_MESSAGE_RS(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
    
    INT Check()
    {   
        int result = STRU_SERVER_PACKAGE_RS::Check();
        if(result < 0)
        {
            return -1;
        }

        if(!t_fb_data->s_msg())
        {
            return -1;
        }

        if(!t_fb_data->s_msg()->msg_content())
        {
            return -1;
        }

        if(!t_fb_data->op_user_id())
        {
            return -1;
        }

        return 1;
    }
};

struct STRU_T_OFFLINE_MSG
{
    public:
    T_OFFLINE_MSG *t_offline_msg;

    STRU_T_OFFLINE_MSG()
    {
        t_offline_msg = NULL;
    };

    INT Serialize(ISerialize & aoSerialize)
    {
        t_offline_msg = (T_OFFLINE_MSG*)GetT_OFFLINE_MSG(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
    
    INT Check()
    {
        if(!t_offline_msg)
        {
            return -1;
        }

        if(!t_offline_msg->op_user_id())
        {
            return -1;
        }
        if(!t_offline_msg->message_id())
        {
            return -1;
        }

        if(!t_offline_msg->s_msg())
        {
            return -1;
        }

        return 1;
    }
};

struct STRU_GET_USER_STATUS_RQ : public STRU_PACKAGE_RQ<T_GET_USER_STATUS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GET_USER_STATUS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GET_USER_STATUS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GET_USER_STATUS_RQ*)GetT_GET_USER_STATUS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GET_USER_STATUS_RS : public STRU_PACKAGE_RS<T_GET_USER_STATUS_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_GET_USER_STATUS_RS);

    STRU_GET_USER_STATUS_RS() : STRU_PACKAGE_RS<T_GET_USER_STATUS_RS>(NEW_DEF_GET_USER_STATUS_RS)
    {
        
    }
};

struct STRU_SINGLE_CHAT_STATUS_RQ : public STRU_PACKAGE_RQ<T_SINGLE_CHAT_STATUS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_SINGLE_CHAT_STATUS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_SINGLE_CHAT_STATUS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_SINGLE_CHAT_STATUS_RQ*)GetT_SINGLE_CHAT_STATUS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_SINGLE_CHAT_STATUS_RS : public STRU_PACKAGE_RS<T_SINGLE_CHAT_STATUS_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_SINGLE_CHAT_STATUS_RS);

    STRU_SINGLE_CHAT_STATUS_RS() : STRU_PACKAGE_RS<T_SINGLE_CHAT_STATUS_RS>(NEW_DEF_SINGLE_CHAT_STATUS_RS)
    {
        
    }
};

struct STRU_CHAT_GET_OFFLINE_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CHAT_GET_OFFLINE_MESSAGE_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_CHAT_GET_OFFLINE_MESSAGE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_CHAT_GET_OFFLINE_MESSAGE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_CHAT_GET_OFFLINE_MESSAGE_RQ*)GetT_CHAT_GET_OFFLINE_MESSAGE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_CHAT_GET_OFFLINE_MESSAGE_RS : public STRU_PACKAGE_RS<T_CHAT_GET_OFFLINE_MESSAGE_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_CHAT_GET_OFFLINE_MESSAGE_RS);

    STRU_CHAT_GET_OFFLINE_MESSAGE_RS() : STRU_PACKAGE_RS<T_CHAT_GET_OFFLINE_MESSAGE_RS>(NEW_DEF_CHAT_GET_OFFLINE_MESSAGE_RS)
    {
        
    }
};


struct STRU_GET_MSG_UNREAD_RQ : public STRU_PACKAGE_RQ<T_GET_MSG_UNREAD_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GET_MSG_UNREAD_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GET_MSG_UNREAD_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GET_MSG_UNREAD_RQ*)GetT_GET_MSG_UNREAD_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_GET_MSG_UNREAD_RS : public STRU_PACKAGE_RS<T_GET_MSG_UNREAD_RS>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_GET_MSG_UNREAD_RS);

    STRU_GET_MSG_UNREAD_RS() : STRU_PACKAGE_RS<T_GET_MSG_UNREAD_RS>(NEW_DEF_GET_MSG_UNREAD_RS)
    {

    }
};
#endif //__SCSERVERPACKDEF_H
