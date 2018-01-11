package com.qbao.im.api.service.msg;

import com.alibaba.fastjson.JSONObject;
import com.qbao.im.api.msg.ClientMessage;

/**
 * Created by tangxiaojun on 2017/6/14.
 */
public interface IMerchantMessageService {
	
    ClientMessage query(Integer bid,Integer cid,Long messageid,Integer maxcnt);

    ClientMessage queryCurrNew(Integer bid,Integer offset,Integer maxcnt);
}
