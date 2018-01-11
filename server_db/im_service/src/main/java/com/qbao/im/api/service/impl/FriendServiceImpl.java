package com.qbao.im.api.service.impl;

import com.qbao.im.api.dao.IFriendDao;
import com.qbao.im.api.entity.Friend;
import com.qbao.im.api.entity.SliceInfo;
import com.qbao.im.api.service.friend.IFriendService;
import com.qianbao.framework.datasource.DynamicDataSourceHolder;
import com.qianbao.framework.datasource.annotation.DataSource;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Date;
import java.util.List;

/**
 * Created by tangxiaojun on 2017/5/15.
 */
@Service
public class FriendServiceImpl implements IFriendService{

    private static final Logger logger = LoggerFactory.getLogger(FriendServiceImpl.class);

    @Autowired
    private IFriendDao friendDao;

    @Override
    public List<Friend> getFriends(@DataSource(field = "userId") Long userId, Integer pageIndex, Integer pageSize,Integer status) {
        Friend friend = new Friend();
        friend.setUserId(userId);
        friend.setPageIndex(pageIndex);
        friend.setPageSize(pageSize);
        friend.setStatus(status);
        List<Friend> list = this.friendDao.getFriends(friend);
        return list;
    }

    @Override
    public List<Friend> getFriends(@DataSource(field = "userId") Long userId, Long firendId) {
        Friend friend = new Friend(userId,firendId);
        return this.friendDao.getFriends(friend);
    }

    @Override
    public int save(@DataSource(field = "userId")Long userId, Friend friend) throws Exception {
        friend.setUserId(userId);
        friend.setCreateDate(new Date());
        return this.friendDao.save(friend);
    }

    @Override
    public int delFriend(@DataSource(field = "userId")Long userId, Friend friend) {
        return this.friendDao.del(friend);
    }

    @Override
    public int modifyFriend(@DataSource(field = "userId")Long userId, Friend friend) {
        return this.friendDao.modify(friend);
    }
    
    @Deprecated
    @Override
    public int modifyBlack(@DataSource(field = "userId") Long userId, Friend friend, Integer type) {
        return this.friendDao.modifyBlack(friend,type);
    }

    @Override
    public int getFriendMaxNum(@DataSource(field = "userId") Long userId, Friend friend) {
        return this.friendDao.getFriendCount(friend);
    }

	@Override
	public SliceInfo getFriendDBAndTableName(@DataSource(field = "userId")Long userId) {
		Friend f = new Friend();
		f.setUserId(userId);
		SliceInfo si = new SliceInfo();
		si.setTableName(f.getTabName());
		si.setDatasourceName(DynamicDataSourceHolder.getDataSouce());
		return si;
	}

}
