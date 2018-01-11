package com.qbao.im.api.service.msg;

import com.qbao.im.api.entity.PrivateMessage;

import java.util.Date;
import java.util.List;

/**
 * Created by tangxiaojun on 2017/6/7.
 */
public interface IPrivateMessageService {

    List<PrivateMessage> queryAll(Integer bid, Integer cid, Integer wid, Date createDate, Integer pageIndex, Integer pageSize);
}
