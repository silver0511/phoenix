package com.qbao.im.api.controller;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.qbao.im.api.entity.BatchTest;
import com.qbao.im.api.msg.ClientMessage;
import com.qbao.im.api.msg.ErrorCode;
import com.qbao.im.api.service.batch.IBatchService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import redis.clients.jedis.JedisCluster;

import javax.ws.rs.*;
import javax.ws.rs.core.MediaType;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by tangxiaojun on 2017/6/20.
 */
@Controller
@Path("/test")
public class TestController {

    @Autowired
    private IBatchService iBatchService;

    @Autowired
    JedisCluster jedisCluster;

    @GET
    @Path("/batch")
    @Produces(MediaType.APPLICATION_JSON)
    public ClientMessage test(){
        try{
            List<BatchTest> list = new ArrayList<>();
            BatchTest batchTest = new BatchTest();
            batchTest.setGroupid(98517984l);
            batchTest.setUserid(10000000l);
            list.add(batchTest);
            BatchTest batchTest1 = new BatchTest();
            batchTest1.setGroupid(98517984l);
            batchTest1.setUserid(10000023l);
            list.add(batchTest1);
            BatchTest batchTest2 = new BatchTest();
            batchTest2.setGroupid(93426280l);
            batchTest2.setUserid(10000036l);
            list.add(batchTest2);
            BatchTest batchTest3 = new BatchTest();
            batchTest3.setGroupid(93426280l);
            batchTest3.setUserid(10000036l);
            list.add(batchTest3);
            BatchTest batchTest4 = new BatchTest();
            batchTest4.setGroupid(93426280l);
            batchTest4.setUserid(10000023l);
            list.add(batchTest4);
            this.iBatchService.batch(93426280l,list);
        }catch (Exception e){
            e.printStackTrace();
            return new ClientMessage.Builder().responseCode(ErrorCode.END_OF_ERROR).build();
        }
        return new ClientMessage.Builder().responseCode(ErrorCode.RET_SUCCESS).build();
    }

    @POST
    @Path("/redis")
    @Produces("application/json; charset=utf-8")
    @Encoded
     public ClientMessage redisTest(String data){
        JSONObject jsonObject = JSONObject.parseObject(data);
//        ListOperations list = this.redisTemplate.opsForList();
        String userid = jsonObject.getString("userid");
        jedisCluster.lpush("nim_test_lpush","test111");
//        redisUtil.addList("nim_redis_test_".concat(userid),"test_".concat(userid));
//        Long result = redisUtil.hincrBy("groupKey","",10000000);
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
//        list.leftPush("nim_redis_test_".concat(userid),"test_".concat(userid));
        return new ClientMessage.Builder().responseCode(ErrorCode.RET_SUCCESS).build();
     }

    @POST
    @Path("/redis2")
    @Produces("application/json; charset=utf-8")
    @Encoded
    public ClientMessage redisTest2(String data){
        JSONObject jsonObject = JSONObject.parseObject(data);
        JSONArray arr = jsonObject.getJSONArray("list_user_change");
//        ListOperations list = this.redisTemplate.opsForList();
        if(!arr.isEmpty()){
            for(int i =0;i<arr.size();i++){
                String userId = JSONObject.parseObject(arr.get(i).toString()).getString("user_id");
                jedisCluster.lpush("nim_redis_test2_".concat(userId),"test3_".concat(userId));
//                list.leftPush("nim_redis_test2_".concat(userId),"test2_".concat(userId));
//                redisUtil.addList("nim_redis_test2_".concat(userId),"test2_".concat(userId));
            }
        }

        return new ClientMessage.Builder().responseCode(ErrorCode.RET_SUCCESS).build();
    }

    @GET
    @Path("/hash")
    @Produces(MediaType.APPLICATION_JSON)
    public String test3(){
        Map<String ,String> map1 = new HashMap<>();
        map1.put("userId_100000","100000");
        map1.put("rcvType_100000","0");
        Map<String , String> map2 = new HashMap<>();
        map2.put("userId_100001","100001");
        map2.put("rcvType_100001","0");
        this.jedisCluster.hmset("grouId:1001",map1);
        this.jedisCluster.hmset("grouId:1001",map2);
        return "SUCCESS";
    }

    @GET
    @Path("/test4")
    @Produces("text/plain; charset=utf-8")
    public String test4(@QueryParam("name") String name){
        System.out.println(name);
        return name;
    }
}
