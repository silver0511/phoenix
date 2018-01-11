/**
 * 
 */
package com.qbao.im.api.handler;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.qbao.im.api.entity.SliceInfo;
import com.qbao.im.api.service.friend.IFriendService;
import com.qbao.im.api.service.group.IGroupMemberService;
import com.qbao.im.api.service.group.IGroupService;

/**
 * @author andersen
 *
 */
@Service
public class SliceInfoHandler {	
	@Autowired
	private IFriendService friendService;
	@Autowired
	private IGroupService groupService;
	@Autowired
	private IGroupMemberService groupMemberService;
	
	public SliceInfo getSliceInfoBySingleKey(String table, Long value){
		switch(table.toLowerCase()){
			case "nim_friend":
				return friendService.getFriendDBAndTableName(value);
			case "nim_group":
				return groupService.getGroupDBAndTableName(value);
			case "nim_group_member":
				return groupMemberService.getGroupMemberDBAndTableName(value);
			default:
				return new SliceInfo();
		}
	}	
}
