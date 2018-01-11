package com.qbao.im.api.service.impl;

import com.qbao.im.api.dao.IGroupDao;
import com.qbao.im.api.dao.IGroupMemberDao;
import com.qbao.im.api.entity.Group;
import com.qbao.im.api.entity.GroupMember;
import com.qbao.im.api.entity.SliceInfo;
import com.qbao.im.api.service.group.IGroupMemberService;
import com.qianbao.framework.datasource.DynamicDataSourceHolder;
import com.qianbao.framework.datasource.annotation.DataSource;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.*;

/**
 * Created by tangxiaojun on 2017/4/27.
 */
@Service
public class GroupMemberServiceImpl implements IGroupMemberService {

	private static final Logger logger = LoggerFactory.getLogger(GroupMemberServiceImpl.class);

	@Autowired
	private IGroupDao groupDao;

	@Autowired
	private IGroupMemberDao groupMemberDao;

	// @Autowired
	// private IMemberGroupListDao memberGroupListDao;

	@Override
	public Map<String, Object> add(@DataSource(field = "groupId") Long groupId, Group group,
			List<GroupMember> groupMembers, Long userId) {
		Map<String, Object> map = new HashMap<String, Object>();
		Date now = new Date();
		// 加入更新时间
		for (int i = 0; i < groupMembers.size(); i++) {
			groupMembers.get(i).setUpdate_date(now);
		}
		Map<String, Object> batchMap = new HashMap<>();
		batchMap.put("tabName", groupMembers.get(0).getTabName());
		batchMap.put("list", groupMembers);
		this.groupMemberDao.batchSave(batchMap);
		group.setGroup_count(group.getGroup_count() + groupMembers.size());
		// 修改群信息
		this.groupDao.update(group);
		map.put("group_add_is_agree", 0);
		return map;
	}

	@Override
	public Map<String, Object> kick(@DataSource(field = "groupId") Long groupId, Group group,
			List<GroupMember> groupMembers, Long userId) {
		Map<String, Object> map = new HashMap<>();
		map.put("success", 0);
		// 群主自己退出或者用户自己退出
		if (groupMembers.size() == 1) {
			GroupMember groupMember = new GroupMember();
			groupMember.setGroup_id(groupId);

			// 群主自己退出
			if (groupMembers.get(0).getUser_id().intValue() == group.getGroup_manager_user_id().intValue()) {
				groupMember.setUser_id(userId.intValue());
				// 删除群成员信息
				this.groupMemberDao.del(groupMember);
				if (this.groupMemberDao.getGroupMemberOrderById(groupMember).isEmpty()) {
					this.groupDao.del(group);
				} else {
					// 修改群信息
					group.setGroup_manager_user_id(
							this.groupMemberDao.getGroupMemberOrderById(groupMember).get(0).getUser_id());
					group.setGroup_count(group.getGroup_count() - groupMembers.size());
					this.groupDao.update(group);
				}

				// 修改群成员列表
				GroupMember gm = new GroupMember();
				gm.setGroup_id(groupId);
				gm.setUser_id(group.getGroup_manager_user_id());
				gm.setUpdate_date(new Date());
				gm.setMember_type(2);
				this.groupMemberDao.update(gm);
			} else {
				// 删除群成员信息
				groupMember.setUser_id(groupMembers.get(0).getUser_id());
				this.groupMemberDao.del(groupMember);
				group.setGroup_count(group.getGroup_count() - groupMembers.size() < 0 ? 0
						: group.getGroup_count() - groupMembers.size());
				// 修改群人数信息
				this.groupDao.update(group);
			}
			map.put("success", 1);
		}
		// 群主正常踢人
		if (userId.intValue() == group.getGroup_manager_user_id() && groupMembers.size() > 1) {
			Map<String, Object> batchGroupMemberMap = new HashMap<>();
			// List<GroupMember> batchList = new ArrayList<>();
			List<Integer> batchList = new ArrayList<>();
			for (int i = 0; i < groupMembers.size(); i++) {
				// GroupMember groupMember = new GroupMember();
				// groupMember.setGroup_id(groupId);
				// groupMember.setUser_id(groupMembers.get(i).getUser_id());
				batchList.add(groupMembers.get(i).getUser_id());
				// this.groupMemberDao.del(groupMember);
			}
			GroupMember tabName = new GroupMember();
			tabName.setGroup_id(groupId);
			batchGroupMemberMap.put("group_id", groupId);
			batchGroupMemberMap.put("tabName", tabName.getTabName());
			batchGroupMemberMap.put("list", batchList);
			this.groupMemberDao.batchDel(batchGroupMemberMap);
			GroupMember groupMember = new GroupMember();
			groupMember.setGroup_id(groupId);
			// int memberCount =
			// this.groupMemberDao.getMemberListCount(groupMember);
			group.setGroup_count(group.getGroup_count() - groupMembers.size() < 0 ? 0
					: group.getGroup_count() - groupMembers.size());
			// 修改群信息
			this.groupDao.update(group);
			map.put("success", 1);
		}
		return map;
		// return null;
	}

	@Override
	public List<GroupMember> getMemberList(@DataSource(field = "groupId") Long groupId, Integer pageIndex,
			Integer pageSize) {
		GroupMember groupMember = new GroupMember();
		groupMember.setGroup_id(groupId);
		groupMember.setPageIndex(pageIndex);
		groupMember.setPageSize(pageSize);
		return this.groupMemberDao.getMemberList(groupMember);
		// return null;
	}

	@Override
	public int getMemberListCount(@DataSource(field = "groupId") Long groupId) {
		GroupMember groupMember = new GroupMember();
		groupMember.setGroup_id(groupId);
		return this.groupMemberDao.getMemberListCount(groupMember);
		// return 0;
	}

	@Override
	public GroupMember getGroupMemberOrderBy(@DataSource(field = "groupId") Long groupId, GroupMember groupMember) {
		return this.groupMemberDao.getGroupMemberByUserId(groupMember);
		// return null;
	}

	@Override
	public int getGrouoMemberCount(@DataSource(field = "groupId") Long groupId, GroupMember groupMember) {
		return this.groupMemberDao.getGroupMemberCount(groupMember);
		// return 0;
	}

	@Override
	public GroupMember getGroupMemberInfo(@DataSource(field = "groupId") Long groupId, GroupMember groupMember) {
		return this.groupMemberDao.getGroupMemberByUserId(groupMember);
	}

	@Override
	public Boolean isExsistGroup(@DataSource(field = "groupId") Long groupId, Long userId) {
		GroupMember group = new GroupMember();
		group.setGroup_id(groupId);
		group.setUser_id(userId.intValue());
		return this.groupMemberDao.isExsist(group);
	}

	@Override
	public void batchGroupMember(@DataSource(field = "groupId") Long groupId, Map<String, Object> map) {
		logger.info("batch save group. groupId:{}", groupId);
		this.groupMemberDao.batchSave(map);
	}

	@Override
	public void callGroupMember(String groupList, Long groupId) {
		Map<String, Object> map = new HashMap<String, Object>();
		map.put("group_list", groupList);
		map.put("group_id", groupId);
		this.groupMemberDao.callSave(map);
	}

	@Override
	public void update(@DataSource(field = "groupId") Long groupId, GroupMember groupMember) {
		this.groupMemberDao.update(groupMember);
	}

	@Override
	public void updateMemberType(@DataSource(field = "groupId") Long groupId, GroupMember groupMember) {
		this.groupMemberDao.updateMemberType(groupMember);
	}

	@Override
	public SliceInfo getGroupMemberDBAndTableName(@DataSource(field = "groupId") Long groupId) {
		GroupMember gm = new GroupMember();
		gm.setGroup_id(groupId);
		SliceInfo si = new SliceInfo();
		si.setTableName(gm.getTabName());
		si.setDatasourceName(DynamicDataSourceHolder.getDataSouce());
		return si;
	}
}
