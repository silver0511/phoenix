package com.qbao.im.api.entity;

import com.qbao.im.api.DBEnum;

import java.util.Date;

/**
 * Created by tangxiaojun on 2017/4/14.
 * 群成员
 */
public class GroupMember extends BaseEntity {

    private Long orderid;

    //群号
    private transient Long group_id;

    //用户编号
    private Integer user_id;

    //群成员类型，0:普通成员 1:管理员 2:群主
    private transient Integer member_type;

    //群成员昵称
    private String member_nickname;

    private transient Date member_joindate;

    private transient Date update_date;

    //0:正常1:收消息不提醒2:收入群助手不提醒
    private Integer rcv_msg_status;

    private Integer is_address_book;

    @Override
    public String getTabName() {
        Long modVal = this.group_id % 32;
        String modValStr = modVal+"";
        String tabName = null;
        if(modValStr.length()>1){
            tabName = DBEnum.GROUP_MEMBER_SHIP.getValue().concat(modVal+"");
        }else {
            tabName = DBEnum.GROUP_MEMBER_SHIP.getValue().concat("0"+modVal+"");
        }
        return tabName;
    }

    public Long getOrderid() {
        return orderid;
    }

    public void setOrderid(Long orderid) {
        this.orderid = orderid;
    }

    public Long getGroup_id() {
        return group_id;
    }

    public void setGroup_id(Long group_id) {
        this.group_id = group_id;
        Long modVal = this.group_id % 32;
        String modValStr = modVal+"";
        String tabName = null;
        if(modValStr.length()>1){
            tabName = DBEnum.GROUP_MEMBER_SHIP.getValue().concat(modVal+"");
        }else {
            tabName = DBEnum.GROUP_MEMBER_SHIP.getValue().concat("0"+modVal+"");
        }
        super.setTabName(tabName);
    }

    public Integer getUser_id() {
        return user_id;
    }

    public void setUser_id(Integer user_id) {
        this.user_id = user_id;
    }

    public Integer getMember_type() {
        return member_type;
    }

    public void setMember_type(Integer member_type) {
        this.member_type = member_type;
    }

    public String getMember_nickname() {
        return member_nickname;
    }

    public void setMember_nickname(String member_nickname) {
        this.member_nickname = member_nickname;
    }

    public Date getMember_joindate() {
        return member_joindate;
    }

    public void setMember_joindate(Date member_joindate) {
        this.member_joindate = member_joindate;
    }

    public Date getUpdate_date() {
        return update_date;
    }

    public void setUpdate_date(Date update_date) {
        this.update_date = update_date;
    }

    public Integer getRcv_msg_status() {
        return rcv_msg_status;
    }

    public void setRcv_msg_status(Integer rcv_msg_status) {
        this.rcv_msg_status = rcv_msg_status;
    }

    public Integer getIs_address_book() {
        return is_address_book;
    }

    public void setIs_address_book(Integer is_address_book) {
        this.is_address_book = is_address_book;
    }
}
