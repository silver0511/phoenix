#ifndef __SC_MODEL_DEF_H
#define __SC_MODEL_DEF_H
#include "common/System.h"

class CScChatMessage
{
public:
    static char* GetOpUserId()
    {
        return "op_user_id";
    }

    static char* GetBID()
    {
        return "b_id";
    }

    static char* GetCID()
    {
        return "c_id";
    }
    
    static char* GetWID()
    {
        return "w_id";
    }

    static char* GetGroupId()
    {
        return "group_id";
    }

    static char* GetAppID()
    {
        return "app_id";
    }

    static char* GetSessionID()
    {
        return "session_id";
    }

    static char* GetChatType()
    {
        return "chat_type";
    }

    static char* GetMType()
    {
        return "m_type";
    }

    static char* GetSType()
    {
        return "s_type";
    }

    static char* GetExtType()
    {
        return "ext_type";
    }

    static char* GetMsgContent()
    {
        return "msg_content";
    }

    static char* GetMsgTime()
    {
        return "msg_time";
    }

    static char* GetSendUserName()
    {
        return "send_user_name";
    }

    static char* GetMessageID()
    {
        return "message_id";
    }
};


#endif