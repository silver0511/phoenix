package com.qbao.im.api.msg;

/**
 * Created by tangxiaojun on 2017/3/29.
 */
public class BaseMessage {

    private String user_id;

    public BaseMessage() {
    }

    public String getUser_id() {
        return user_id;
    }

    public void setUser_id(String user_id) {
        this.user_id = user_id;
    }

    public BaseMessage(String user_id) {
        this.user_id = user_id;
    }

}
