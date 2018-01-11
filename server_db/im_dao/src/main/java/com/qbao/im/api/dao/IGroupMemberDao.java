package com.qbao.im.api.dao;

import com.qbao.im.api.entity.GroupMember;

import java.util.List;
import java.util.Map;

/**
 * Created by tangxiaojun on 2017/5/23.
 */
public interface IGroupMemberDao {

    /**
     * 批量添加数据
     * @param map
     * @return
     */
    int batchSave(Map<String , Object> map);

    /**
     * 添加群组列表
     * @param map
     */
    void callSave(Map<String , Object> map);

    /**
     * 添加群组信息
     * @param member
     * @return
     */
    int save(GroupMember member);

    /**
     * 查询用户是否在群组列表中
     * @return
     */
    Boolean isExsist(GroupMember member);

    /**
     * 获取群成员列表
     * @return
     */
    List<GroupMember> getMemberList(GroupMember groupMember);

    /**
     * 获取群成员列表总个数
     * @param groupMember
     * @return
     */
    int getMemberListCount(GroupMember groupMember);

    /**
     * 修改成员列表信息
     * @param groupMember
     */
    void update(GroupMember groupMember);

    /**
     * 群主转让
     * @param groupMember
     */
    void updateMemberType(GroupMember groupMember);

    /**
     * 删除成员列表信息
     * @param groupMember
     */
    void del(GroupMember groupMember);

    /**
     * 批量删除
     * @param map
     */
    void batchDel(Map<String,Object> map);

    /**
     * 获取群成员信息
     * @param groupMember
     * @return
     */
    GroupMember getGroupMemberByUserId(GroupMember groupMember);

    /**
     * 查询群成员列表是否存在
     * @param groupMember
     * @return
     */
    int getGroupMemberCount(GroupMember groupMember);

    /**
     * 获取最新进群的成员编号
     * @param groupMember
     * @return
     */
    List<GroupMember> getGroupMemberOrderById(GroupMember groupMember);
}
