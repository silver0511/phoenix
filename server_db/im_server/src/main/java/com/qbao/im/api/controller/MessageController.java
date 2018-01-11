package com.qbao.im.api.controller;

import com.qbao.im.api.msg.ClientMessage;
import com.qbao.im.api.service.msg.IMerchantMessageService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;

import javax.ws.rs.*;
import javax.ws.rs.core.MediaType;

/**
 * Created by tangxiaojun on 2017/6/8.
 */
@Controller
@Path("/api/message")
public class MessageController {

    private Logger logger = LoggerFactory.getLogger(MessageController.class);

    @Autowired
    private IMerchantMessageService merchantMessageService;

    /**
     * 私聊历史消息
     * @param data
     * @return
     */
    @POST
    @Path("/private")
    @Produces("application/json; charset=utf-8")
    @Encoded
    public ClientMessage queryPrivate(String data){
        return null;
    }

    /**
     * 群组历史消息
     * @param data
     * @return
     */
    @POST
    @Path("/group")
    @Produces("application/json; charset=utf-8")
    @Encoded
    public ClientMessage queryGroup(String data){
        return null;
    }
    
    /**
     * 查询商家与客户聊天历史信息
     * @param bid 商家id
     * @param cid 用户id
     * @param messageid 消息id
     * @param maxcnt 记录条数
     * @return
     */
    @GET
    @Path("/merchant")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage queryMerchant(@QueryParam("b_id") Integer bid,@QueryParam("c_id") Integer cid,@QueryParam("message_id") Long messageid,@QueryParam("max_cnt") Integer maxcnt, @QueryParam("b_type")Integer type){
        return this.merchantMessageService.query(bid, cid, messageid, maxcnt);
    }
    
    /**
     * 获取指定商家当天所有聊天记录
     * @param bid	商家id
     * @param offset	起始位置
     * @param maxcnt	结束位置
     * @return
     */
    @GET
    @Path("/merchant/currday")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage queryCurrMerchant(@QueryParam("b_id") Integer bid,@QueryParam("offset") Integer offset,@QueryParam("max_cnt") Integer maxcnt){
        return this.merchantMessageService.queryCurrNew(bid,offset,maxcnt);
    }
}
