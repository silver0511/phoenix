package com.qbao.im.api.constants;

/**
 * Created by tangxiaojun on 2017/7/26.
 */
public enum FriendBlackEnums {

    /**
     * 0:正常状态
     * 1:被动黑名单状态
     * 2:黑名单状态
     * 3:互为黑名单
     */
    NORMAL(0),BLACK_LIST(2),BE_BLACK_LIST(1),EACH_BLACK_LIST(3);

    private int value;

    FriendBlackEnums(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
