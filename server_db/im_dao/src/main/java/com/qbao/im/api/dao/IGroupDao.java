package com.qbao.im.api.dao;

import com.qbao.im.api.entity.Group;

/**
 * Created by tangxiaojun on 2017/5/22.
 */
public interface IGroupDao {

    /**
     * 保存群组信息
     * @param group
     * @return
     */
    int save(Group group);

    /**
     * 根据群组编号查询群组信息
     * @param group
     * @return
     */
    Group query(Group group);

    /**
     * 群主转让
     * @param group
     */
    void update(Group group);

    /**
     * 删除群
     * @param group
     */
    void del(Group group);

    /**
     * 查询群是否存在
     * @param group
     * @return
     */
    int isExsist(Group group);
}
