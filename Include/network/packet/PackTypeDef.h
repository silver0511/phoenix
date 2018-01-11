#ifndef __PACKTYPEDEF_H
#define __PACKTYPEDEF_H


//客户端包起始地址
#define NEW_DEF_DEFAULT_CLIENT_PACKET_BASE                  (0)
//系统级消息起始地址
#define NEW_DEF_SYS_BASE                                    (NEW_DEF_DEFAULT_CLIENT_PACKET_BASE + 0)
//用户消息起始地址
#define NEW_DEF_USER_BASE                                   (NEW_DEF_DEFAULT_CLIENT_PACKET_BASE +200)
//聊天消息起始地址
#define NEW_DEF_CHAT_BASE                                   (NEW_DEF_DEFAULT_CLIENT_PACKET_BASE + 3000)
//好友消息起始地址
#define NEW_DEF_FRIEND_BASE                                 (NEW_DEF_DEFAULT_CLIENT_PACKET_BASE + 4000)
//群消息起始地址
#define NEW_DEF_GROUP_BASE                                  (NEW_DEF_DEFAULT_CLIENT_PACKET_BASE + 5000)
//公众号起始地址
#define NEW_DEF_OFFCIAL_BASE                                (NEW_DEF_DEFAULT_CLIENT_PACKET_BASE + 6000)
//商家小二起始地址
#define NEW_DEF_BUSINESS_BASE                               (NEW_DEF_DEFAULT_CLIENT_PACKET_BASE + 6500)

//////////////////////////////////////sys start//////////////////////////////////////
//握手包
#define NEW_DEF_HANDSHAKE_RQ                                (NEW_DEF_SYS_BASE + 1)
#define NEW_DEF_HANDSHAKE_RS                                (NEW_DEF_SYS_BASE + 2)
//握手ack
#define NEW_DEF_ACK_ID                                      (NEW_DEF_SYS_BASE + 3)
//心跳包
#define NEW_DEF_HEART_RQ                                    (NEW_DEF_SYS_BASE + 4)
#define NEW_DEF_HEART_RS                                    (NEW_DEF_SYS_BASE + 5)
//服务器主动断开连接通知
#define NEW_DEF_SERVER_DISCON_ID                            (NEW_DEF_SYS_BASE + 6)
//错误包的type
#define NEW_DEF_SERVER_BASE_ERROR                           (NEW_DEF_SYS_BASE + 10)
//登录
#define NEW_DEF_LOGIN_RQ                                    (NEW_DEF_SYS_BASE + 20)
#define NEW_DEF_LOGIN_RS                                    (NEW_DEF_SYS_BASE + 21)
//时间同步请求包
#define NEW_DEF_TIME_SYNC_RQ                                (NEW_DEF_SYS_BASE + 22)
#define NEW_DEF_TIME_SYNC_RS                                (NEW_DEF_SYS_BASE + 23)
//路由信息同步包
#define NEW_DEF_ROUTE_SYNC_ID                               (NEW_DEF_SYS_BASE + 24)
//注册包
#define NEW_DEF_REG_RQ                                      (NEW_DEF_SYS_BASE + 25)
#define NEW_DEF_REG_RS                                      (NEW_DEF_SYS_BASE + 26)
//短信获取验证码包
#define NEW_DEF_SMS_VALID_RQ                                (NEW_DEF_SYS_BASE + 27)
#define NEW_DEF_SMS_VALID_RS                                (NEW_DEF_SYS_BASE + 28)
//////////////////////////////////////sys end///////////////////////////////////////

//////////////////////////////////////user start//////////////////////////////////////
//搜索用户
#define NEW_DEF_USER_SEARCH_RQ                              (NEW_DEF_USER_BASE + 1)
#define NEW_DEF_USER_SEARCH_RS                              (NEW_DEF_USER_BASE + 2)
//修改用户信息
#define NEW_DEF_USER_CHANGE_RQ                              (NEW_DEF_USER_BASE + 5)
#define NEW_DEF_USER_CHANGE_RS                              (NEW_DEF_USER_BASE + 6)
//获取自己的信息
#define NEW_DEF_ME_INFO_RQ                                  (NEW_DEF_USER_BASE + 7)
#define NEW_DEF_ME_INFO_RS                                  (NEW_DEF_USER_BASE + 8)
//获取用户信息
#define NEW_DEF_USER_INFO_RQ                                (NEW_DEF_USER_BASE + 9)
#define NEW_DEF_USER_INFO_RS                                (NEW_DEF_USER_BASE + 10)
//属性变化包
#define NEW_DEF_USER_ATTR_ID                                (NEW_DEF_USER_BASE + 13)
//获取用户列表信息
#define NEW_DEF_USERLST_INFO_RQ                             (NEW_DEF_USER_BASE + 14)
#define NEW_DEF_USERLST_INFO_RS                             (NEW_DEF_USER_BASE + 15)
//推送注册包
#define NEW_DEF_REGISTER_APNS_RQ                            (NEW_DEF_USER_BASE + 20)
#define NEW_DEF_REGISTER_APNS_RS                            (NEW_DEF_USER_BASE + 21)
//举报包
#define NEW_DEF_USER_COMPLAINT_RQ                           (NEW_DEF_USER_BASE + 22)
#define NEW_DEF_USER_COMPLAINT_RS                           (NEW_DEF_USER_BASE + 23)
//修改手机号码包
#define NEW_DEF_CHANGE_MOBILE_RQ                            (NEW_DEF_USER_BASE + 24)
#define NEW_DEF_CHANGE_MOBILE_RS                            (NEW_DEF_USER_BASE + 25)
//修改邮箱包
#define NEW_DEF_CHANGE_MAIL_RQ                              (NEW_DEF_USER_BASE + 26)
#define NEW_DEF_CHANGE_MAIL_RS                              (NEW_DEF_USER_BASE + 27)
//////////////////////////////////////user end///////////////////////////////////////

//////////////////////////////////////chat start//////////////////////////////////////
//消息发送
#define NEW_DEF_CHAT_CLIENT_SEND_MESSAGE_RQ                 (NEW_DEF_CHAT_BASE + 1)
#define NEW_DEF_CHAT_CLIENT_SEND_MESSAGE_RS                 (NEW_DEF_CHAT_BASE + 2)
#define NEW_DEF_CHAT_SERVER_SEND_MESSAGE_RQ                 (NEW_DEF_CHAT_BASE + 3)
#define NEW_DEF_CHAT_SERVER_SEND_MESSAGE_RS                 (NEW_DEF_CHAT_BASE + 4)
//离线消息获取
#define NEW_DEF_CHAT_GET_OFFLINE_MESSAGE_RQ                 (NEW_DEF_CHAT_BASE + 5)
#define NEW_DEF_CHAT_GET_OFFLINE_MESSAGE_RS                 (NEW_DEF_CHAT_BASE + 6)
//获取用户消息状态
#define NEW_DEF_GET_USER_STATUS_RQ                          (NEW_DEF_CHAT_BASE + 7)
#define NEW_DEF_GET_USER_STATUS_RS                          (NEW_DEF_CHAT_BASE + 8)
//设置用户消息免打扰状态
#define NEW_DEF_SINGLE_CHAT_STATUS_RQ                       (NEW_DEF_CHAT_BASE + 9)
#define NEW_DEF_SINGLE_CHAT_STATUS_RS                       (NEW_DEF_CHAT_BASE + 10)
//获取未读数
#define NEW_DEF_GET_MSG_UNREAD_RQ                           (NEW_DEF_CHAT_BASE + 11)
#define NEW_DEF_GET_MSG_UNREAD_RS                           (NEW_DEF_CHAT_BASE + 12)
//////////////////////////////////////chat end///////////////////////////////////////

//////////////////////////////////////friend start//////////////////////////////////////
//获取好友列表
#define NEW_DEF_FRIEND_LIST_RQ                              (NEW_DEF_FRIEND_BASE + 1)
#define NEW_DEF_FRIEND_LIST_RS                              (NEW_DEF_FRIEND_BASE + 2)
//添加好友
#define NEW_DEF_CLIENT_FRIEND_ADD_RQ                        (NEW_DEF_FRIEND_BASE + 5)
#define NEW_DEF_CLIENT_FRIEND_ADD_RS                        (NEW_DEF_FRIEND_BASE + 6)
#define NEW_DEF_SERVER_FRIEND_ADD_RQ                        (NEW_DEF_FRIEND_BASE + 7)
#define NEW_DEF_SERVER_FRIEND_ADD_RS                        (NEW_DEF_FRIEND_BASE + 8)
//好友确认
#define NEW_DEF_SERVER_FRIEND_CONFIRM_RQ                    (NEW_DEF_FRIEND_BASE + 9)
#define NEW_DEF_SERVER_FRIEND_CONFIRM_RS                    (NEW_DEF_FRIEND_BASE + 10)
//删除好友操作
#define NEW_DEF_FRIEND_DEL_RQ                               (NEW_DEF_FRIEND_BASE + 11)
#define NEW_DEF_FRIEND_DEL_RS                               (NEW_DEF_FRIEND_BASE + 12)
//修改备注
#define NEW_DEF_FRIEND_REMARK_RQ                            (NEW_DEF_FRIEND_BASE + 13)
#define NEW_DEF_FRIEND_REMARK_RS                            (NEW_DEF_FRIEND_BASE + 14)
#define NEW_DEF_CLIENT_FRIEND_CONFIRM_RQ                    (NEW_DEF_FRIEND_BASE + 15)
#define NEW_DEF_CLINET_FRIEND_CONFIRM_RS                    (NEW_DEF_FRIEND_BASE + 16)
#define NEW_DEF_SERVER_FRIEND_DEL_RQ                        (NEW_DEF_FRIEND_BASE + 17)
#define NEW_DEF_SERVER_FRIEND_DEL_RS                        (NEW_DEF_FRIEND_BASE + 18)
//黑名单操作
#define NEW_DEF_CLIENT_FRIEND_BLACKLIST_RQ                  (NEW_DEF_FRIEND_BASE + 19)
#define NEW_DEF_CLIENT_FRIEND_BLACKLIST_RS                  (NEW_DEF_FRIEND_BASE + 20)
//黑名单操作
#define NEW_DEF_SERVER_FRIEND_BLACKLIST_RQ                  (NEW_DEF_FRIEND_BASE + 21)
#define NEW_DEF_FRIEND_UPDATE_RQ                            (NEW_DEF_FRIEND_BASE + 22)
#define NEW_DEF_FRIEND_UPDATE_RS                            (NEW_DEF_FRIEND_BASE + 23)
#define NEW_DEF_FRIEND_RESTORE_RQ                           (NEW_DEF_FRIEND_BASE + 24)
//////////////////////////////////////friend end///////////////////////////////////////

//////////////////////////////////////group start//////////////////////////////////////
//创建群
#define NEW_DEF_GROUP_CREATE_RQ                             (NEW_DEF_GROUP_BASE + 1)
#define NEW_DEF_GROUP_CREATE_RS                             (NEW_DEF_GROUP_BASE + 2)
//获取群成员信息
#define NEW_DEF_GROUP_DETAIL_INFO_RQ                        (NEW_DEF_GROUP_BASE + 11)
#define NEW_DEF_GROUP_DETAIL_INFO_RS                        (NEW_DEF_GROUP_BASE + 12)
//获取群列表
#define NEW_DEF_GROUP_LIST_RQ                               (NEW_DEF_GROUP_BASE + 15)
#define NEW_DEF_GROUP_LIST_RS                               (NEW_DEF_GROUP_BASE + 16)
//群消息
#define NEW_DEF_GROUP_CLIENT_SEND_MESSAGE_RQ                (NEW_DEF_GROUP_BASE + 24)
#define NEW_DEF_GROUP_CLIENT_SEND_MESSAGE_RS                (NEW_DEF_GROUP_BASE + 25)
//群离线消息
#define NEW_DEF_GROUP_GET_OFFLINE_MESSAGE_RQ                (NEW_DEF_GROUP_BASE + 28)
#define NEW_DEF_GROUP_GET_OFFLINE_MESSAGE_RS                (NEW_DEF_GROUP_BASE + 29)
//修改群成员(邀请进群、踢人、修改群属性)
#define NEW_DEF_GROUP_MODIFY_CHANGE_RQ                      (NEW_DEF_GROUP_BASE + 34)
#define NEW_DEF_GROUP_MODIFY_CHANGE_RS                      (NEW_DEF_GROUP_BASE + 35)
//群主转让
#define NEW_DEF_GROUP_LEADER_CHANGE_RQ                      (NEW_DEF_GROUP_BASE + 40)
#define NEW_DEF_GROUP_LEADER_CHANGE_RS                      (NEW_DEF_GROUP_BASE + 41)
//获取群组公告
#define NEW_DEF_GROUP_REMARK_DETAIL_RQ                      (NEW_DEF_GROUP_BASE + 44)
#define NEW_DEF_GROUP_REMARK_DETAIL_RS                      (NEW_DEF_GROUP_BASE + 45)
//群消息通知
#define NEW_DEF_GROUP_CHAT_NOTIFY_SIMPLE_RQ                 (NEW_DEF_GROUP_BASE + 46)
//更新群登出时间
#define NEW_DEF_GROUP_UPDATE_LOGOUT_ID                      (NEW_DEF_GROUP_BASE + 50)
//获取群列表ids
#define NEW_DEF_GROUP_LIST_IDS_RQ                           (NEW_DEF_GROUP_BASE + 52)
#define NEW_DEF_GROUP_LIST_IDS_RS                           (NEW_DEF_GROUP_BASE + 53)
//设置群消息状态
#define NEW_DEF_GROUP_MESSAGE_STATUS_RQ                     (NEW_DEF_GROUP_BASE + 54)
#define NEW_DEF_GROUP_MESSAGE_STATUS_RS                     (NEW_DEF_GROUP_BASE + 55)
//获取群类型
#define NEW_DEF_GROUP_TYPE_LIST_RQ                          (NEW_DEF_GROUP_BASE + 56)
#define NEW_DEF_GROUP_TYPE_LIST_RS                          (NEW_DEF_GROUP_BASE + 57)
//获取用户备注昵称
#define NEW_DEF_GROUP_GET_USER_REMARK_NAME_RQ               (NEW_DEF_GROUP_BASE + 58)
#define NEW_DEF_GROUP_GET_USER_REMARK_NAME_RS               (NEW_DEF_GROUP_BASE + 59)
//设置用户存储方式
#define NEW_DEF_GROUP_SAVE_CHANGE_RQ                        (NEW_DEF_GROUP_BASE + 60)
#define NEW_DEF_GROUP_SAVE_CHANGE_RS                        (NEW_DEF_GROUP_BASE + 61)
//扫描二维码获取群信息
#define NEW_DEF_GROUP_SCAN_RQ                               (NEW_DEF_GROUP_BASE + 62)
#define NEW_DEF_GROUP_SCAN_RS                               (NEW_DEF_GROUP_BASE + 63)
//批量获取群信息
#define NEW_DEF_GROUP_BATCH_INFO_RQ                         (NEW_DEF_GROUP_BASE + 64)
#define NEW_DEF_GROUP_BATCH_INFO_RS                         (NEW_DEF_GROUP_BASE + 65)
//////////////////////////////////////group end///////////////////////////////////////

//////////////////////////////////////offcial start//////////////////////////////////////
//粉丝端消息
#define NEW_DEF_CLIENT_FANS_SEND_MESSAGE_RQ                 (NEW_DEF_OFFCIAL_BASE + 1)
#define NEW_DEF_CLIENT_FANS_SEND_MESSAGE_RS                 (NEW_DEF_OFFCIAL_BASE + 2)
#define NEW_DEF_SERVER_OFFCIAL_MESSAGE_ID                   (NEW_DEF_OFFCIAL_BASE + 3)
#define NEW_DEF_SERVER_OFFCIAL_PRIVATE_MESSAGE_RQ           (NEW_DEF_OFFCIAL_BASE + 5)
#define NEW_DEF_SERVER_OFFCIAL_PRIVATE_MESSAGE_RS           (NEW_DEF_OFFCIAL_BASE + 6)
//公众号端消息
#define NEW_DEF_CLIENT_OFFCIAL_SEND_MESSAGE_RQ              (NEW_DEF_OFFCIAL_BASE + 7)
#define NEW_DEF_CLIENT_OFFCIAL_SEND_MESSAGE_RS              (NEW_DEF_OFFCIAL_BASE + 8)
#define NEW_DEF_SERVER_FANS_MESSAGE_RQ                      (NEW_DEF_OFFCIAL_BASE + 9)
#define NEW_DEF_SERVER_FANS_MESSAGE_RS                      (NEW_DEF_OFFCIAL_BASE + 10)
//公众号离线消息
#define NEW_DEF_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RQ       (NEW_DEF_OFFCIAL_BASE + 11)
#define NEW_DEF_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RS       (NEW_DEF_OFFCIAL_BASE + 12)
#define NEW_DEF_CLIENT_FANS_GET_OFFLINE_MESSAGE_RQ          (NEW_DEF_OFFCIAL_BASE + 13)
#define NEW_DEF_CLIENT_FANS_GET_OFFLINE_MESSAGE_RS          (NEW_DEF_OFFCIAL_BASE + 14)
//公众号系统消息
#define NEW_DEF_CLIENT_OFFCIAL_SEND_SYS_MESSAGE_RQ          (NEW_DEF_OFFCIAL_BASE + 15)
#define NEW_DEF_CLIENT_OFFCIAL_SEND_SYS_MESSAGE_RS          (NEW_DEF_OFFCIAL_BASE + 16)
#define NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_SYS_MESSAGE_RQ     (NEW_DEF_OFFCIAL_BASE + 17)
#define NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_SYS_MESSAGE_RS     (NEW_DEF_OFFCIAL_BASE + 18)
#define NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_SYS_MESSAGE_RQ      (NEW_DEF_OFFCIAL_BASE + 19)
#define NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_SYS_MESSAGE_RS      (NEW_DEF_OFFCIAL_BASE + 20)
#define NEW_DEF_CLIENT_FANS_GET_SYS_MESSAGE_RQ              (NEW_DEF_OFFCIAL_BASE + 21)
#define NEW_DEF_CLIENT_FANS_GET_SYS_MESSAGE_RS              (NEW_DEF_OFFCIAL_BASE + 22)
//公众号端私聊消息
#define NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_MESSAGE_RQ          (NEW_DEF_OFFCIAL_BASE + 23)
#define NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_MESSAGE_RS          (NEW_DEF_OFFCIAL_BASE + 24)
#define NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_MESSAGE_RQ         (NEW_DEF_OFFCIAL_BASE + 25)
#define NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_MESSAGE_RS         (NEW_DEF_OFFCIAL_BASE + 26)
//商家小二获取是否有未读消息
#define NEW_DEF_CLIENT_OFFCIAL_GET_UNREAD_CNT_RQ            (NEW_DEF_OFFCIAL_BASE + 27)
#define NEW_DEF_CLIENT_OFFCIAL_GET_UNREAD_CNT_RS            (NEW_DEF_OFFCIAL_BASE + 28)
//////////////////////////////////////offcial end///////////////////////////////////////

//////////////////////////////////////business start//////////////////////////////////////
//请求小二分配
#define NEW_DEF_BUSINESS_GETFREEWAITER_RQ                   (NEW_DEF_BUSINESS_BASE +1)
#define NEW_DEF_BUSINESS_GETFREEWAITER_RS                   (NEW_DEF_BUSINESS_BASE +2)
//最近联系人
#define NEW_DEF_BUSINESS_GETRECENTUSERS_RQ                  (NEW_DEF_BUSINESS_BASE +3)
#define NEW_DEF_BUSINESS_GETRECENTUSERS_RS                  (NEW_DEF_BUSINESS_BASE +4)
//历史消息
#define NEW_DEF_BUSINESS_GETRECENTMSGS_RQ                   (NEW_DEF_BUSINESS_BASE+5)
#define NEW_DEF_BUSINESS_GETRECENTMSGS_RS                   (NEW_DEF_BUSINESS_BASE+6)
//最后一条历史消息
#define NEW_DEF_BUSINESS_GETLASTMSGS_RQ                     (NEW_DEF_BUSINESS_BASE+7)
#define NEW_DEF_BUSINESS_GETLASTMSGS_RS                     (NEW_DEF_BUSINESS_BASE+8)
//离线消息
#define NEW_DEF_BUSINESS_GETOFFLINEMSGS_RQ                  (NEW_DEF_BUSINESS_BASE+9)
#define NEW_DEF_BUSINESS_GETOFFLINEMSGS_RS                  (NEW_DEF_BUSINESS_BASE+10)
//离线消息数目
#define NEW_DEF_BUSINESS_GETOFFLINECNT_RQ                   (NEW_DEF_BUSINESS_BASE+11)
#define NEW_DEF_BUSINESS_GETOFFLINECNT_RS                   (NEW_DEF_BUSINESS_BASE+12)
//设置小二状态
#define NEW_DEF_BUSINESS_SETLOGINSTATUS_RQ                  (NEW_DEF_BUSINESS_BASE+13)
#define NEW_DEF_BUSINESS_SETLOGINSTATUS_RS                  (NEW_DEF_BUSINESS_BASE+14)
//更新登录时间
#define NEW_DEF_BUSINESS_LOGIN_ID                           (NEW_DEF_BUSINESS_BASE + 16)
//获取最新订单
#define NEW_DEF_BUSINESS_GETRECENTORDERS_RQ                 (NEW_DEF_BUSINESS_BASE + 17)
#define NEW_DEF_BUSINESS_GETRECENTORDERS_RS                 (NEW_DEF_BUSINESS_BASE + 18)
//查询用户服务状态
#define NEW_DEF_BUSINESS_ISUSERBUSY_RQ                      (NEW_DEF_BUSINESS_BASE + 19)
#define NEW_DEF_BUSINESS_ISUSERBUSY_RS                      (NEW_DEF_BUSINESS_BASE + 20)
//设置商家信息
#define NEW_DEF_BUSINESS_SETINFO_RQ                         (NEW_DEF_BUSINESS_BASE + 21)
#define NEW_DEF_BUSINESS_SETINFO_RS                         (NEW_DEF_BUSINESS_BASE + 22)
//获取商家信息
#define NEW_DEF_BUSINESS_GETINFO_RQ                         (NEW_DEF_BUSINESS_BASE + 23)
#define NEW_DEF_BUSINESS_GETINFO_RS                         (NEW_DEF_BUSINESS_BASE + 24)
//设置商家提示语信息
#define NEW_DEF_BUSINESS_SETPROMPTS_RQ                      (NEW_DEF_BUSINESS_BASE + 25)
#define NEW_DEF_BUSINESS_SETPROMPTS_RS                      (NEW_DEF_BUSINESS_BASE + 26)
//获取商家提示语信息
#define NEW_DEF_BUSINESS_GETPROMPTS_RQ                      (NEW_DEF_BUSINESS_BASE + 27)
#define NEW_DEF_BUSINESS_GETPROMPTS_RS                      (NEW_DEF_BUSINESS_BASE + 28)
//////////////////////////////////////business end///////////////////////////////////////




#endif //__PACKTYPEDEF_H