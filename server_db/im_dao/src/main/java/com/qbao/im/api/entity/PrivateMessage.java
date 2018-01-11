package com.qbao.im.api.entity;

import java.util.Date;

/**
 * Created by tangxiaojun on 2017/6/7.
 * 私聊消息表
 */
public class PrivateMessage extends BaseEntity {

    private Integer b_id;

    private Integer c_id;

    private Integer w_id;

    private Integer send_userid;

    private String send_nickname;

    private Integer op_user_id;

    private Integer app_id;

    private Integer session_id;

    private Integer c_type;

    private Integer b_type;

    private Integer ext_type;

    private String msg;

    private String msg_preview;

    private Date create_date;

    private Integer msg_status;

    private String message_id;

    public Integer getB_id() {
        return b_id;
    }

    public void setB_id(Integer b_id) {
        this.b_id = b_id;
    }

    public Integer getC_id() {
        return c_id;
    }

    public void setC_id(Integer c_id) {
        this.c_id = c_id;
    }

    public Integer getW_id() {
        return w_id;
    }

    public void setW_id(Integer w_id) {
        this.w_id = w_id;
    }

    public Integer getSend_userid() {
        return send_userid;
    }

    public void setSend_userid(Integer send_userid) {
        this.send_userid = send_userid;
    }

    public String getSend_nickname() {
        return send_nickname;
    }

    public void setSend_nickname(String send_nickname) {
        this.send_nickname = send_nickname;
    }

    public Integer getOp_user_id() {
        return op_user_id;
    }

    public void setOp_user_id(Integer op_user_id) {
        this.op_user_id = op_user_id;
    }

    public Integer getApp_id() {
        return app_id;
    }

    public void setApp_id(Integer app_id) {
        this.app_id = app_id;
    }

    public Integer getSession_id() {
        return session_id;
    }

    public void setSession_id(Integer session_id) {
        this.session_id = session_id;
    }

    public Integer getC_type() {
        return c_type;
    }

    public void setC_type(Integer c_type) {
        this.c_type = c_type;
    }

    public Integer getB_type() {
        return b_type;
    }

    public void setB_type(Integer b_type) {
        this.b_type = b_type;
    }

    public Integer getExt_type() {
        return ext_type;
    }

    public void setExt_type(Integer ext_type) {
        this.ext_type = ext_type;
    }

    public String getMsg() {
        return msg;
    }

    public void setMsg(String msg) {
        this.msg = msg;
    }

    public String getMsg_preview() {
        return msg_preview;
    }

    public void setMsg_preview(String msg_preview) {
        this.msg_preview = msg_preview;
    }

    public Date getCreate_date() {
        return create_date;
    }

    public void setCreate_date(Date create_date) {
        this.create_date = create_date;
    }

    public Integer getMsg_status() {
        return msg_status;
    }

    public void setMsg_status(Integer msg_status) {
        this.msg_status = msg_status;
    }

    public String getMessage_id() {
        return message_id;
    }

    public void setMessage_id(String message_id) {
        this.message_id = message_id;
    }
}
