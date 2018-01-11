#ifndef __NETADDR_DEF_H
#define __NETADDR_DEF_H

const std::string DBSERVERADDR = "http://imdb.qbao.com";
const std::string ORDERSERVERADDR = "http://oc.qbao.com";

//--------------friend interface begin ----------
const std::string FRIEND_SETBLACLIST_API = DBSERVERADDR + "/api/friend/settingBlacklist";
const std::string FRIEND_ADD_API      =   DBSERVERADDR + "/api/friend/save";
const std::string FRIEND_AGREE_API    =   DBSERVERADDR + "/api/friend/agree";
const std::string FRIEND_DEL_API      =   DBSERVERADDR + "/api/friend/del";
const std::string FRIEND_GETLIST_API  =   DBSERVERADDR + "/api/friend/get";
const std::string FRIEND_REMARK_API     =   DBSERVERADDR + "/api/friend/update";
const std::string FRIEND_UPDATESTATUS_API =  DBSERVERADDR + "/api/friend/update/status";
//--------------friend interface end -----------

//--------------business interface begin ---------
const std::string EC_RECENTORDER_API = ORDERSERVERADDR + "/api/recently/orderInfo.html";
const std::string EC_LASTMSG_API = DBSERVERADDR + "/api/message/merchant/currday";
const std::string EC_RECENTMSG_API = DBSERVERADDR + "/api/message/merchant";
//--------------business interface end ----------


//--------------group interface begin-----------------
const std::string GROUP_CREATE_RQ_API           = DBSERVERADDR + "/api/group/create";
const std::string GROUP_DETAIL_RQ_API           = DBSERVERADDR + "/api/group/memberList";
const std::string GROUP_LEADER_CHANGE_RQ_API    = DBSERVERADDR + "/api/group/transfer";
const std::string GROUP_GET_INFO_RQ_API           = DBSERVERADDR + "/api/group/info";
const std::string GROUP_LIST_RQ_API           = DBSERVERADDR + "/api/group/list";
const std::string GROUP_LIST_IDS_RQ_API           = DBSERVERADDR + "/api/group/list/getGroupId";
const std::string GROUP_ADD_USER_RQ_API           = DBSERVERADDR + "/api/group/oper/add";
const std::string GROUP_KICK_USER_RQ_API           = DBSERVERADDR + "/api/group/oper/kick";
const std::string GROUP_MODIFY_AGREE_RQ_API           = DBSERVERADDR + "/api/group/modify/agree";
const std::string GROUP_MODIFY_DEFAULT_RQ_API           = DBSERVERADDR + "/api/group/modify/default";
const std::string GROUP_MODIFY_GROUP_NAME_RQ_API           = DBSERVERADDR + "/api/group/modify/m_name";
const std::string GROUP_MODIFY_NICK_NAME_RQ_API           = DBSERVERADDR + "/api/group/modify/m_nickname";
const std::string GROUP_MODIFY_REMARK_RQ_API           = DBSERVERADDR + "/api/group/modify/m_remark";
const std::string GROUP_REMARK_DETAIL_RQ_API           = DBSERVERADDR + "/api/group/query/remark";
const std::string GROUP_RECV_TYPE_RQ_API           = DBSERVERADDR + "/api/group/modify/rcvType";
const std::string GROUP_GET_USER_REMARK_NAME_RQ_API           = DBSERVERADDR + "/api/group/user/remark";
const std::string GROUP_SAVE_CHANGE_RQ_API           = DBSERVERADDR + "/api/group/modify/isAddressBook";
const std::string GROUP_GET_BATCH_INFO_API          = DBSERVERADDR + "/api/group/info/list";
//--------------group interface end-------------------


//--------------tim sms interface begin-----------------
const std::string TT_SMS_VERIFY_API = "https://yun.tim.qq.com/v5/tlssmssvr/sendsms";
//--------------tim sms interface begin-----------------
#endif
