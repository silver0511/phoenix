package com.qbao.im.api.entity;

import com.qbao.im.api.DBEnum;

import java.util.Date;

/**
 * Created by tangxiaojun on 2017/4/14.
 */
public class Group extends BaseEntity {

    //群号
    private Long group_id;

    //群名称
    private String group_name;

    //群头像
    private String group_avater;

    //群备注
    private String group_remark;

    //是否需要群主同意
    private Integer group_add_is_agree;

    //群创建人
    private Integer group_manager_user_id;

    private Integer initCreateid;

    //群当前人数
    private Integer group_count;

    //群最大人数
    private Integer group_max_count;

    //群介绍
    private String introduce;

    //创建时间
    private Date group_ct;

    //修改时间
    private Date update_date;

    //修改人
    private Integer update_user;

    //单次加人人数限制
    private Integer group_add_max_count;

    private Integer is_valid;

    private Integer message_status;

    //是否群成员
    private Integer isMember;
    
    //群公告修改时间
    private Date remark_update_time;

    @Override
    public String getTabName() {
        Long modVal = this.group_id %32;
        String modValStr = modVal+"";
        String tabName = null;
        if(modValStr.length()>1){
            tabName = DBEnum.GROUP_SHIP.getValue().concat(modVal+"");
        }else {
            tabName = DBEnum.GROUP_SHIP.getValue().concat("0"+modVal+"");
        }
        return tabName;
    }

    public Long getGroup_id() {
        return group_id;
    }

    public void setGroup_id(Long group_id) {
        this.group_id = group_id;
        Long modVal = this.group_id %32;
        String modValStr = modVal+"";
        String tabName = null;
        if(modValStr.length()>1){
            tabName = DBEnum.GROUP_SHIP.getValue().concat(modVal+"");
        }else {
            tabName = DBEnum.GROUP_SHIP.getValue().concat("0"+modVal+"");
        }
        super.setTabName(tabName);
    }

    public String getGroup_name() {
        return group_name;
    }

    public void setGroup_name(String group_name) {
        this.group_name = group_name;
    }

    public String getGroup_avater() {
        return group_avater;
    }

    public void setGroup_avater(String group_avater) {
        this.group_avater = group_avater;
    }

    public String getGroup_remark() {
        return group_remark;
    }

    public void setGroup_remark(String group_remark) {
        this.group_remark = group_remark;
    }

    public Integer getGroup_add_is_agree() {
        return group_add_is_agree;
    }

    public void setGroup_add_is_agree(Integer group_add_is_agree) {
        this.group_add_is_agree = group_add_is_agree;
    }

    public Integer getGroup_manager_user_id() {
        return group_manager_user_id;
    }

    public void setGroup_manager_user_id(Integer group_manager_user_id) {
        this.group_manager_user_id = group_manager_user_id;
    }

    public Integer getInitCreateid() {
        return initCreateid;
    }

    public void setInitCreateid(Integer initCreateid) {
        this.initCreateid = initCreateid;
    }

    public Integer getGroup_count() {
        return group_count;
    }

    public void setGroup_count(Integer group_count) {
        this.group_count = group_count;
    }

    public Integer getGroup_max_count() {
        return group_max_count;
    }

    public void setGroup_max_count(Integer group_max_count) {
        this.group_max_count = group_max_count;
    }

    public String getIntroduce() {
        return introduce;
    }

    public void setIntroduce(String introduce) {
        this.introduce = introduce;
    }

    public Date getGroup_ct() {
        return group_ct;
    }

    public void setGroup_ct(Date group_ct) {
        this.group_ct = group_ct;
    }

    public Date getUpdate_date() {
        return update_date;
    }

    public void setUpdate_date(Date update_date) {
        this.update_date = update_date;
    }

    public Integer getUpdate_user() {
        return update_user;
    }

    public void setUpdate_user(Integer update_user) {
        this.update_user = update_user;
    }

    public Integer getGroup_add_max_count() {
        return group_add_max_count;
    }

    public void setGroup_add_max_count(Integer group_add_max_count) {
        this.group_add_max_count = group_add_max_count;
    }

    public Integer getIs_valid() {
        return is_valid;
    }

    public void setIs_valid(Integer is_valid) {
        this.is_valid = is_valid;
    }

    public Integer getMessage_status() {
        return message_status;
    }

    public void setMessage_status(Integer message_status) {
        this.message_status = message_status;
    }

    public Integer getIsMember() {
        return isMember;
    }

    public void setIsMember(Integer isMember) {
        this.isMember = isMember;
    }

	public Date getRemark_update_time() {
		return remark_update_time;
	}

	public void setRemark_update_time(Date remark_update_time) {
		this.remark_update_time = remark_update_time;
	}
    
    
}
