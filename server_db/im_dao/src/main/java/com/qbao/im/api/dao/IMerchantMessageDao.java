package com.qbao.im.api.dao;

import com.qbao.im.api.entity.MerchantMessage;

import java.util.List;

/**
 * Created by tangxiaojun on 2017/6/13.
 */
public interface IMerchantMessageDao {

    /**
     * 查询历史记录
     * @param merchantMessage
     * @return
     */
    List<MerchantMessage> query(MerchantMessage merchantMessage);

    /**
     * 查询当天每个用户的最新消息
     * @param merchantMessage
     * @return
     */
    List<MerchantMessage> queryCurrNew(MerchantMessage merchantMessage);
}
