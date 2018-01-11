package com.qbao.im.api.entity;

import com.alibaba.fastjson.JSON;
import com.qbao.im.api.DBEnum;

import java.util.Date;

/**
 * Created by tangxiaojun on 2017/3/28.
 * rebuild by lishuguang on 2017/10/31
 */
public class Friend extends BaseEntity {


    /**
	 * 
	 */
	private static final long serialVersionUID = -1811887983945816111L;

	private Long userId;

    private Long friendId;

    private Integer sourceType;

    private String remarkName;

    private Date createDate;

    private Integer status;

    private Integer isBlackList;

    private String friendMsg;

    public Friend() {
    }

    public Friend(Long userId, Long friendId, String remarkName) {
       this(userId, friendId, null, remarkName);
    }

    public Friend(Long userId, Long friendId, Integer sourceType, String remarkName) {
    	this.userId = userId;
    	this.friendId = friendId;
    	this.sourceType = sourceType;
    	this.remarkName = remarkName;
    }

    public Friend(Long userId, Long friendId) {
        this(userId, friendId, null, null);
    }

    @Override
    public String getTabName() {
        int modVal = (int) (this.userId%32);
        String modValStr = modVal+"";
        String tabName = null;
        if(modValStr.length()>1){
            tabName = DBEnum.FRIEND_SHIP.getValue().concat(modVal+"");
        }else {
            tabName = DBEnum.FRIEND_SHIP.getValue().concat("0"+modVal+"");
        }
        return tabName;
    }

    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
        int modVal = (int) (this.userId%32);
        String modValStr = modVal+"";
        String tabName = null;
        if(modValStr.length()>1){
            tabName = DBEnum.FRIEND_SHIP.getValue().concat(modVal+"");
        }else {
            tabName = DBEnum.FRIEND_SHIP.getValue().concat("0"+modVal+"");
        }
        super.setTabName(tabName);
    }

    public Integer getStatus() {
        return status;
    }

    public void setStatus(Integer status) {
        this.status = status;
    }

	public Long getFriendId() {
		return friendId;
	}

	public void setFriendId(Long friendId) {
		this.friendId = friendId;
	}

	public Integer getSourceType() {
		return sourceType;
	}

	public void setSourceType(Integer sourceType) {
		this.sourceType = sourceType;
	}

	public String getRemarkName() {
		return remarkName;
	}

	public void setRemarkName(String remarkName) {
		this.remarkName = remarkName;
	}

	public Date getCreateDate() {
		return createDate;
	}

	public void setCreateDate(Date createDate) {
		this.createDate = createDate;
	}

	public Integer getIsBlackList() {
		return isBlackList;
	}

	public void setIsBlackList(Integer isBlackList) {
		this.isBlackList = isBlackList;
	}

	public String getFriendMsg() {
		return friendMsg;
	}

	public void setFriendMsg(String friendMsg) {
		this.friendMsg = friendMsg;
	}
    
	@Override
    public String toString(){
    	return JSON.toJSONString(this);
    }
	

}
