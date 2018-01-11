package com.qbao.im.api.analysis;

import com.alibaba.fastjson.JSONObject;
import com.qbao.im.api.entity.Group;

import java.util.Date;

/**
 * Created by tangxiaojun on 2017/4/14.
 * 封装群组请求信息
 */
public class GroupRequest {

    public static Group getGroup(JSONObject object,int currentNum,Long groupId){
    	Date createDate = new Date(object.getLong("group_ct")/1000);
        Group group = new Group();
        group.setGroup_id(groupId);
        group.setGroup_name(object.getString("group_name"));
        group.setGroup_avater(object.containsKey("groupavater")?object.getString("groupavater"):"");
        group.setGroup_remark(object.containsKey("group_remark")?object.getString("group_remark"):"");
        if (!"".equals(group.getGroup_remark())){
        	group.setRemark_update_time(createDate);
        }
        group.setGroup_manager_user_id(object.getInteger("user_id"));
        group.setGroup_count(currentNum);
        group.setGroup_max_count(Integer.parseInt(object.getString("group_max_count")));
        group.setGroup_add_max_count(Integer.parseInt(object.getString("group_add_max_count")));
        group.setGroup_ct(createDate);
        group.setGroup_add_is_agree(object.containsKey("group_add_is_agree")?Integer.parseInt(object.getString("group_add_is_agree")):0);
        return group;
    }
}
