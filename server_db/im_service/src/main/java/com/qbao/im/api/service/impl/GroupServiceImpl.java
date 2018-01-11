package com.qbao.im.api.service.impl;

import com.qbao.im.api.dao.IGroupDao;
import com.qbao.im.api.entity.Group;
import com.qbao.im.api.entity.SliceInfo;
import com.qbao.im.api.msg.BaseMessage;
import com.qbao.im.api.service.group.IGroupService;
import com.qianbao.framework.datasource.DynamicDataSourceHolder;
import com.qianbao.framework.datasource.annotation.DataSource;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.*;

/**
 * Created by tangxiaojun on 2017/4/14.
 */
@Service
public class GroupServiceImpl implements IGroupService {

    private static final Logger logger = LoggerFactory.getLogger(GroupServiceImpl.class);

    @Autowired
    private IGroupDao groupDao;

    @Override
    public Group getGroupInfo(@DataSource(field = "groupId") Long groupId, Group group) {
        return this.groupDao.query(group);
    }

    @Override
    public Group getGroupInfoByGroupId(@DataSource(field = "groupId") Long groupId) {
        Group group = new Group();
        group.setGroup_id(groupId);
        return this.groupDao.query(group);
    }

    @Override
    public void saveGroup(@DataSource(field = "groupId") Long groupId, Group group) {
        this.groupDao.save(group);
    }

    @Override
    public void update(@DataSource(field = "groupId") Long groupId, Group group) {
        this.groupDao.update(group);
    }

    @Override
    public int isExsist(@DataSource(field = "groupId") Long groupId, Group group) {
        return this.groupDao.isExsist(group);
    }

	@Override
	public SliceInfo getGroupDBAndTableName(@DataSource(field = "groupId")Long groupId) {
		Group g = new Group();
		g.setGroup_id(groupId);
		SliceInfo si = new SliceInfo();
		si.setTableName(g.getTabName());
		si.setDatasourceName(DynamicDataSourceHolder.getDataSouce());
		return si;
	}
}
