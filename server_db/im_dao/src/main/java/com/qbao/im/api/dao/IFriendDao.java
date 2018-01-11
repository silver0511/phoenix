package com.qbao.im.api.dao;

import com.qbao.im.api.entity.Friend;
import org.apache.ibatis.annotations.Param;
import org.springframework.stereotype.Repository;

import java.util.List;

/**
 * Created by tangxiaojun on 2017/5/12.
 */
//@Repository
public interface IFriendDao {

    /**
     * 查询该用户所有好友信息
     *
     * @param friend
     * @return
     */
    List<Friend> getFriends(Friend friend);

    /**
     * 添加好友关系
     * @param friend
     * @return
     */
    int save(Friend friend);

    /**
     * 修改好友关系
     * @param friend
     * @return
     */
    int modify(Friend friend);

    /**
     * 删除好友关系(物理删除)
     * @param friend
     * @return
     */
    int del(Friend friend);

    /**
     * 修改黑名单
     * @param friend
     * @param blackType
     * @return
     */
    int modifyBlack(@Param("friend") Friend friend,@Param("blackType") Integer blackType);

    /**
     * 查询好友个数
     * @param friend
     * @return
     */
    int getFriendCount(Friend friend);
}
