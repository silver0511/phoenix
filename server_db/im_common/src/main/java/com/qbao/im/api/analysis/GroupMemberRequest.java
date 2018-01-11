package com.qbao.im.api.analysis;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.qbao.im.api.entity.GroupMember;
import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * Created by tangxiaojun on 2017/4/14.
 */
public class GroupMemberRequest {

    public static List<GroupMember> getMemberList(JSONObject object,Long groupId){
        JSONArray arr = object.getJSONArray("list_user_change");
        List<GroupMember> memberList = new ArrayList<>();
        if(!arr.isEmpty()){
        	Date now = new Date();
            for (int i = 0; i < arr.size(); i++) {
                GroupMember groupMember = new GroupMember();
                JSONObject obj = JSONObject.parseObject(arr.get(i).toString());
                groupMember.setUser_id(obj.getInteger("user_id"));
                groupMember.setGroup_id(groupId);
                groupMember.setMember_type(i == 0 ? 2 : 0);
                groupMember.setMember_nickname(obj.getString("user_nick_name"));
                groupMember.setMember_joindate(now);
                memberList.add(groupMember);
            }
        }
        return memberList;
    }

    public static String getMemberListStr(JSONObject object){
        JSONArray arr = object.getJSONArray("list_user_change");
        String arrStr[] = new String[arr.size()];
        if(!arr.isEmpty()){
            for (int i = 0; i < arr.size(); i++) {
                arrStr[i] = JSONObject.parseObject(arr.get(i).toString()).getInteger("user_id")+"".concat("=").concat(JSONObject.parseObject(arr.get(i).toString()).getString("user_nick_name"));
            }
        }
        if(arrStr.length > 0){
            return StringUtils.arrayToCommaDelimitedString(arrStr);
        }
        return null;
    }

    public static JSONArray getMemberList(List<GroupMember> groupMembers){
        JSONArray array = new JSONArray();
        for (int i =0;i<groupMembers.size();i++){
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("user_group_index",groupMembers.get(i).getId());
//            jsonObject.put("user_id",groupMembers.get(i).getUserid()+"");
//            jsonObject.put("user_nick_name",groupMembers.get(i).getMembernickname());
            array.add(jsonObject);
        }
        return array;
    }
}
