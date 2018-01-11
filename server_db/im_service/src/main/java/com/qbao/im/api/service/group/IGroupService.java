package com.qbao.im.api.service.group;

import com.qbao.im.api.entity.Group;
import com.qbao.im.api.entity.SliceInfo;

/**
 * Created by tangxiaojun on 2017/4/14.
 */
public interface IGroupService {

    /**
     * 通过群编号查询群是否存在
     * @param groupId
     * @param group
     * @return
     */
    Group getGroupInfo(Long groupId, Group group);

    /**
     * 获取群信息
     * @param groupId
     * @return
     */
    Group getGroupInfoByGroupId(Long groupId);

    /**
     * 保存群组信息
     * @param groupId
     * @param group
     */
    void saveGroup(Long groupId, Group group);

    /**
     * 修改群信息
     * @param groupId
     * @param group
     */
    void update(Long groupId, Group group);

    /**
     * 查询群是否存在
     * @param groupId
     * @param group
     * @return
     */
    int isExsist(Long groupId,Group group);
    
    /**
     * 查询组相关分库分表信息
     * @param groupId
     * @return
     */
    SliceInfo getGroupDBAndTableName(Long groupId);
}
