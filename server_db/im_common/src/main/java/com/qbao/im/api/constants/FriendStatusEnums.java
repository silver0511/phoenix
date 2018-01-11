package com.qbao.im.api.constants;

/**
 * Created by tangxiaojun on 2017/7/24.
 */
public enum FriendStatusEnums {

    /**
     * 0:好友关系
     * 1:好友拒绝
     * 2:对端验证
     * 3:好友验证
     * 4:任意好友状态
     * 5:被动删除好友
     * 6:主动删除好友
     * 9:无效
     * 11:被删除后申请加好友
     */
    FRIEND(0),TARGET_VALID(2),FRIEND_VALID(3),BEDELETE(7),DELETE(8),INVALID(9),BEDELETEAPPLY(11);

    private int value;

    FriendStatusEnums(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
