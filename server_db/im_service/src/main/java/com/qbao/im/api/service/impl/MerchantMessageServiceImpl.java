package com.qbao.im.api.service.impl;

import com.alibaba.fastjson.JSONObject;
import com.qbao.im.api.dao.IMerchantMessageDao;
import com.qbao.im.api.entity.MerchantMessage;
import com.qbao.im.api.msg.ClientMessage;
import com.qbao.im.api.msg.ErrorCode;
import com.qbao.im.api.service.msg.IMerchantMessageService;
import com.qianbao.framework.datasource.annotation.DataSource;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

/**
 * Created by tangxiaojun on 2017/6/14.
 */
@Service
public class MerchantMessageServiceImpl implements IMerchantMessageService {

    private Logger logger = LoggerFactory.getLogger(MerchantMessageServiceImpl.class);

    @Autowired
    private IMerchantMessageDao merchantMessageDao;

    @Override
    public ClientMessage query(@DataSource(field = "bid") Integer bid, Integer cid,Long messageid,Integer maxcnt) {
        List<MerchantMessage> list = new ArrayList<>();
        try{
            Calendar c = Calendar.getInstance();
            c.add(Calendar.DATE, - 7);
            MerchantMessage merchantMessage = new MerchantMessage();
            merchantMessage.setB_id(bid);
            merchantMessage.setC_id(cid);
            merchantMessage.setMessage_id(messageid);
            merchantMessage.setPageSize(maxcnt);
            merchantMessage.setCreate_time(c.getTime());
            List<MerchantMessage> tmp = this.merchantMessageDao.query(merchantMessage);
            list.addAll(this.merchantMessageDao.query(merchantMessage)) ;
        }catch (Exception e){
            e.printStackTrace();
            logger.error("MerchantMessageServiceImpl:{}",e);
            return new ClientMessage.Builder().data(list).responseCode(ErrorCode.RET_BUSINESS_BASE).build();
        }

        return new ClientMessage.Builder().data(list).responseCode(ErrorCode.RET_SUCCESS).build();
    }

    @Override
    public ClientMessage queryCurrNew(@DataSource(field = "bid") Integer bid, Integer offset, Integer maxcnt) {
        List<MerchantMessage> list = new ArrayList<>();
        try{
            MerchantMessage merchantMessage = new MerchantMessage();
            merchantMessage.setB_id(bid);
            merchantMessage.setPageIndex(offset);
            merchantMessage.setPageSize(maxcnt);
            list.addAll(this.merchantMessageDao.queryCurrNew(merchantMessage));
        }catch (Exception e){
            e.printStackTrace();
            logger.error("MerchantMessageServiceImpl:{}",e);
            return new ClientMessage.Builder().data(list).responseCode(ErrorCode.RET_BUSINESS_BASE).build();
        }
        return new ClientMessage.Builder().data(list).responseCode(ErrorCode.RET_SUCCESS).build();
    }
}
