package com.qbao.im.api.dao;

import com.qbao.im.api.entity.PrivateMessage;

import java.util.List;

/**
 * Created by tangxiaojun on 2017/6/7.
 */
public interface IPrivateMessageDao {

    List<PrivateMessage> query(PrivateMessage privateMessage);
}
