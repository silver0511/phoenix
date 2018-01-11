package com.qbao.im.api.controller;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.core.MediaType;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;

import com.mysql.jdbc.StringUtils;

import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisCluster;
import redis.clients.jedis.ScanParams;
import redis.clients.jedis.ScanResult;

/**
 * Created by anderson on 2017/9/27.
 */
@Controller
@Path("/api/manage/redis")
public class ManageRedisController {

    private Logger logger = LoggerFactory.getLogger(ManageRedisController.class);
    
    private static final Integer DEFAULT_SHOW_KEYS_NUM = 10;

    
    @Autowired
	JedisCluster jedisCluster;

    @GET
    @Path("/getInfo")
    @Produces(MediaType.APPLICATION_JSON)
    public KeyInfoResult getInfo(@QueryParam("keys")String keys, @QueryParam("key_count")Integer keyCount, @QueryParam("show_content")Boolean showContent) {
    	if (StringUtils.isNullOrEmpty(keys)){
			throw new RuntimeException( "missing param[keys]");
		}
    	if (keyCount == null){
    		keyCount = DEFAULT_SHOW_KEYS_NUM;
    	}
    	if (showContent == null){
    		showContent = Boolean.FALSE;
    	}
    	if (keys.contains("*")){
    		return getMultiKeyInfo(keys, keyCount, showContent);
    	}else{
    		return getSingleKeyInfo(keys, showContent);
    	}
    }
    
    private KeyInfoResult getSingleKeyInfo(String key, Boolean showContent){
    	KeyInfoResult result = new KeyInfoResult();
    	String content = jedisCluster.get(key);
    	if (content != null){
    		result.setCount(1);
    		Set<String> keys = new HashSet<String>();
    		keys.add(key);
    		result.setKeys(keys);
    		if (showContent.booleanValue() == true){
    			Map<String, String> data = new HashMap<String, String>();
    			data.put(key, content);
    			result.setData(data);
    		}
    	}else{
    		result.setCount(0);
    	}
    	return result;
    }
    
    private KeyInfoResult getMultiKeyInfo(String keys, Integer keyCount, Boolean showContent){
    	ScanParams sp = new ScanParams();
		sp.match(keys);
		String cursor = "0";
		Set<String> scanedKeys = new HashSet<String>();
		
		for (String key : jedisCluster.getClusterNodes().keySet()){
			Jedis jedis = jedisCluster.getClusterNodes().get(key).getResource();
			
			cursor = "0";
			do{
				ScanResult<String> sr = jedis.scan(cursor, sp);
				cursor = sr.getStringCursor();
				for (String v : sr.getResult()){
					scanedKeys.add(v);
				}
			}while(Integer.valueOf(cursor) > 0);
		}
		
		KeyInfoResult result = new KeyInfoResult();
		result.setCount(scanedKeys.size());
		if (showContent.booleanValue() == true){
			int counter = 0;
			Map<String, String> data = new HashMap<String, String>();
			for (String key : scanedKeys){
				String type = jedisCluster.type(key).toLowerCase();
				if ("string".equals(type)){
					data.put(key, jedisCluster.get(key));
				}else if ("set".equals(type)){
					data.put(key, jedisCluster.srandmember(key) + " ...... ");
				}else if ("hash".equals(type)){
					data.put(key, jedisCluster.hget(key, new ArrayList<String>(jedisCluster.hkeys(key)).get(0)) + " ...... ");
				}else if ("list".equals(type)){
					data.put(key, jedisCluster.lindex(key, 0) + " ...... ");
				}else if ("zset".equals(type)){
					data.put(key, jedisCluster.zrange(key, 0, 0) + " ...... ");
				}else{
					data.put(key, "UNKOWN TYPE!!!");
				}
				
				counter ++;
				if (counter >= keyCount){
					break;
				}
			}
			result.setData(data);
		}else{
			int counter = 0;
			Set<String> tmpKeys = new HashSet<String>();
			for (String key : scanedKeys){
				tmpKeys.add(key);
				counter++;
				if (counter >= keyCount){
					break;
				}
			}
			result.setKeys(tmpKeys);
		}
		
		return result;
    }

    
    public static class KeyInfoResult{
    	private int count;
    	private Set<String> keys;
    	private Map<String, String> data;
		public int getCount() {
			return count;
		}
		public void setCount(int count) {
			this.count = count;
		}
		public Set<String> getKeys() {
			return keys;
		}
		public void setKeys(Set<String> keys) {
			this.keys = keys;
		}
		public Map<String, String> getData() {
			return data;
		}
		public void setData(Map<String, String> data) {
			this.data = data;
		}
    }
    
    @GET
    @Path("/clearKeys")
    @Produces(MediaType.APPLICATION_JSON)
    public String clearKeys(@QueryParam("keys")String keys){
    	if (StringUtils.isNullOrEmpty(keys)){
    		throw new RuntimeException("missing param[keys]");
    	}
    	
    	long counter = 0;
    	
    	//删除指定键
    	if (keys.contains("*") == false){
    		counter = jedisCluster.del(keys);
    		return "removed " + counter + " keys";  
    	}
    	ScanParams sp = new ScanParams();
		sp.match(keys);
    	String cursor = "0";
    	for (String key : jedisCluster.getClusterNodes().keySet()){
			Jedis jedis = jedisCluster.getClusterNodes().get(key).getResource();
			
			cursor = "0";
			do{
				ScanResult<String> sr = jedis.scan(cursor, sp);
				cursor = sr.getStringCursor();
				for (String v : sr.getResult()){
					long r = jedisCluster.del(v);
					counter += r;
				}
			}while(Integer.valueOf(cursor) > 0);
    	}
    	
    	return "removed " + counter + " keys";    	
    }
    
    @GET
    @Path("/addKeys")
    @Produces(MediaType.APPLICATION_JSON)
    public String addKeys(@QueryParam("key_prefix")String keyPrefix, @QueryParam("count")Integer count){
    	if (StringUtils.isNullOrEmpty(keyPrefix)){
    		throw new RuntimeException("missing param[keys]");
    	}
    	
    	if (count == null){
    		count = 1000;
    	}else if (count.intValue() < 1){
    		throw new RuntimeException("param[count] must > 0");
    	}
    	for (int i = 0 ; i < count ; i++){
    		jedisCluster.set(keyPrefix + i, String.valueOf(i));
    	}
    	
    	return "created " + count + " records!";
    }
}
