package com.qbao.im.api.service.impl;

import com.qbao.im.api.service.group.IMemberGroupListService;
import com.qianbao.framework.datasource.annotation.DataSource;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

/**
 * Created by tangxiaojun on 2017/4/28.
 */
@Service
public class MemberGroupListServiceImpl implements IMemberGroupListService {

    private Logger logger = LoggerFactory.getLogger(MemberGroupListServiceImpl.class);

//    @Autowired
//    private IMemberGroupListDao memberGroupListDao;

//    @Override
//    public List<MemberGroupList> getMemberGroupList(@DataSource(field = "userId") Long userId, Integer pageIndex, Integer pageSize) {
//        MemberGroupList memberGroupList = new MemberGroupList();
//        memberGroupList.setUserid(userId.intValue());
//        memberGroupList.setPageIndex(pageIndex);
//        memberGroupList.setPageSize(pageSize);
//        return this.memberGroupListDao.query(memberGroupList);
//    }

//    @Override
//    public Integer getMemberGroupCount(@DataSource(field = "userId") Long userId) {
//        MemberGroupList memberGroupList = new MemberGroupList();
//        memberGroupList.setUserid(userId.intValue());
//        return this.memberGroupListDao.queryCount(memberGroupList);
//    }

//    @Override
//    public void delMemberGroupList(@DataSource(field = "userId")Long userId, MemberGroupList memberGroupList) {
//        this.memberGroupListDao.del(memberGroupList);
//    }

//    @Override
//    public void updateMemberGroupList(@DataSource(field = "userId")Long userId, MemberGroupList memberGroupList) {
//        this.memberGroupListDao.update(memberGroupList);
//    }

//    @Override
//    public void saveMemberGroupList(@DataSource(field = "userId")Long userId, MemberGroupList memberGroupList) {
//        this.memberGroupListDao.save(memberGroupList);
//    }

}
