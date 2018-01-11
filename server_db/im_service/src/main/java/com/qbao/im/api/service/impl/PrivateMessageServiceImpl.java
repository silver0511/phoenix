package com.qbao.im.api.service.impl;

import com.qbao.im.api.dao.IPrivateMessageDao;
import com.qbao.im.api.entity.PrivateMessage;
import com.qbao.im.api.service.msg.IPrivateMessageService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Date;
import java.util.List;

/**
 * Created by tangxiaojun on 2017/6/7.
 */
@Service
public class PrivateMessageServiceImpl implements IPrivateMessageService {

    @Autowired
    private IPrivateMessageDao privateMessageDao;

    @Override
    public List<PrivateMessage> queryAll(Integer bid, Integer cid, Integer wid, Date createDate, Integer pageIndex, Integer pageSize) {
        PrivateMessage pm = new PrivateMessage();
        pm.setB_id(bid);
        pm.setC_id(cid);
        pm.setW_id(wid);
        pm.setCreate_date(createDate);
        pm.setPageIndex(pageIndex);
        pm.setPageSize(pageSize);
        return this.privateMessageDao.query(pm);
    }
}
