package com.qbao.im.api.msg;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by tangxiaojun on 2017/3/28.
 */
public abstract class ErrorMsg {

    private static Map<Integer, String> errorDetailMap = new HashMap<Integer, String>();

    public static String getErrorDetail(int errorCode) {
        if (!errorDetailMap.containsKey(errorCode)) {
            return "未知的错误信息:" + errorCode;
        }
        return errorDetailMap.get(errorCode);
    }

    static {

        //////////////////////////////////////sys start//////////////////////////////////////
        errorDetailMap.put(ErrorCode.RET_SYS_DISCON_USER_KICKED, "此账号在另一个浏览器登陆了");
        errorDetailMap.put(ErrorCode.RET_UNPACK_FAILED_RESULT, "服务器解包是失败");
        errorDetailMap.put(ErrorCode.RET_PLATFORM_ERROR, "错误的登录平台");
        //////////////////////////////////////sys end///////////////////////////////////////

        //////////////////////////////////////user start//////////////////////////////////////
        errorDetailMap.put(ErrorCode.RET_USERINFO_BASE, "用户不存在");
        errorDetailMap.put(ErrorCode.RET_ADDUSER_BASE, "用户已存在");
        errorDetailMap.put(ErrorCode.RET_UPDATEUSERINFO_BASE, "用户属性不存在");
        //////////////////////////////////////user end///////////////////////////////////////

        //////////////////////////////////////chat start//////////////////////////////////////
        //////////////////////////////////////chat end///////////////////////////////////////

        //////////////////////////////////////friend start//////////////////////////////////////
        errorDetailMap.put(ErrorCode.RET_FRIEND_ALREADY_EXISTED,"好友关系已经存在");
        //////////////////////////////////////friend end///////////////////////////////////////

        //////////////////////////////////////group start//////////////////////////////////////
        errorDetailMap.put(ErrorCode.RET_CREATE_USER_LIST_EMPTY, "建群用户列表为空");
        errorDetailMap.put(ErrorCode.RET_OPERATE_TYPE_ERROR, "用户操作无效");
        errorDetailMap.put(ErrorCode.RET_GROUP_ID_INVALID, "群组不存在");
        errorDetailMap.put(ErrorCode.RET_GROUP_OPREATE_USER_ID_INVALID, "用户不是群主");
        errorDetailMap.put(ErrorCode.RET_GROUP_USER_NOT_JOIN, "用户未加入群");
        errorDetailMap.put(ErrorCode.RET_GROUP_USER_HAS_JOIN, "用户已经加入该群");
        errorDetailMap.put(ErrorCode.RET_GROUP_OPERATE_INFO_ERROR, "踢人或者邀请用户信息错误");
        errorDetailMap.put(ErrorCode.RET_GROUP_CREATE_MAX_COUNT, "建群超过默认最大数");
        errorDetailMap.put(ErrorCode.RET_GROUP_INVITE_FAILED, "添加用户失败");
        errorDetailMap.put(ErrorCode.RET_GROUP_KICK_FAILED, "踢人失败");
        errorDetailMap.put(ErrorCode.RET_GROUP_LEADER_CHANGE_SELF, "已经是群主");
        errorDetailMap.put(ErrorCode.RET_GROUP_LEADER_NAME_IS_NIL, "被转让用户参数有误");
        errorDetailMap.put(ErrorCode.RET_GROUP_AGREE_DEFAULT, "群当前为默认加入");
        errorDetailMap.put(ErrorCode.RET_GROUP_AGREE_USER, "群当前为需要群主同意");
        errorDetailMap.put(ErrorCode.DEF_GROUP_AGREE_OLD_MESSAGE_ID_INVALID, "群主同意失败");
        errorDetailMap.put(ErrorCode.RET_GROUP_ADD_MESSAGE_ERROR,"群消息保存失败");
        errorDetailMap.put(ErrorCode.RET_GROUP_SINGLE_LIMIT_ERROR,"单次加人超上限");
        errorDetailMap.put(ErrorCode.RET_GROUP_MODIFY_REMARK_ERROR,"修改群备注失败");
        //////////////////////////////////////group end///////////////////////////////////////

        //////////////////////////////////////offcial start//////////////////////////////////////
        //////////////////////////////////////offcial end///////////////////////////////////////

        //////////////////////////////////////business start//////////////////////////////////////
        //////////////////////////////////////business end///////////////////////////////////////
    }
}
