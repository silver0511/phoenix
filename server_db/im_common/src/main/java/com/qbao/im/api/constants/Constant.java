package com.qbao.im.api.constants;

/**
 * Created by tangxiaojun on 2017/5/25.
 */
public abstract class Constant {
    public static final String REIDS_GROUP_MEMBER_KEY = "is_address_book";

    //加人
    public static final String GROUP_OFFLINE_CHAT_ADD_USER = "add";

    //踢人
    public static final String GROUP_OFFLINE_CHAT_KICK_USER = "kick";

    //群主同意
    public static final String GROUP_OFFLINE_CHAT_ENTER_AGREE = "agree";

    //默认进群
    public static final String GROUP_OFFLINE_CHAT_ENTER_DEFAULT = "default";

    //修改群名称
    public static final String GROUP_OFFLINE_CHAT_MODIFY_GROUP_NAME = "m_name";

    //修改群备注
    public static final String GROUP_OFFLINE_CHAT_MODIFY_GROUP_REMARK = "m_remark";

    //修改用户昵称
    public static final String GROUP_OFFLINE_CHAT_MODIFY_GROUP_USER_NAME = "m_nickname";

    public static final String REDIS_CREATE_GROUP_KEY = "groupKey";

    public static final String REDIS_CREATE_GROUP_FIELD_KEY = "key";

    public static final String REDIS_GROUP_MEMBER = "nim_group_";

    public static final String REDIS_USER_GROUP_LIST = "nim_user_group_list_";

    public static final String REDIS_GROUP_INFO = "nim_group_info_";

    public static final String REDIS_GROUP_MEMBER_PROPERTY = "nim_group_member_attr";

    //好友最大人数
    public static final int FRIEND_MAX_NUM = 2000;

    //群名称字数限制
    public static final int GROUP_NAME_LEN = 32;

    //群成员昵称字数限制
    public static final int GROUP_NICK_NAME_LEN = 24;

    //群ID基数
    public static final int GROUP_INCREBY_ID = 10000000;

    //群DI自增大小
    public static final int GROUP_AUTOMENT_ID = 1;
    
    //redis中的空值
    public static final String REDIS_NULL_VALUE = "_NULL_";
    
    //redis中存储用户群操作信息
    public static final String REDIS_GROUP_OPER_KEY_PREFIX = "nim_group_oper";
}
