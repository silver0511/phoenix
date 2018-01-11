#ifndef __GC_MODEL_DEF_H
#define __GC_MODEL_DEF_H
#include "common/System.h"

class GroupDetailInfo 
{
public:
    static char* GetGroupManagerUserId()
    {
        return "group_manager_user_id";
    };

    static char* GetGroupImageUrl()
    {
        return "group_img_url";
    }

    static char* GetGroupName()
    {
        return "group_name";
    }

    static char* GetGroupAddIsAgree()
    {
        return "group_add_is_agree";
    }

    static char* GetGroupRemark()
    {
        return "group_remark";
    }

    static char* GetGroupId()
    {
        return "group_id";
    }

    static char* GetGroupCT()
    {
        return "group_ct";
    }

    static char* GetGroupCount()
    {
        return "group_count";
    }

    static char* GetIsMember()
    {
        return "isMember";
    }

    static char* GetGroupList()
    {
        return "group_list";
    }

    static char* GetGroupListIndex()
    {
        return "group_list_index";
    }

    static char* GetGroupListPageNumber()
    {
        return "group_list_page_number";
    }
};

class CGroupUserBaseInfo
{
public:
    static char* GetUserId()
    {
        return "user_id";
    }

    static char* GetUserNickName()
    {
        return "user_nick_name";
    }

    static char* GetUserGroupIndex()
    {
        return "user_group_index";
    }

    static char* GetGroupMemberIndex()
    {
        return "group_member_index";
    }

    static char* GetGroupMemberPageNumber()
    {
        return "group_page_span";
    }

    static char* GetMemberList()
    {
        return "memberList";
    }
};

class CGroupModifyMember
{
public:
    static char* GetOpUserID()
    {
        return "op_user_id";
    }

    static char* GetBigMsgType()
    {
        return "big_msg_type";
    }

    static char* GetGroupId()
    {
        return "group_id";
    }

    static char* GetAddUserList()
    {
        return "list_add_user";
    }

    static char* GetChangeUserList()
    {
        return "list_user_change";
    }

    static char* GetOperateUserName()
    {
        return "operate_user_name";
    }

    static char* GetMsgTime()
    {
        return "msg_time";
    }

    static char* GetMessageId()
    {
        return "message_id";
    }

    static char* GetGroupManagerUserId()
    {
        return "group_manager_user_id";
    }

    static char* GetGroupCount()
    {
        return "group_count";
    }

    static char* GetGroupMaxCount()
    {
        return "group_max_count";
    }

    static char* GetMessageOldId()
    {
        return "message_old_id";
    }

    static char* GetGroupAddIsAgree()
    {
        return "group_add_is_agree";
    }

    static char* GetGroupCT()
    {
        return "group_ct";
    }

    static char* GetGroupModifyContent()
    {
        return "group_modify_content";
    }
};


class CGroupChatMessage
{
public:
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