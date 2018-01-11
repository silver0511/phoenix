package com.qbao.im.api.controller;

import java.util.ArrayList;
import java.util.List;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;

import com.mysql.jdbc.StringUtils;
import com.qbao.im.api.config.FriendConfiguration;
import com.qbao.im.api.constants.FriendStatusEnums;
import com.qbao.im.api.dto.FriendDto;
import com.qbao.im.api.entity.Friend;
import com.qbao.im.api.handler.FriendHandler;
import com.qbao.im.api.msg.ClientMessage;
import com.qbao.im.api.msg.ErrorCode;
import com.qbao.im.api.service.impl.FriendServiceImpl;

/**
 * Created by tangxiaojun on 2017/5/15.
 */
@Controller
@Path("/api/friend")
public class FriendController {

    private Logger logger = LoggerFactory.getLogger(FriendController.class);

    @Autowired
    private FriendServiceImpl friendServiceImpl;

    @Autowired
    private FriendHandler friendHandler;

    @Autowired
    private FriendConfiguration friendConfiguration;
    
    /**
     * 获取用户好友列表
     * @param userId 用户id
     * @param pageIndex 起始记录位置，0开始
     * @param pageSize 返回记录最大数
     * @return
     */
    @GET
    @Path("/get")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage getFriends(@QueryParam("user_id") Long userId,
                                    @QueryParam("offset") Integer pageIndex, @QueryParam("max_cnt") Integer pageSize) {
        logger.info("friend/get param,user_id:{}", userId);
        List<Friend> friendList = this.friendHandler.getFriend(userId,pageIndex,pageSize,FriendStatusEnums.INVALID.getValue());
        //转化为dto
        List<FriendDto> dtoList = new ArrayList<FriendDto>();
        if (friendList != null && friendList.isEmpty() == false){
        	for (Friend f : friendList){
        		FriendDto dto = new FriendDto();
        		dto.setId(f.getId());
        		dto.setCreate_date(f.getCreateDate());
        		dto.setFriend_id(String.valueOf(f.getFriendId()));
        		dto.setFriend_msg(f.getFriendMsg());
        		dto.setIs_black_list(f.getIsBlackList());
        		dto.setRemark_name(f.getRemarkName());
        		dto.setSource_type(f.getSourceType());
        		dto.setStatus(f.getStatus());
        		dto.setUser_id(String.valueOf(f.getUserId()));
        		dtoList.add(dto);
        	}
        }
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(dtoList).build();
    }
    
    /**
     * 保存好友申请信息
     * @param userId
     * @param friendId
     * @param sourceType
     * @param remarkName
     * @param friendMsg
     * @return
     */
    @GET
    @Path("/save")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage save(@QueryParam("user_id") Long userId, @QueryParam("friend_id") Long friendId, @QueryParam("source_type") Integer sourceType,
                              @QueryParam("remark_name") String remarkName,@QueryParam("friend_msg") String friendMsg) {
        ClientMessage result = friendHandler.applyFriend(userId, friendId, sourceType, remarkName, friendMsg);
        if (logger.isInfoEnabled()){
        	logger.info("[SaveFriend]:  user_id:{}, friend_id:{}, resultCode:{}", userId, friendId, result.getResponseCode());
        }
        return result;
    }
    
    /**
     * 同意好友申请
     * @param userId
     * @param friendId
     * @param sourceType
     * @return
     */
    @GET
    @Path("/agree")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage agreeApply(@QueryParam("user_id") Long userId, @QueryParam("friend_id") Long friendId,@QueryParam("source_type") Integer sourceType){
    	 ClientMessage result = friendHandler.agree(userId, friendId, sourceType);
    	 if (logger.isInfoEnabled()){
         	logger.info("[AgreeFriend]: user_id:{}, friend_id:{}, resultCode:{}", userId, friendId, result.getResponseCode());
         }
        return result;
    }

    @GET
    @Path("/refuse")
    @Produces(MediaType.APPLICATION_JSON)
    @Deprecated
    public ClientMessage refuseFriend(@QueryParam("user_id") Long userId, @QueryParam("friendid") Long friendId){
    	return null;
    }
    
    /**
     * 删除好友
     * @param userId
     * @param friendId
     * @return
     */
    @GET
    @Path("/del")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage delFriend(@QueryParam("user_id") Long userId, @QueryParam("friend_id") Long friendId) {
        boolean result = this.friendHandler.del(userId, friendId);
        logger.info("[DelFriend]: return {}. userId:{}, friendId:{}", result, userId, friendId);
        if (result) {
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).build();
        }
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_DEL_ERROR).build();
    }
    
    /**
     * 修改好友备注名称
     * @param userId
     * @param friendId
     * @param remarkName
     * @return
     */
    @GET
    @Path("/update")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage modifyRemark(@QueryParam("user_id") Long userId, @QueryParam("friend_id") Long friendId, @QueryParam("remark_name") String remarkName) {
    	if (StringUtils.isNullOrEmpty(remarkName)){
    		remarkName = "";
    	}
    	if(remarkName.length()> 24){
    		return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_REMARK_ERROR).build();
    	}
        Friend friend = new Friend(userId, friendId, remarkName);
        boolean result = this.friendHandler.modifyRemark(friend);
        logger.info("[ModifyRemark]: return {}. userId:{}, friendId:{}", result, userId, friendId);
        if (result) {
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).build();
        }
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_MODIFY_ERROR).build();
    }
    
    /**
     * 处理 发送好友宴请后，对方直接删除申请信息
     * @param user_id
     * @param friendId
     * @return
     */
    @GET
    @Path("/update/status")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage updateStatus(@QueryParam("user_id")Long userId,@QueryParam("friend_id") Long friendId){
        boolean result = this.friendHandler.deleteApply(userId, friendId);
        logger.info("[DeleteApply]: return {}. userId:{}, friendId:{}", result, userId, friendId);
        if(result){
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).build();
        }
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_SETTING_STATUS_ERROR).build();
    }
    
    /**
     * 设置黑名单
     * @param userId
     * @param friendId
     * @param blackType 0：取消 拉黑  1：设置拉黑
     * @return 
     */
    @GET
    @Path("/settingBlacklist")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage settingBlackList(@QueryParam("user_id")Long userId,@QueryParam("friend_id") Long  friendId,@QueryParam("black_type")Integer blackType){
    	boolean result = this.friendHandler.settingBlack(userId, friendId, blackType);
    	logger.info("[SettingBlack]: return {}. userId:{}, friendId:{}", result, userId, friendId);
    	if(result){
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).build();
        }
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_MODIFY_ERROR).build();
    }
}
