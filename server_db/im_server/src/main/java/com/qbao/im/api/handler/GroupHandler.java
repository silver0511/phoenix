package com.qbao.im.api.handler;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.dao.DuplicateKeyException;
import org.springframework.stereotype.Service;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.qbao.im.api.analysis.GroupMemberRequest;
import com.qbao.im.api.analysis.GroupRequest;
import com.qbao.im.api.constants.Constant;
import com.qbao.im.api.entity.Group;
import com.qbao.im.api.entity.GroupMember;
import com.qbao.im.api.msg.ClientMessage;
import com.qbao.im.api.msg.ErrorCode;
import com.qbao.im.api.service.impl.GroupMemberServiceImpl;
import com.qbao.im.api.service.impl.GroupServiceImpl;
import com.qbao.im.api.service.impl.MemberGroupListServiceImpl;
import com.qbao.im.api.utils.GenerateUtil;

import redis.clients.jedis.JedisCluster;

/**
 * Created by tangxiaojun on 2017/5/23.
 */
@Service
public class GroupHandler {

    private Logger logger = LoggerFactory.getLogger(GroupHandler.class);

    @Autowired
    private GroupServiceImpl groupServiceImpl;

    @Autowired
    private GroupMemberServiceImpl groupMemberServiceImpl;

    @Autowired
    private MemberGroupListServiceImpl memberGroupListService;

//    @Autowired
//    private RedisUtil redisUtil;

    @Autowired
    JedisCluster jedisCluster;
    
    //判断重复发包时间间隔
    @Value("${group.repeat.oper.interval}")
    private Integer repeatOperInterval;

    /**
     * 创建群
     *
     * @param jsonObject
     * @return
     */
    public ClientMessage save(String data) {
        Long groupId = 0l;
        Long userId = null;
        ClientMessage clientMessage = null;
        String user_id = null;
        try {
        	JSONObject jsonObject = JSONObject.parseObject(data);
        	user_id = jsonObject.getString("user_id");
        	userId = Long.parseLong(user_id);
        	Long sessionId = jsonObject.getLong("pack_session_id");
        	String oper = "save_group";
        	
        	//判重
        	if (checkRepeatOper(Long.parseLong(user_id), sessionId, oper, data).intValue() != 1){
        		return new ClientMessage.Builder().user_id(user_id).responseCode(ErrorCode.RET_GROUP_REPEATED_PACK).build();
        	}
            //通过redis单线程获取自增groupid
            /*String groupKey = jedisCluster.hget(Constant.REDIS_CREATE_GROUP_KEY, Constant.REDIS_CREATE_GROUP_FIELD_KEY);
            if (groupKey == null) {
                groupId = jedisCluster.hincrBy(Constant.REDIS_CREATE_GROUP_KEY, Constant.REDIS_CREATE_GROUP_FIELD_KEY, Constant.GROUP_INCREBY_ID);
            } else {
                groupId = jedisCluster.hincrBy(Constant.REDIS_CREATE_GROUP_KEY, Constant.REDIS_CREATE_GROUP_FIELD_KEY, Constant.GROUP_AUTOMENT_ID);
            }*/
        	groupId = GenerateUtil.generateKey();
            if(logger.isInfoEnabled()){
                logger.info("user_id:{} create group_id:{}",user_id, groupId);
            }
            //解析建群加人列表
            List<GroupMember> memberList = GroupMemberRequest.getMemberList(jsonObject, groupId);
            if (memberList.isEmpty()){
            	return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_CREATE_USER_LIST_EMPTY).build();
            }
            //检查是否超过单次拉人上限
            Group group = GroupRequest.getGroup(jsonObject, memberList.size(), groupId);
            if(memberList.size() > group.getGroup_add_max_count()){
                return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_MAX_LIMIT_ERROR).build();
            }

            try {
                saveGroupInfoInDB(group, memberList);
            } catch (DuplicateKeyException dke) { //主键冲突时重试一次
            	logger.warn("duplicate key {} when create group", groupId);
                groupId = GenerateUtil.generateKey();
                logger.warn("user new Key {} when create group", groupId);
                rebuidNeedSaveGroupInfo(group, memberList, groupId);
                try{
                	saveGroupInfoInDB(group, memberList);
                }catch(Exception e){
                	e.printStackTrace();
                    logger.error("group save error!  groupId :{} ,e:{}", groupId, e);
                    return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_ADD_ERROR).build();
                }
            } catch (Exception e){
            	e.printStackTrace();
                logger.error("group save error!  groupId :{} ,e:{}", groupId, e);
                return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_ADD_ERROR).build();
            }
            
            //确认删除redis中原有群成员信息（性能问题时可删除）
            List<String> strGroupMember = new ArrayList<>();
            Map<String , String> redisMap = this.jedisCluster.hgetAll(Constant.REDIS_GROUP_MEMBER.concat(groupId+""));
            if(redisMap!=null){
                this.jedisCluster.del(Constant.REDIS_GROUP_MEMBER.concat(groupId+""));
            }

            //redis添加群成员信息
            Map<String, String> memberMap = new HashMap<String, String>();
            for (GroupMember groupMember : memberList) {
                memberMap.put(groupMember.getUser_id()+"","0");
            }
            this.jedisCluster.hmset(Constant.REDIS_GROUP_MEMBER.concat(groupId+""),memberMap);

            //redis添加群信息
            Set<String> redisSet = this.jedisCluster.smembers(Constant.REDIS_GROUP_INFO.concat(groupId + ""));
            if(!redisSet.isEmpty()){
                this.jedisCluster.del(Constant.REDIS_GROUP_INFO.concat(""+groupId));
            }
            this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat(groupId + ""), JSONObject.toJSONString(group));
            
            //redis添加用户群信息
            Map<String , Object> properMap = new HashMap<String , Object>();
            properMap.put("rcv_type",0);
            properMap.put("save_type",0);
            properMap.put("group_id",groupId);
            for (int i = 0; i < memberList.size(); i++) {
                //添加群列表
                jedisCluster.zadd(Constant.REDIS_USER_GROUP_LIST.concat(memberList.get(i).getUser_id() + ""), groupId, JSONObject.toJSONString(properMap));
            }
            
            Map<String, Object> returnMap = new ConcurrentHashMap<>();
            returnMap.put("group_id", groupId);
            returnMap.put("group_name", group.getGroup_name());
            returnMap.put("group_remark",group.getGroup_remark());
            returnMap.put("group_ct", jsonObject.getString("group_ct"));
            returnMap.put("group_manager_user_id", group.getGroup_manager_user_id());
            returnMap.put("group_count", group.getGroup_count());
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(returnMap).build();
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("error:{},groupId:{},user_id:{}", e,groupId,user_id);
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_ADD_ERROR).build();
        }
    }
    
    /**
     * 根据新的groupId重新组合要存库的群信息
     * @param group
     * @param gms
     * @param groupId
     */
    private void rebuidNeedSaveGroupInfo(Group group, List<GroupMember> gms, Long groupId){
    	group.setGroup_id(groupId);
    	for (GroupMember gm : gms){
    		gm.setGroup_id(groupId);
    	}
    }
    
    /**
     * 把群信息存到数据库
     * @param group
     * @param gms
     */
    private void saveGroupInfoInDB(Group group, List<GroupMember> gms){
    	 //添加群信息
        this.groupServiceImpl.saveGroup(group.getGroup_id(), group);
        //批量添加群成员
        Map<String, Object> param = new HashMap<String, Object>();
        param.put("list", gms);
        param.put("tabName",gms.get(0).getTabName());
        this.groupMemberServiceImpl.batchGroupMember(group.getGroup_id(), param);
    }

    /**
     * 获取用户群列表
     *
     * @param jsonObject
     * @return
     */
    public ClientMessage getGroupList(JSONObject jsonObject) {
        ClientMessage clientMessage = null;
        String user_id = jsonObject.getString("user_id");
        try {
            Integer pageIndex = jsonObject.getInteger("group_list_index");
            Integer pageSize = jsonObject.getInteger("group_list_page_number");
            Set<String> setList = this.jedisCluster.zrange(Constant.REDIS_USER_GROUP_LIST.concat(user_id), pageIndex, pageIndex+pageSize-1);
            List<Group> groupList = new ArrayList<>();
            List<Integer> rcvType = new ArrayList<>();
            for (String string : setList) {
                Map<String , Object> map = JSONObject.parseObject(string);
                Long groupId = Long.parseLong(map.get("group_id").toString());
                rcvType.add(Integer.valueOf(map.get("rcv_type").toString()));
                Group group = JSONObject.parseObject(this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(groupId + "")), Group.class);
                if(group == null){
                    //删除踢人的群列表
                    this.jedisCluster.zremrangeByScore(Constant.REDIS_USER_GROUP_LIST.concat(user_id), groupId, groupId);
                }else {
                    groupList.add(group);
                }
            }

            Map<String , Object> returnMap = new HashMap<String,Object>();
            if(!groupList.isEmpty()){
                for(int i =0;i<groupList.size();i++){
                    groupList.get(i).setMessage_status(rcvType.get(i));
                }
            }
            returnMap.put("group_list",groupList);
            clientMessage = new ClientMessage.Builder().responseCode(ErrorCode.RET_SUCCESS).data(returnMap).build();
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("user_id:{},,e:{}", user_id,e);
            clientMessage = new ClientMessage.Builder().responseCode(ErrorCode.RET_GROUP_MEMBER_LIST_ERROR).build();
        }
        return clientMessage;
    }

    public ClientMessage getGroupInfo(JSONObject jsonObject) {
        Long groupId = jsonObject.getLong("group_id");
        String user_id = jsonObject.getString("user_id");
        Group group = this.groupServiceImpl.getGroupInfoByGroupId(groupId);
        //群信息是否存在
        if (group == null) {
            return new ClientMessage.Builder().responseCode(ErrorCode.RET_GROUP_ID_INVALID).build();
        }
        //是否是群成员
        Boolean isMember = this.groupMemberServiceImpl.isExsistGroup(groupId, jsonObject.getLong("user_id"));
        group.setIsMember(isMember ? Integer.valueOf(1) : Integer.valueOf(0));
        return new ClientMessage.Builder().responseCode(ErrorCode.RET_SUCCESS).data(group).build();
    }

    public ClientMessage getMemberList(JSONObject jsonObject) {
        Long groupId = jsonObject.getLong("group_id");
        String user_id = jsonObject.getString("user_id");
        Long userId = Long.parseLong(user_id);
        Integer pageIndex = jsonObject.getInteger("group_member_index");
        Integer pageSize = jsonObject.getInteger("group_page_span");
        Map<String, Object> map = new HashMap<String, Object>();
        ClientMessage clientMessage = null;
        try {
            List<GroupMember> memberList = this.groupMemberServiceImpl.getMemberList(groupId, pageIndex, pageSize);
            if (!memberList.isEmpty()) {
                List<Map<String , Object>> responsesList = new ArrayList<>();
                for (int i = 0; i < memberList.size(); i++) {
                    GroupMember gm = memberList.get(i);
                    Map<String , Object> m = new HashMap<>();
                    m.put("user_id",gm.getUser_id());
                    m.put("user_group_index",gm.getOrderid().intValue());
                    m.put("user_nick_name",gm.getMember_nickname());
                    responsesList.add(m);
                }
                map.put("memberList", responsesList);
            } else {
                map.put("memberList", memberList);
            }
            clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(map).build();
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("groupId:{},user_id:{},e",groupId,user_id,e);
            clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_MEMBER_LIST_ERROR).build();
        }
        return clientMessage;
    }

    /**
     * 群主转让
     *
     * @param jsonObject
     * @return
     */
    public ClientMessage transfer(JSONObject jsonObject) {
        ClientMessage clientMessage = null;
        String user_id = jsonObject.getString("user_id");
        Long groupId = jsonObject.getLong("group_id");
        try {
            Long changeuser_id = JSONObject.parseObject(jsonObject.getJSONArray("list_user_change").get(0).toString()).getLong("user_id");
            Group group = new Group();
            group.setGroup_id(groupId);
            group.setGroup_manager_user_id(changeuser_id.intValue());
            group.setUpdate_date(new Date());
            //修改群信息
            this.groupServiceImpl.update(groupId, group);
            GroupMember groupMember = new GroupMember();
            groupMember.setGroup_id(groupId);
            groupMember.setUser_id(changeuser_id.intValue());
            groupMember.setUpdate_date(new Date());
            groupMember.setMember_type(2);
            GroupMember memberType = new GroupMember();
            memberType.setGroup_id(groupId);
            this.groupMemberServiceImpl.updateMemberType(groupId, memberType);
            this.groupMemberServiceImpl.update(groupId, groupMember);
            //修改群信息
            Group redisGroup = JSONObject.parseObject(this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(groupId + "")), Group.class);
            this.jedisCluster.srem(Constant.REDIS_GROUP_INFO.concat(groupId + ""), JSONObject.toJSONString(redisGroup));
            redisGroup.setGroup_manager_user_id(changeuser_id.intValue());
            this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat(groupId + ""), JSONObject.toJSONString(redisGroup));
            clientMessage = new ClientMessage.Builder().responseCode(ErrorCode.RET_SUCCESS).data(jsonObject).build();
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("groupid:{},user_id:{},e",groupId,user_id,e);
            clientMessage = new ClientMessage.Builder().responseCode(ErrorCode.RET_GROUP_MEMBER_CHANGE_ERROR).build();
        }
        return clientMessage;
    }

    /**
     * 加人
     *
     * @param jsonObject
     * @param groupMemberList
     * @return
     */
    public ClientMessage plusPerson(JSONObject jsonObject, List<GroupMember> groupMemberList, String data) {
        ClientMessage clientMessage = null;
        Long groupId = jsonObject.getLong("group_id");
        Long userId = jsonObject.getLong("user_id");       
        if (logger.isInfoEnabled()){
        	logger.info("group(id={}) user(id={}) plusPerson {}", groupId, userId, groupMemberList!=null?groupMemberList.size():0);
        }
        try {
        	//首先判重
        	Long sessionId = jsonObject.getLong("pack_session_id");
        	String oper = "group_add_member";
        	if (checkRepeatOper(userId, sessionId, oper, data) != 1){
        		return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_REPEATED_PACK).build();
        	}
        	
        	//获取群信息
        	Group group = getGroupInfoFromCacheAndDB(groupId);
        	if (group == null){
        		return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_ID_INVALID).build();
        	}
        	//如果加入列表为空，直接返回
        	if (groupMemberList == null || groupMemberList.isEmpty()){
        		new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(buildReturnInfoFromGroup(group)).build();
        	}
        	//如果不是群主加群且群不是默认进群，直接返回
        	if(group.getGroup_add_is_agree().intValue() != 0 && group.getGroup_manager_user_id().intValue() != userId.intValue()){
        		new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(buildReturnInfoFromGroup(group)).build();
        	}
        	
        	//添加人数大于单次添加人数
        	if(groupMemberList.size() > group.getGroup_add_max_count().intValue()){
        		return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_SINGLE_LIMIT_ERROR).build();
        	}
        	
        	//添加群成员
        	//1. 过滤已有群成员
        	//1.1 获取群已有成员
        	Map<String, String> existMembers = jedisCluster.hgetAll(Constant.REDIS_GROUP_MEMBER.concat(groupId+""));
        	List<GroupMember> needAddMember = new ArrayList<GroupMember>();
        	List<Integer> existMemberIds = new ArrayList<Integer>();
        	if (existMembers != null && existMembers.isEmpty() == false){
        		for (GroupMember gm : groupMemberList){
        			if (existMembers.keySet().contains(gm.getUser_id()+"") == false){
        				needAddMember.add(gm);
        			}else{
        				existMemberIds.add(gm.getUser_id());
        			}
        		}
        	}else{        		
        		needAddMember = groupMemberList;
        	}
        	//当前群人数+添加人数大于群总人数
        	if(group.getGroup_count()+needAddMember.size() > group.getGroup_max_count()){
        		return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_MAX_LIMIT_ERROR).build();
        	}
        	//1.2如果都已是群成员，则直接返回
        	if (needAddMember.isEmpty()){
        		Map<String, Object> result = buildReturnInfoFromGroup(group);
        		result.put("duplicate_member_ids", existMemberIds);
        		return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(result).build();
        	}
        	
        	//2. 存库
        	this.groupMemberServiceImpl.add(groupId, group, needAddMember, userId);
        	
        	//3. 处理redis
            //3.1 添加群成员
            Map<String , String> hashMap = new HashMap<>();
            for (GroupMember groupMember : needAddMember) {
                hashMap.put(groupMember.getUser_id()+"","0");
            }
            this.jedisCluster.hmset(Constant.REDIS_GROUP_MEMBER.concat(groupId+""),hashMap);
            
            //3.2 添加用户群列表
            Map<String , Object> properMap = new HashMap<String , Object>();
            properMap.put("rcv_type",0);
            properMap.put("save_type",0);
            properMap.put("group_id",groupId);
            for (int i = 0; i < needAddMember.size(); i++) {
                //添加用户群列表
                this.jedisCluster.zadd(Constant.REDIS_USER_GROUP_LIST.concat(needAddMember.get(i).getUser_id().toString()), groupId, JSONObject.toJSONString(properMap));
            }
            
            //3.3 更新群信息
            String resultGroup = this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(groupId+""));
            if(resultGroup != null){
                Group redisGroup = JSONObject.parseObject(resultGroup,Group.class);
                redisGroup.setGroup_count(group.getGroup_count());
                this.jedisCluster.srem(Constant.REDIS_GROUP_INFO.concat(groupId+""),resultGroup);
                this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat(""+groupId),JSONObject.toJSONString(redisGroup));
            }else {
                this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat(""+groupId),JSONObject.toJSONString(group));
            }

            Map<String, Object> result = buildReturnInfoFromGroup(group);
            result.put("duplicate_member_ids", existMemberIds);
            clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(result).build();
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("groupid :{},user_id:{},e:{}",groupId,userId,e);
            clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_OPERATE_INFO_ERROR).build();
        }
        return clientMessage;
    }
    
    /**
     * 根据群信息组合加人返回map
     * @param group
     * @return
     */
    private Map<String, Object> buildReturnInfoFromGroup(Group group){
    	Map<String, Object> result = new HashMap<String, Object>();
    	result.put("group_name", group.getGroup_name());
    	result.put("group_manager_user_id", group.getGroup_manager_user_id());
    	result.put("group_count", group.getGroup_count());
    	result.put("group_id", group.getGroup_id());
    	result.put("group_max_count",group.getGroup_max_count());
    	result.put("group_add_max_count",group.getGroup_add_max_count());
    	result.put("group_add_is_agree", group.getGroup_add_is_agree());
    	return result;
    }

    /**
     * 踢人
     *
     * @param jsonObject
     * @param groupMemberList
     * @return
     */
    public ClientMessage killPerson(JSONObject jsonObject, List<GroupMember> groupMemberList, String data) {
        ClientMessage clientMessage = null;
        Long groupId = jsonObject.getLong("group_id");
        Long userId = jsonObject.getLong("user_id");
        try {
        	//首先判重
        	Long sessionId = jsonObject.getLong("pack_session_id");
        	String oper = "group_remove_member";
        	if (checkRepeatOper(userId, sessionId, oper, data) != 1){
        		return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_REPEATED_PACK).build();
        	}
        	
        	//获取群信息
        	Group group = getGroupInfoFromCacheAndDB(groupId);
        	if (group == null){
        		return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_ID_INVALID).build();
        	}
            //数据库删除
            Map<String, Object> map = this.groupMemberServiceImpl.kick(groupId, group, groupMemberList, userId);
            group = this.groupServiceImpl.getGroupInfoByGroupId(groupId);

            if (Integer.parseInt(map.get("success").toString()) == 1) {
                //redis 群成员踢人,null群不存在直接删除群成员
                if(group == null){
                    this.jedisCluster.del(Constant.REDIS_GROUP_MEMBER.concat(groupId+""));
                    this.jedisCluster.del(Constant.REDIS_GROUP_INFO.concat(groupId + ""));
                }else {
                    //redis踢群成员
                    for(GroupMember groupMember:groupMemberList){
                        this.jedisCluster.hdel(Constant.REDIS_GROUP_MEMBER.concat(groupId+""),groupMember.getUser_id()+"");
                    }
                    //删除踢人的群列表
                    for (GroupMember gm : groupMemberList) {
                        this.jedisCluster.zremrangeByScore(Constant.REDIS_USER_GROUP_LIST.concat(gm.getUser_id().toString()), gm.getGroup_id(), gm.getGroup_id());
                    }
                    //修改群人数
                    String resultGroup = this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(groupId + ""));
                    if(resultGroup != null){
                        Group redisGroup = JSONObject.parseObject(resultGroup, Group.class);
                        redisGroup.setGroup_count(group.getGroup_count());
                        redisGroup.setGroup_manager_user_id(group.getGroup_manager_user_id());
                        this.jedisCluster.srem(Constant.REDIS_GROUP_INFO.concat(groupId + ""), resultGroup);
                        this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(redisGroup));
                    }else {
                        this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(group));
                    }
                }

                jsonObject.put("group_add_is_agree", 0);
                jsonObject.put("group_id", groupId);
                if (group == null) {
                    jsonObject.put("group_manager_user_id", 0);
                    jsonObject.put("group_count", 0);
                } else {
                    jsonObject.put("group_manager_user_id", group.getGroup_manager_user_id());
                    jsonObject.put("group_count", group.getGroup_count());
                }
                clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId))
                        .responseCode(ErrorCode.RET_SUCCESS).data(jsonObject).build();
            }
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("groupid:{},user_id:{},e:{}",groupId,userId,e);
            clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_OPERATE_INFO_ERROR).build();
        }
        return clientMessage;
    }

    /**
     * 群主同意
     *
     * @param jsonObject
     * @return
     */
    public ClientMessage masterAgree(JSONObject jsonObject) {
        ClientMessage clientMessage = null;
        Long groupId = jsonObject.getLong("group_id");
        Long userId = jsonObject.getLong("user_id");
        try {
            Group group = this.groupServiceImpl.getGroupInfoByGroupId(groupId);
            group.setGroup_add_is_agree(1);
            group.setUpdate_date(new Date());
            //修改群信息
            this.groupServiceImpl.update(groupId, group);
            //修改redis群信息
            String resultGroup = this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(groupId + ""));
            if(resultGroup != null){
                Group redisGroup = JSONObject.parseObject(resultGroup, Group.class);
                redisGroup.setGroup_add_is_agree(1);
                redisGroup.setInitCreateid(jsonObject.getInteger("user_id"));
                this.jedisCluster.srem(Constant.REDIS_GROUP_INFO.concat(groupId + ""), resultGroup);
                this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(redisGroup));
            }else {
                this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(group));
            }
            jsonObject.put("group_add_is_agree", 1);
            jsonObject.put("group_name", group.getGroup_name());
            jsonObject.put("group_manager_user_id", group.getGroup_manager_user_id());
            jsonObject.put("group_count", group.getGroup_count());
            jsonObject.put("group_id",groupId);
            clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(jsonObject).build();
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("groupid:{},user_id:{},e:{}",groupId,userId,e);
            clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_OPERATE_INFO_ERROR).build();
        }
        return clientMessage;
    }

    /**
     * 默认进群
     *
     * @param jsonObject
     * @return
     */
    public ClientMessage enterGroupDefault(JSONObject jsonObject) {
        ClientMessage clientMessage = null;
        Long groupId = jsonObject.getLong("group_id");
        Long userId = jsonObject.getLong("user_id");
        String cometid = jsonObject.getString("cometid");
        Integer platform = jsonObject.getInteger("platform");
        Integer sessionId = jsonObject.getInteger("sessionid");
        try {
            Group group = this.groupServiceImpl.getGroupInfoByGroupId(groupId);
            group.setGroup_add_is_agree(0);
            group.setUpdate_date(new Date());
            String resultGroup = this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(groupId + ""));
            if(resultGroup != null){
                Group redisGroup = JSONObject.parseObject(resultGroup, Group.class);
                redisGroup.setGroup_add_is_agree(0);
                redisGroup.setInitCreateid(userId.intValue());
                this.jedisCluster.srem(Constant.REDIS_GROUP_INFO.concat(groupId + ""), resultGroup);
                this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(redisGroup));
            }else {
                this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(group));
            }
            //修改群信息
            this.groupServiceImpl.update(groupId, group);
            jsonObject.put("group_add_is_agree", 0);
            jsonObject.put("group_name", group.getGroup_name());
            jsonObject.put("group_manager_user_id", group.getGroup_manager_user_id());
            jsonObject.put("group_count", group.getGroup_count());
            jsonObject.put("group_id",groupId);
            clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(jsonObject).build();
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("groupid:{},user_id:{},e:{}",groupId,userId,e);
            clientMessage = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_OPERATE_INFO_ERROR).build();
        }
        return clientMessage;
    }

    /**
     * 修改群名称
     *
     * @param jsonObject
     * @return
     */
    public ClientMessage modifyGroupName(JSONObject jsonObject) {
        ClientMessage clientMessage = null;
        String groupName = jsonObject.getString("group_modify_content");
        Long userId = jsonObject.getLong("user_id");
        Long groupId = jsonObject.getLong("group_id");
        if(groupName.length() > Constant.GROUP_NAME_LEN){
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_MODIFY_NAME_ERROR).build();
        }

        Group group = this.groupServiceImpl.getGroupInfoByGroupId(groupId);
        group.setGroup_name(groupName);
        group.setUpdate_date(new Date());
        //修改群信息
        this.groupServiceImpl.update(groupId, group);
        //修改群列表信息
        String resultGroup = this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(groupId + ""));
        if(resultGroup != null){
            Group redisGroup = JSONObject.parseObject(resultGroup, Group.class);
            redisGroup.setGroup_name(jsonObject.getString("group_modify_content"));
            redisGroup.setInitCreateid(userId.intValue());
            redisGroup.setInitCreateid(userId.intValue());
            this.jedisCluster.srem(Constant.REDIS_GROUP_INFO.concat(groupId + ""), resultGroup);
            this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(redisGroup));
        }else {
            this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(group));
        }
        jsonObject.put("group_name", group.getGroup_name());
        jsonObject.put("group_manager_user_id", group.getGroup_manager_user_id());
        jsonObject.put("group_count", group.getGroup_count());
        jsonObject.put("group_id",groupId);
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(jsonObject).build();
    }

    /**
     * 修改群公告
     *
     * @param jsonObject
     * @return
     */
    public ClientMessage modifyGroupRemak(JSONObject jsonObject) {
        ClientMessage clientMessage = null;
        Long userId = jsonObject.getLong("user_id");
        Long groupId = jsonObject.getLong("group_id");
        Group group = this.groupServiceImpl.getGroupInfoByGroupId(groupId);
        group.setGroup_remark(jsonObject.getString("group_modify_content"));
        group.setRemark_update_time(new Date());
        try{

            //修改群信息
            this.groupServiceImpl.update(groupId, group);
            //修改群信息

            String resultGroup = this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(groupId + ""));
            if(resultGroup != null){
                Group redisGroup = JSONObject.parseObject(resultGroup, Group.class);
                redisGroup.setGroup_remark(jsonObject.getString("group_modify_content"));
                redisGroup.setInitCreateid(userId.intValue());
                this.jedisCluster.srem(Constant.REDIS_GROUP_INFO.concat(groupId + ""), resultGroup);
                this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(redisGroup));
            }else {
                this.jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat("" + groupId), JSONObject.toJSONString(group));
            }
        }catch (Exception e){
            logger.error("groupId:{},user_id:{},e:{}",groupId,userId,e);
            e.printStackTrace();
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_MODIFY_REMARK_ERROR).build();
        }
        jsonObject.put("group_name", group.getGroup_name());
        jsonObject.put("group_manager_user_id", group.getGroup_manager_user_id());
        jsonObject.put("group_count", group.getGroup_count());
        jsonObject.put("group_id",groupId);
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(jsonObject).build();
    }

    /**
     * 修改用户昵称
     *
     * @param jsonObject
     * @return
     */
    public ClientMessage modifyUserNickName(JSONObject jsonObject) {
        String user_id = jsonObject.getString("user_id");
        Long groupId = jsonObject.getLong("group_id");
        String nickName = jsonObject.getString("group_modify_content");
        Long userId = Long.parseLong(user_id);
        if(nickName.length() > Constant.GROUP_NICK_NAME_LEN){
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_MODIFY_NICK_NAME_ERROR).build();
        }
        Group group = this.groupServiceImpl.getGroupInfoByGroupId(groupId);
        GroupMember groupMember = new GroupMember();
        groupMember.setUser_id(userId.intValue());
        groupMember.setUpdate_date(new Date());
        groupMember.setMember_nickname(nickName);
        groupMember.setGroup_id(jsonObject.getLong("group_id"));
        this.groupMemberServiceImpl.update(groupId, groupMember);

        jsonObject.put("group_name", group.getGroup_name());
        jsonObject.put("group_manager_user_id", group.getGroup_manager_user_id());
        jsonObject.put("group_count", group.getGroup_count());
        jsonObject.put("group_id",groupId);
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(jsonObject).build();
    }

    /**
     * 查询群公告
     *
     * @param jsonObject
     * @return
     */
    public ClientMessage queryGroupRemark(JSONObject jsonObject) {
        String user_id = jsonObject.getString("user_id");
        Long groupId = jsonObject.getLong("group_id");
        Long userId = Long.parseLong(user_id);
        Group group = this.groupServiceImpl.getGroupInfoByGroupId(groupId);
        
        if(group  == null){
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_ID_INVALID).build();
        }
        Map<String, Object> map = new HashMap<String, Object>();
        map.put("group_id", groupId);
        map.put("group_remark",null == group.getGroup_remark()?"":group.getGroup_remark());
        
        Date remarkUpdateDate = null;
        if (group.getRemark_update_time() != null){
        	remarkUpdateDate = group.getRemark_update_time();
        }else if (group.getUpdate_date() != null){
        	remarkUpdateDate = group.getUpdate_date();
        }else if (group.getGroup_ct() != null){
        	remarkUpdateDate = group.getGroup_ct();
        }else{
        	remarkUpdateDate = new Date();
        }
        
        Integer remarUpdateUser = null;
        if (group.getGroup_manager_user_id() != null){
        	remarUpdateUser = group.getGroup_manager_user_id();
        }else if (group.getUpdate_user() != null){
        	remarUpdateUser = group.getUpdate_user();
        }else{
        	remarUpdateUser = 0;
        }
        map.put("update_date", remarkUpdateDate.getTime()/1000);
        map.put("update_user", remarUpdateUser);
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(map).build();
    }

    public ClientMessage queryGroupListGroupId(Long userId){
        Set<String> stringSet = this.jedisCluster.zrange(Constant.REDIS_USER_GROUP_LIST.concat(String.valueOf(userId)),0,-1);
        List<Map<String , Object>> list = new ArrayList<>();
        for(String string:stringSet){
            Map<String,Object> map = JSONObject.parseObject(string,Map.class);
            list.add(map);
        }
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(list).build();
    }

    public ClientMessage modifyRcvType(JSONObject jsonObject){
        String groupId = jsonObject.getString("group_id");
        String user_id = jsonObject.getString("user_id");
        Integer rcvType = jsonObject.getInteger("message_status");
        Long userId = Long.parseLong(user_id);
        Map<String , String> hashMap = new HashMap<>();
        hashMap.put(user_id,rcvType+"");
        //修改群成员信息
        this.jedisCluster.hmset(Constant.REDIS_GROUP_MEMBER.concat(groupId),hashMap);
        //修改群列表信息
        Set<String> stringSet = this.jedisCluster.zrangeByScore(Constant.REDIS_USER_GROUP_LIST.concat(user_id),groupId,groupId);
        for(String str:stringSet){
            Map map = JSONObject.parseObject(str);
            map.put("rcv_type",rcvType);
            this.jedisCluster.zremrangeByScore(Constant.REDIS_USER_GROUP_LIST.concat(user_id),Long.parseLong(groupId),Long.parseLong(groupId));
            this.jedisCluster.zadd(Constant.REDIS_USER_GROUP_LIST.concat(user_id),Long.parseLong(groupId),map.toString());
        }
        //数据库修改
        GroupMember groupMember = new GroupMember();
        groupMember.setGroup_id(Long.parseLong(groupId));
        groupMember.setUpdate_date(new Date());
        groupMember.setRcv_msg_status(rcvType);
        groupMember.setUser_id(Integer.parseInt(user_id));
        this.groupMemberServiceImpl.update(Long.parseLong(groupId),groupMember);
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).build();
    }

    public ClientMessage modifyIsAddressBook(JSONObject object){
        Long userId = object.getLong("user_id");
        Long groupId = object.getLong("group_id");
        Integer isAddressBook = object.getInteger("save_type");
        try{
            GroupMember groupMember = new GroupMember();
            groupMember.setUser_id(userId.intValue());
            groupMember.setGroup_id(groupId);
            groupMember.setIs_address_book(isAddressBook);
            groupMember.setUpdate_date(new Date());
            this.groupMemberServiceImpl.update(groupId,groupMember);

            //redis修改
            String strUserId = String.valueOf(userId);
            Set<String> stringSet = this.jedisCluster.zrangeByScore(Constant.REDIS_USER_GROUP_LIST.concat(strUserId),groupId,groupId);;
            for(String str:stringSet){
                Map<String , Object> map = JSONObject.parseObject(str);
                map.put("save_type",isAddressBook);
                this.jedisCluster.zremrangeByScore(Constant.REDIS_USER_GROUP_LIST.concat(strUserId),Long.parseLong(groupId+""),Long.parseLong(groupId+""));
                this.jedisCluster.zadd(Constant.REDIS_USER_GROUP_LIST.concat(strUserId),Long.parseLong(groupId+""),map.toString());
            }
        }catch (Exception e){
            e.printStackTrace();
            logger.error("e:{}",e);
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SYS_BASE).build();
        }
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).build();
    }

    public ClientMessage getUserGroupRemark(Long groupId,Long userId){
        GroupMember gm = new GroupMember();
        gm.setGroup_id(groupId);
        gm.setUser_id(userId.intValue());
        GroupMember groupMember = this.groupMemberServiceImpl.getGroupMemberInfo(groupId,gm);
        if(groupMember == null){
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_ID_INVALID).build();
        }
        Map<String , Object> map = new HashMap<String , Object>();
        map.put("user_remark_name",groupMember.getMember_nickname());
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(map).build();
    }

    public ClientMessage getGroupInfoList(JSONObject object){
        Long userId = object.getLong("user_id");
        JSONArray array = object.getJSONArray("list_group_id");
        if(array.isEmpty()){
            return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_GROUP_ID_INVALID).build();
        }

        List<Group> groupList = new ArrayList<>();
        for(int i = 0;i<array.size();i++){
            String groupId = array.get(i).toString();
            String groupInfo = this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(groupId));
            Group group = null;
            if(groupInfo !=null){
                group = JSONObject.parseObject(groupInfo,Group.class);
                group.setIs_valid(1);
            }else {
                group = new Group();
                group.setGroup_id(Long.parseLong(groupId));
                group.setIs_valid(0);
            }
            groupList.add(group);
        }
        return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).data(groupList).build();
    }
    
    /**
     * 根据groupId从缓存和db中获取群信息
     * @param groupId
     * @return
     */
    private Group getGroupInfoFromCacheAndDB(Long groupId){
    	if (groupId == null) return null;
    	//先从缓存中拿数据
    	String strGroup = this.jedisCluster.srandmember(Constant.REDIS_GROUP_INFO.concat(String.valueOf(groupId)));
    	//如果为空，则从数据库中拿
    	if (strGroup == null){
    		Group g = groupServiceImpl.getGroupInfoByGroupId(groupId);
    		if (g == null){
    			jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat(String.valueOf(groupId)), Constant.REDIS_NULL_VALUE);
    		}else{
    			jedisCluster.sadd(Constant.REDIS_GROUP_INFO.concat(String.valueOf(groupId)), JSONObject.toJSONString(g));
    		}
    		return g;
    	}else if (Constant.REDIS_NULL_VALUE.equals(strGroup)){  //如果缓存中缓存是是空值，则返回空
    		return null;
    	}else {
    		return JSONObject.parseObject(strGroup, Group.class);
    	}
    }
    
    //生成判重键值
    private String buildCheckRepeatOperKey(Long userId, Long sessionId, String operation, String data){
    	return String.format("%s_%d_%d_%s_%d", Constant.REDIS_GROUP_OPER_KEY_PREFIX, userId, sessionId, operation, data.hashCode());
    }
    
    /**
     * 检查是否重复操作（或发包）
     * @param userId
     * @param sessionId
     * @param operation
     * @return 1:不是重复操作   0：重复操作
     */
    private Integer checkRepeatOper(Long userId, Long sessionId, String operation, String data){
    	String redisKey = buildCheckRepeatOperKey(userId, sessionId, operation, data);
    	Long r = jedisCluster.setnx(redisKey, "0");    
    	//或操作成功，则设置有效期，再返回成功
    	if (r.intValue() == 1){
    		jedisCluster.expire(redisKey, repeatOperInterval);
    		return 1;
    	}else{
    		logger.warn("received repeat group operation in {} seconds interval, userId:{}, sessionId:{}, operation:{}, data:{}", repeatOperInterval, userId, sessionId, operation, data);
    		return 0;
    	}
    }
}
