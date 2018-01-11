package com.qbao.im.api.service.group;

import java.util.List;
import java.util.Map;

import com.qbao.im.api.entity.Group;
import com.qbao.im.api.entity.GroupMember;
import com.qbao.im.api.entity.SliceInfo;

/**
 * Created by tangxiaojun on 2017/4/27.
 * 群成员接口
 */
public interface IGroupMemberService {

    /**
     * 群加人
     * @param groupId
     * @param group
     * @param groupMembers
     * @param userId
     * @return
     */
    Map<String , Object> add(Long groupId, Group group, List<GroupMember> groupMembers, Long userId);

    /**
     * 踢人
     * @param groupId
     * @param group
     * @param groupMembers
     * @param userId
     */
    Map<String, Object> kick(Long groupId, Group group, List<GroupMember> groupMembers, Long userId);

    /**
     * 获取群成员列表
     * @param groupId
     * @param pageIndex
     * @param pageSize
     * @return
     */
    public List<GroupMember> getMemberList(Long groupId, Integer pageIndex, Integer pageSize);

    /**
     * 获取群成员列表总数
     * @param groupId
     * @return
     */
    int getMemberListCount(Long groupId);
    /**
     * 获取最先加进群的成员
     * @param groupId
     * @param groupMember
     * @return
     */
    GroupMember getGroupMemberOrderBy(Long groupId, GroupMember groupMember);

    /**
     * 查询群成员是否存在
     * @param groupId
     * @param groupMember
     * @return
     */
    int getGrouoMemberCount(Long groupId, GroupMember groupMember);

    /**
     * 获取群成员信息
     * @param groupId
     * @param groupMember
     * @return
     */
    GroupMember getGroupMemberInfo(Long groupId, GroupMember groupMember);

    /**
     * 批量保存群成员列表信息
     * @param groupId
     * @param map
     */
    void batchGroupMember(Long groupId, Map<String, Object> map);

    /**
     * 存储过程
     * @param groupList
     * @param groupId
     */
    void callGroupMember(String groupList,Long groupId);

    /**
     * 查询群成员是否存在
     * @param groupId
     * @param userId
     * @return
     */
    Boolean isExsistGroup(Long groupId, Long userId);

    /**
     * 修改群成员列表
     * @param groupId
     * @param groupMember
     */
    void update(Long groupId, GroupMember groupMember);

    /**
     * 修改群成员类型
     * @param groupId
     * @param groupMember
     */
    void updateMemberType(Long groupId, GroupMember groupMember);
    
    /**
     * 获取群成员表分库分表信息
     * @param groupId
     * @return
     */
    SliceInfo getGroupMemberDBAndTableName(Long groupId);
}
