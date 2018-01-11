package com.qbao.im.api.dto;

import java.io.Serializable;
import java.util.Date;

public class FriendDto implements Serializable{
	/**
	 * 
	 */
	private static final long serialVersionUID = -8978452070492942844L;
	private Long id;
	private String user_id;

    private String friend_id;

    private Integer source_type;

    private String remark_name;

    private Date create_date;

    private Integer status;

    private Integer is_black_list;

    private String friend_msg;

	public Long getId() {
		return id;
	}

	public void setId(Long id) {
		this.id = id;
	}

	public String getUser_id() {
		return user_id;
	}

	public void setUser_id(String user_id) {
		this.user_id = user_id;
	}

	public String getFriend_id() {
		return friend_id;
	}

	public void setFriend_id(String friend_id) {
		this.friend_id = friend_id;
	}

	public Integer getSource_type() {
		return source_type;
	}

	public void setSource_type(Integer source_type) {
		this.source_type = source_type;
	}

	public String getRemark_name() {
		return remark_name;
	}

	public void setRemark_name(String remark_name) {
		this.remark_name = remark_name;
	}

	public Date getCreate_date() {
		return create_date;
	}

	public void setCreate_date(Date create_date) {
		this.create_date = create_date;
	}

	public Integer getStatus() {
		return status;
	}

	public void setStatus(Integer status) {
		this.status = status;
	}

	public Integer getIs_black_list() {
		return is_black_list;
	}

	public void setIs_black_list(Integer is_black_list) {
		this.is_black_list = is_black_list;
	}

	public String getFriend_msg() {
		return friend_msg;
	}

	public void setFriend_msg(String friend_msg) {
		this.friend_msg = friend_msg;
	}
    
}
