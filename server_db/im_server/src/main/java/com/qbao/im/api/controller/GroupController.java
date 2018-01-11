package com.qbao.im.api.controller;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.qbao.im.api.constants.Constant;
import com.qbao.im.api.entity.GroupMember;
import com.qbao.im.api.handler.GroupHandler;
import com.qbao.im.api.msg.ClientMessage;
import com.qbao.im.api.msg.ErrorCode;
import org.apache.ibatis.annotations.Param;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;

import javax.ws.rs.*;
import javax.ws.rs.core.MediaType;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * Created by tangxiaojun on 2017/5/19.
 */
@Controller
@Path("/api/group")
public class GroupController {

    private Logger logger = LoggerFactory.getLogger(GroupController.class);

    @Autowired
    private GroupHandler groupHandler;

    /**
     * 创建群信息
     *
     * @param data
     * @return
     * @throws UnsupportedEncodingException
     */
    @POST
    @Path("/create")
    @Produces("application/json; charset=utf-8")
    @Encoded
    public ClientMessage create(String data) throws UnsupportedEncodingException {
        if(logger.isInfoEnabled()){
            logger.info("json:{}", data, "UTF-8");
        }
       
        return this.groupHandler.save(data);
    }

    /**
     * 获取用户群列表
     *
     * @param data
     * @return
     * @throws UnsupportedEncodingException
     */
    @POST
    @Path("/list")
    @Produces("application/json; charset=utf-8")
    @Encoded
    public ClientMessage getGroupList(String data) throws UnsupportedEncodingException {
        JSONObject jsonObject = JSONObject.parseObject(data);
        return this.groupHandler.getGroupList(jsonObject);
    }

    /**
     * 获取群信息
     *
     * @param data
     * @return
     */
    @POST
    @Path("/info")
    @Produces("application/json; charset=utf-8")
    @Encoded
    public ClientMessage groupInfo(String data) {
        JSONObject jsonObject = JSONObject.parseObject(data);
        return this.groupHandler.getGroupInfo(jsonObject);
    }

    /**
     * 群成员列表
     *
     * @param data
     * @return
     */
    @POST
    @Path("/memberList")
    @Produces("application/json; charset=utf-8")
    @Encoded
    public ClientMessage groupMemberList(String data) {
        JSONObject jsonObject = JSONObject.parseObject(data);
        return this.groupHandler.getMemberList(jsonObject);
    }

    /**
     * 群主转入
     *
     * @param data
     * @return
     */
    @POST
    @Path("/transfer")
    @Produces("application/json; charset=utf-8")
    @Encoded
    public ClientMessage transferMaster(String data) {
        JSONObject jsonObject = JSONObject.parseObject(data);
        return this.groupHandler.transfer(jsonObject);
    }

    /**
     * 加人/踢人
     * @param oper
     * @param data
     * @return
     */
    @POST
    @Path("/oper/{oper}")
    @Produces("application/json; charset=utf-8")
    @Encoded
    public ClientMessage operGroup(@PathParam("oper") String oper, String data) {
        JSONArray array = null;
        ClientMessage clientMessage = null;
        JSONObject jsonObject = JSONObject.parseObject(data);
        List<GroupMember> groupMembers = new ArrayList<>();
        Long groupId = jsonObject.getLong("group_id");
        ClientMessage isExsistGroup = this.groupHandler.getGroupInfo(jsonObject);
        if(isExsistGroup.getResponseCode().intValue() == ErrorCode.RET_GROUP_ID_INVALID){
            return isExsistGroup;
        }
        //获取需要加人或踢人列表
        if (jsonObject.containsKey("list_user_change")) {
            array = jsonObject.getJSONArray("list_user_change");
            for (int i = 0; i < array.size(); i++) {
                //构建群成员对象
                GroupMember gm = new GroupMember();
                Integer userId = JSONObject.parseObject(array.get(i).toString()).getInteger("user_id");
                gm.setGroup_id(groupId);
                gm.setMember_type(0);
                gm.setUser_id(userId);
                gm.setMember_nickname(JSONObject.parseObject(array.get(i).toString()).getString("user_nick_name"));
                gm.setMember_joindate(new Date());
                groupMembers.add(gm);
            }
        }
        if(oper.equals(Constant.GROUP_OFFLINE_CHAT_ADD_USER)){
            clientMessage = this.groupHandler.plusPerson(jsonObject,groupMembers, data);
        }

        if(oper.equals(Constant.GROUP_OFFLINE_CHAT_KICK_USER)){
            clientMessage = this.groupHandler.killPerson(jsonObject,groupMembers, data);
        }

        return clientMessage;
    }

    @POST
    @Path("/modify/{oper}")
    @Encoded
    @Produces("application/json; charset=utf-8")
    public ClientMessage modify(@PathParam("oper") String oper,String data){
        JSONObject jsonObject = JSONObject.parseObject(data);
        ClientMessage clientMessage = null;
        //需群主同意
        if(oper.equals(Constant.GROUP_OFFLINE_CHAT_ENTER_AGREE)){
            clientMessage = this.groupHandler.masterAgree(jsonObject);
        }

        //默认进群
        if(oper.equals(Constant.GROUP_OFFLINE_CHAT_ENTER_DEFAULT)){
            clientMessage = this.groupHandler.enterGroupDefault(jsonObject);
        }

        //修改群名称
        if(oper.equals(Constant.GROUP_OFFLINE_CHAT_MODIFY_GROUP_NAME)){
            clientMessage = this.groupHandler.modifyGroupName(jsonObject);
        }

        //修改群备注
        if(oper.equals(Constant.GROUP_OFFLINE_CHAT_MODIFY_GROUP_REMARK)){
            clientMessage = this.groupHandler.modifyGroupRemak(jsonObject);
        }

        //修改群中用户昵称
        if(oper.equals(Constant.GROUP_OFFLINE_CHAT_MODIFY_GROUP_USER_NAME)){
            clientMessage = this.groupHandler.modifyUserNickName(jsonObject);
        }
        return clientMessage;
    }

    /**
     * 查询群公告
     * @param data
     * @return
     */
    @POST
    @Path("/query/remark")
    @Encoded
    @Produces("application/json; charset=utf-8")
    public ClientMessage query(String data){
        JSONObject jsonObject = JSONObject.parseObject(data);
        return this.groupHandler.queryGroupRemark(jsonObject);
    }

    /**
     * 获取群列表编号
     * @param data
     * @return
     */
    @POST
    @Path("/list/getGroupId")
    @Encoded
    @Produces("application/json; charset=utf-8")
    public ClientMessage queryGroupList(String data){
        Long userId = JSONObject.parseObject(data).getLong("user_id");
        return this.groupHandler.queryGroupListGroupId(userId);
    }

    /**
     * 修改群接受消息状态
     * @param data
     * @return
     */
    @POST
    @Path("/modify/rcvType")
    @Encoded
    @Produces("application/json; charset=utf-8")
    public ClientMessage modifyGroupMemberRcvType(String data){
        JSONObject jsonObject = JSONObject.parseObject(data);
        return this.groupHandler.modifyRcvType(jsonObject);
    }

    /**
     * 获取用户单个群备注信息
     * @param data
     * @return
     */
    @POST
    @Path("/user/remark")
    @Encoded
    @Produces("application/json; charset=utf-8")
    public ClientMessage getUserGroupRemark(String data){
        JSONObject jsonObject = JSONObject.parseObject(data);
        return this.groupHandler.getUserGroupRemark(jsonObject.getLong("group_id"),jsonObject.getLong("userid"));
    }

    @POST
    @Path("/modify/isAddressBook")
    @Encoded
    @Produces("application/json; charset=utf-8")
    public ClientMessage modifyIsAddressBook(String data){
        JSONObject jsonObject = JSONObject.parseObject(data);
        return this.groupHandler.modifyIsAddressBook(jsonObject);
    }

    @POST
    @Path("/info/list")
    @Encoded
    @Produces("application/json; charset=utf-8")
    public ClientMessage getGroupInfoList(String data){
        JSONObject jsonObject = JSONObject.parseObject(data);
        return this.groupHandler.getGroupInfoList(jsonObject);
    }
}
