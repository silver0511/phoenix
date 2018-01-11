package com.qbao.im.api.utils;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;

import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Created by tangxiaojun on 2017/4/14.
 */
public class GenerateUtil {
	
	private static final SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");
	private static final Random random = new Random();

    public static Long getGroupId(){
//       Long currTime = System.currentTimeMillis();
//       Long nonTime = System.nanoTime();
//       String handTime = (currTime+"").substring(1)+(nonTime+"").substring(7,10);
//       handTime = handTime.substring(7,handTime.length());
//        return Math.abs(Long.parseLong(handTime));
        Long t = System.currentTimeMillis();
        Long t2 = System.nanoTime();
        Long groupId = 0l;
        int rand = (int)(Math.random()*25);
        t2 = t2 >> rand;
        if(t2.toString().length() > t.toString().length()){
            groupId = t2 - t;
        }else {
            groupId = t - t2;
        }
        if(groupId.toString().length()>8){
            groupId = Long.parseLong(groupId.toString().substring(groupId.toString().length()-8));
        }
        return Math.abs(groupId+Thread.currentThread().getId());
    }

    public static Long subEnd(Long time,int len){
        String t = String.valueOf(time);
        t = t.substring(t.length()-len,t.length());
        return Long.parseLong(t);
    }

    public static Long subStart(Long time,int len){
        String t = String.valueOf(time);
        t = t.substring(len);
        return Long.parseLong(t);
    }
    
    /**
     * 生成键值 
     * <br/>
     * 生成规则为年(2位)月(2)日(2)时(2)分(2)秒(2)毫秒(3)随机数(4)
     * <br/>
     * <br/>
     * <strong>注：不保证唯一性</strong>
     * @return
     */
    public static Long generateKey(){
    	return Long.parseLong(sdf.format(new Date()).concat(String.format("%04d", random.nextInt(10000))));
    }

    public static void main(String[] args) {
//        Long t = System.currentTimeMillis();
//        Long t2 = System.nanoTime();
//        Long groupId = 0l;
//        int rand = (int)(Math.random()*25);
//        t2 = t2 >> rand;
//        System.out.println("t1:"+t);
//        System.out.println("t2:"+t2);
//        if(t2.toString().length() > t.toString().length()){
//            groupId = t2 - t;
//        }else {
//            groupId = t - t2;
//        }
//        if(groupId.toString().length()>8){
//            groupId = Long.parseLong(groupId.toString().substring(groupId.toString().length()-8));
//            System.out.println(groupId);
//        }
//        System.out.println(groupId);
//
//        Long a = 1000l;
//        if(a.intValue() == 1000){
//            System.out.println(true);
//        }
//        System.out.println(GenerateUtil.getGroupId());
//        System.out.println("1498118922187".substring(5));

//        Long  a1 = 31743890l;
//        Long a2s =31743891l;
//        Long a3 = 31743892l;
//        System.out.println("a1"+a1%8);
//        System.out.println("a2:"+a2s%8);
//        System.out.println("a3:"+a3%8);
//        String a = "5504105_0";
//        System.out.println(a.substring(0,a.indexOf("_")));
        /*String a = "asdasda";
        System.out.println(a.contains("address"));*/
    	System.out.println(generateKey());
    	
    	Date start = new Date();
    	for (int i = 0 ;i < 10000; i++){
    		generateKey();
    	}
    	Date end = new Date();
    	System.out.println(end.getTime() - start.getTime());
    }
}
