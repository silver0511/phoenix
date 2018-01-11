#ifndef __ERRORCODEDEF_H_
#define __ERRORCODEDEF_H_


//成功
#define RET_SUCCESS                                         0x80000000
//网络包解析失败
#define RET_ERR_NET_RECV_FAILED                             0x00000001
//系统错误偏移
#define RET_SYS_BASE                                        0x10000000
//用户错误偏移
#define RET_USER_BASE                                       0x20000000
//聊天错误偏移
#define RET_CHAT_BASE                                       0x22000000
//好友逻辑错误
#define RET_FRIEND_BASE                                     0x24000000
//群组错误
#define RET_GROUP_BASE                                      0x26000000
//公众号错误
#define RET_OFFCIAL_BASE                                    0x28000000
//商家错误
#define RET_BUSINESS_BASE                                   0x30000000

//////////////////////////////////////sys start//////////////////////////////////////
//此账号在另一处登录了
#define RET_SYS_DISCON_USER_KICKED                          (RET_SYS_BASE + 1)
//服务器解包失败
#define RET_UNPACK_FAILED_RESULT                            (RET_SYS_BASE + 2)
//错误的登录平台
#define RET_PLATFORM_ERROR                                  (RET_SYS_BASE + 3)
//请求过于频繁
#define RET_REQ_FAST_ERROR                                  (RET_SYS_BASE + 4)
//redis操作失败
#define RET_REQ_REDIS_ERROR                                 (RET_SYS_BASE + 5)
//服务异常，请稍后再试
#define RET_SYS_PACK_TYPE_INVALID                           (RET_SYS_BASE + 6)
//此账号已登录
#define RET_SYS_REPEAT_LOGINED                              (RET_SYS_BASE + 7)
//注册失败
#define RET_REG_FAILED                                      (RET_SYS_BASE + 8)
//无效的验证码
#define RET_INVALID_VERITY_CODE                             (RET_SYS_BASE + 9)
//验证码获取太频繁
#define RET_VERIFY_CODE_FREQUENCY                           (RET_SYS_BASE + 10)
//////////////////////////////////////sys end///////////////////////////////////////

//////////////////////////////////////user start//////////////////////////////////////
//用户不存在
#define RET_USERINFO_BASE                                   (RET_USER_BASE + 1)
//用户已存在
#define RET_ADDUSER_BASE                                    (RET_USER_BASE + 2)
//用户属性不存在
#define RET_UPDATEUSERINFO_BASE                             (RET_USER_BASE + 3)
//用户信息已是最新
#define RET_GETUSERINFO_BASE                                (RET_USER_BASE + 4)
//上传用户信息属性不全(user_name和mobile为必填项)
#define RET_NOATTRIBUTE_BASE                                (RET_USER_BASE + 5)
//user_name已存在
#define RET_USERNAME_BASE                                   (RET_USER_BASE + 6)
//mobile已存在
#define RET_MOBILE_BASE                                     (RET_USER_BASE + 7)
//mail已存在
#define RET_MAIL_BASE                                       (RET_USER_BASE + 8)
//举报类型错误
#define RET_COMPLAINTTYPE_BASE                              (RET_USER_BASE + 9)
//新手机号码不能与旧手机号码相同
#define RET_NEW_MOBILE_BASE                                 (RET_USER_BASE + 10)
//mobile验证失败
#define RET_CHECK_MOBILE_BASE                               (RET_USER_BASE + 11)
//新邮箱不能与旧邮箱相同
#define RET_NEW_MAIL_BASE                                   (RET_USER_BASE + 12)
//旧邮箱验证失败
#define RET_CHECK_MAIL_BASE                                 (RET_USER_BASE + 13)
//////////////////////////////////////user end///////////////////////////////////////

//////////////////////////////////////chat start//////////////////////////////////////
//上传方式不正确
#define RET_CHAT_UPLOAD_METHOD                              (RET_CHAT_BASE + 1)
//上传格式不正确
#define RET_CHAT_UPLOAD_TYPE                                (RET_CHAT_BASE + 2)
//上传失败
#define RET_CHAT_UPLOAD_RESULT                              (RET_CHAT_BASE + 3)
//对端用户id错误
#define RET_CHAT_SINGLE_STATUS_OP_USER_ID_INVALID           (RET_CHAT_BASE + 4)
//消息内容过长
#define RET_CHAT_MSG_CONTENT_MAX                            (RET_CHAT_BASE + 5)
//////////////////////////////////////chat end///////////////////////////////////////

//////////////////////////////////////friend start//////////////////////////////////////
//好友已存在
#define RET_FRIEND_ALREADY_EXISTED                          (RET_FRIEND_BASE + 1)
//查询好友列表失败
#define RET_FRIEND_LIST_ERROR                               (RET_FRIEND_BASE + 2)
//添加好友失败
#define RET_FRIEND_ADD_ERROR                                (RET_FRIEND_BASE + 3)
//删除好友失败
#define RET_FRIEND_DEL_ERROR                                (RET_FRIEND_BASE +4)
//修改好友信息失败
#define RET_FRIEND_MODIFY_ERROR                             (RET_FRIEND_BASE +5)
//好友确认请求超时
#define RET_FRIEND_CONFIRM_TIMEOUT_ERROR                    (RET_FRIEND_BASE + 6)
//好友同意失败
#define RET_FRIEND_AGREE_ERROR                              (RET_FRIEND_BASE + 7)
//好友拒绝失败
#define RET_FRIEND_REFUSE_ERROR                             (RET_FRIEND_BASE + 8)
//黑名单设置失败
#define RET_FRIEND_BLACK_LIST_ERROR                         (RET_FRIEND_BASE + 9)
//添加好友失败，是对方的黑名单用户
#define RET_FRIEND_HAS_BLACK_ERROR                          (RET_FRIEND_BASE + 10)
//设置状态失败
#define RET_FRIEND_SETTING_STATUS_ERROR                     (RET_FRIEND_BASE + 11)
//已经在黑名单里
#define RET_FRIEND_HAVE_BLACK_ERROR                         (RET_FRIEND_BASE + 12)
//被删除好友，恢复好友关系
#define RET_FRIEND_BE_DELETE_ERROR                          (RET_FRIEND_BASE + 13)
//等待好友处理
#define RET_FRIEND_RELATION_ERROR                           (RET_FRIEND_BASE + 14)
//你已达到好友上限
#define RET_FRIEND_MAXCNT_ERROR                             (RET_FRIEND_BASE + 15)
//对方好友达到上限
#define RET_FRIEND_PEERMAXCNT_ERROR                         (RET_FRIEND_BASE + 16)
//好友备注名过长
#define RET_FRIEND_REMARK_ERROR                             (RET_FRIEND_BASE + 17)
//////////////////////////////////////friend end///////////////////////////////////////

//////////////////////////////////////group start//////////////////////////////////////
//建群用户列表为空
#define RET_CREATE_USER_LIST_EMPTY                          (RET_GROUP_BASE + 1)
//用户操作无效
#define RET_OPERATE_TYPE_ERROR                              (RET_GROUP_BASE + 2)
//群组不存在
#define RET_GROUP_ID_INVALID                                (RET_GROUP_BASE + 3)
//用户不是群主
#define RET_GROUP_OPREATE_USER_ID_INVALID                   (RET_GROUP_BASE + 4)
//用户未加入群
#define RET_GROUP_USER_NOT_JOIN                             (RET_GROUP_BASE + 5)
//用户已经加入该群
#define RET_GROUP_USER_HAS_JOIN                             (RET_GROUP_BASE + 6)
//踢人或者邀请用户信息错误
#define RET_GROUP_OPERATE_INFO_ERROR                        (RET_GROUP_BASE + 7)
//建群超过默认最大数
#define RET_GROUP_CREATE_MAX_COUNT                          (RET_GROUP_BASE + 8)
//添加用户失败
#define RET_GROUP_INVITE_FAILED                             (RET_GROUP_BASE + 9)
//踢人失败
#define RET_GROUP_KICK_FAILED                               (RET_GROUP_BASE + 10)
//已经是群主
#define RET_GROUP_LEADER_CHANGE_SELF                        (RET_GROUP_BASE + 11)
//被转让用户参数有误
#define RET_GROUP_LEADER_NAME_IS_NIL                        (RET_GROUP_BASE + 12)
//群当前为默认加入
#define RET_GROUP_AGREE_DEFAULT                             (RET_GROUP_BASE + 13)
//群当前为需要群主同意
#define RET_GROUP_AGREE_USER                                (RET_GROUP_BASE + 14)
//群主同意失败
#define DEF_GROUP_AGREE_OLD_MESSAGE_ID_INVALID              (RET_GROUP_BASE + 15)
//添加群组失败
#define RET_GROUP_ADD_ERROR                                 (RET_GROUP_BASE + 16)
//当前群组已经存在
#define RET_GROUP_ADD_EXSIST                                (RET_GROUP_BASE + 17)
//获取群成员列表失败
#define RET_GROUP_MEMBER_LIST_ERROR                         (RET_GROUP_BASE + 18)
//群主转让失败
#define RET_GROUP_MEMBER_CHANGE_ERROR                       (RET_GROUP_BASE + 19)
//群消息保存失败
#define RET_GROUP_ADD_MESSAGE_ERROR                         (RET_GROUP_BASE + 20)
//群人数已上限
#define RET_GROUP_MAX_LIMIT_ERROR                           (RET_GROUP_BASE + 21)
//单次拉人上限
#define RET_GROUP_SINGLE_LIMIT_ERROR                        (RET_GROUP_BASE + 22)
//修改群公告失败
#define RET_GROUP_MODIFY_REMARK_ERROR                       (RET_GROUP_BASE + 23)
//群聊消息id无效
#define RET_GROUP_MESSAGE_ID_INVALID                        (RET_GROUP_BASE + 24)
//批量获取群信息列表为空
#define RET_GROUP_BATCH_GET_LIST_EMPTY                      (RET_GROUP_BASE + 25)
//批量获取群信息列表过大
#define RET_GROUP_BATCH_LIST_INVALID                        (RET_GROUP_BASE + 26)
//批量获取群离线超过上限
#define RET_GROUP_OFFLINE_MAX_COUNT                         (RET_GROUP_BASE + 27)
//修改群名称失败名称过长
#define RET_GROUP_MODIFY_NAME_ERROR                         (RET_GROUP_BASE + 28)
//修改群成员昵称失败昵称过长
#define RET_GROUP_MODIFY_NICK_NAME_ERROR                    (RET_GROUP_BASE + 29)
//收到重发群信息包
#define RET_GROUP_REPEATED_PACK                             (RET_GROUP_BASE + 30)
//////////////////////////////////////group end///////////////////////////////////////

//////////////////////////////////////offcial start//////////////////////////////////////
//公众号消息发送过于频繁
#define RET_OFFCIALMSG_BASE                                 (RET_OFFCIAL_BASE + 1)
//公众号用户名不能为空
#define RET_OFFCIALNAME_BASE                                (RET_OFFCIAL_BASE + 2)
//公众号消息结构不全
#define RET_OFFCIALMSG_CONTENT_BASE                         (RET_OFFCIAL_BASE + 3)
//////////////////////////////////////offcial end///////////////////////////////////////

//////////////////////////////////////business start//////////////////////////////////////
//////////////////////////////////////business end///////////////////////////////////////




#endif //__ERRORCODEDEF_H_