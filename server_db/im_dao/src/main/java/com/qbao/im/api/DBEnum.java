package com.qbao.im.api;

/**
 * Created by tangxiaojun on 2017/3/28.
 */
public enum DBEnum {

    FRIEND_SHIP("nim_friend_"),GROUP_SHIP("nim_group_"),GROUP_MEMBER_SHIP("nim_group_member_"),MEMBER_GROUP_LIST("nim_user_group_list_"),
    BATCH_TEST("nim_test_");

    private String value;

    private DBEnum(String value) {
        this.value = value;
    }

    public String getValue(){
        return value;
    }

    public static void main(String[] args) {
        System.out.println(DBEnum.FRIEND_SHIP.getValue());
    }
}
