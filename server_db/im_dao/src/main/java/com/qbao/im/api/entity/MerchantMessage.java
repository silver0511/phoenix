package com.qbao.im.api.entity;

import java.util.Date;

/**
 * Created by tangxiaojun on 2017/6/13.
 * 商家消息
 */
public class MerchantMessage extends BaseEntity{

    private Integer b_id;

    private Integer w_id;

    private Integer c_id;

    private Integer from_id;

    private Integer to_id;

    private Integer app_id;

    private Long message_id;

    private Integer session_id;

    private String send_user_name;

    private Integer chat_type;

    private Integer m_type;

    private Integer s_type;

    private Integer ext_type;

    private Long msg_time;

    private Date create_time;

    private String msg;

    public Integer getB_id() {
        return b_id;
    }

    public void setB_id(Integer b_id) {
        this.b_id = b_id;
    }

    public Integer getW_id() {
        return w_id;
    }

    public void setW_id(Integer w_id) {
        this.w_id = w_id;
    }

    public Integer getC_id() {
        return c_id;
    }

    public void setC_id(Integer c_id) {
        this.c_id = c_id;
    }

    public Integer getFrom_id() {
        return from_id;
    }

    public void setFrom_id(Integer from_id) {
        this.from_id = from_id;
    }

    public Integer getTo_id() {
        return to_id;
    }

    public void setTo_id(Integer to_id) {
        this.to_id = to_id;
    }

    public Integer getApp_id() {
        return app_id;
    }

    public void setApp_id(Integer app_id) {
        this.app_id = app_id;
    }

    public Long getMessage_id() {
        return message_id;
    }

    public void setMessage_id(Long message_id) {
        this.message_id = message_id;
    }

    public Integer getSession_id() {
        return session_id;
    }

    public void setSession_id(Integer session_id) {
        this.session_id = session_id;
    }

    public Integer getChat_type() {
        return chat_type;
    }

    public void setChat_type(Integer chat_type) {
        this.chat_type = chat_type;
    }

    public Integer getM_type() {
        return m_type;
    }

    public void setM_type(Integer m_type) {
        this.m_type = m_type;
    }

    public Integer getS_type() {
        return s_type;
    }

    public void setS_type(Integer s_type) {
        this.s_type = s_type;
    }

    public Integer getExt_type() {
        return ext_type;
    }

    public void setExt_type(Integer ext_type) {
        this.ext_type = ext_type;
    }

    public Long getMsg_time() {
        return msg_time;
    }

    public void setMsg_time(Long msg_time) {
        this.msg_time = msg_time;
    }

    public Date getCreate_time() {
        return create_time;
    }

    public void setCreate_time(Date create_time) {
        this.create_time = create_time;
    }

    public String getMsg() {
        return msg;
    }

    public void setMsg(String msg) {
        this.msg = msg;
    }

    public String getSend_user_name() {
        return send_user_name;
    }

    public void setSend_user_name(String send_user_name) {
        this.send_user_name = send_user_name;
    }
}
