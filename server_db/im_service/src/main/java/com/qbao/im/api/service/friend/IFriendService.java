package com.qbao.im.api.service.friend;

import java.util.List;

import com.qbao.im.api.entity.Friend;
import com.qbao.im.api.entity.SliceInfo;

/**
 * Created by tangxiaojun on 2017/5/15.
 */
public interface IFriendService {

    /**
     * 获取好友列表
     * @param userId
     * @param pageIndex
     * @param pageSize
     * @return
     */
    List<Friend> getFriends(Long userId, Integer pageIndex, Integer pageSize,Integer status);

    /**
     * 获取好友列表
     * @param userId
     * @param firendId
     * @return
     */
    List<Friend> getFriends(Long userId , Long firendId);

    /**
     * 保存好友关系
     * @param userId
     * @param friend
     * @return
     * @throws Exception
     */
    int save(Long userId,Friend friend) throws Exception;

    /**
     * 删除用户
     * @param userId
     * @param friend
     * @return
     */
    int delFriend(Long userId, Friend friend);

    /**
     * 修改用户数据
     * @param userId
     * @param friend
     * @return
     */
    int modifyFriend(Long userId,Friend friend);

    /**
     * 修改黑名单
     * @param userId
     * @param friend
     * @param type
     * @return
     */
    int modifyBlack(Long userId,Friend friend,Integer type);

    /**
     * 查询有效好友个数
     * @param userId
     * @param friend
     * @return
     */
    int getFriendMaxNum(Long userId,Friend friend);
    
    /**
     * 根据用户id查找所在数据库和表名
     * @param userId
     * @return
     */
    SliceInfo getFriendDBAndTableName(Long userId);
}
