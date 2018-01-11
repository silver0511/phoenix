package com.qbao.im.api;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Map;
import java.util.Properties;

import org.mybatis.spring.boot.autoconfigure.MybatisAutoConfiguration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.jdbc.DataSourceTransactionManagerAutoConfiguration;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.EnableAspectJAutoProxy;

import com.qbao.im.api.config.FriendConfiguration;

/**
 * Hello world!
 */
@SpringBootApplication
@EnableAspectJAutoProxy(proxyTargetClass = true)
@EnableAutoConfiguration(exclude = {DataSourceAutoConfiguration.class, DataSourceTransactionManagerAutoConfiguration.class, MybatisAutoConfiguration.class})
@EnableConfigurationProperties({FriendConfiguration.class})
public class App {
    private static final Logger logger = LoggerFactory.getLogger(App.class);

    public static void main(String[] args) {
    	String configFileRootPath = null;
        Map<String, String> map = System.getenv();
        if (map.get("NIM_DB_CONF") != null) {
        	configFileRootPath = map.get("NIM_DB_CONF");
        }else if (System.getProperty("NIM_DB_CONF") != null){
        	configFileRootPath = System.getProperty("NIM_DB_CONF");
        }
        if (configFileRootPath != null){
        	FileInputStream in = null;
            try {
                
                File f = new File(configFileRootPath.concat("/im_jdbc.properties"));
                if (f.exists()){
                	Properties pro = new Properties();
	                in = new FileInputStream(configFileRootPath.concat("/im_jdbc.properties"));
	                pro.load(in);
	                System.setProperty("jdbc.im1.url", pro.getProperty("jdbc.im1.url"));
	                System.setProperty("jdbc.im1.username", pro.getProperty("jdbc.im1.username"));
	                System.setProperty("jdbc.im1.password", pro.getProperty("jdbc.im1.password"));
	                System.setProperty("jdbc.im2.url", pro.getProperty("jdbc.im2.url"));
	                System.setProperty("jdbc.im2.username", pro.getProperty("jdbc.im2.username"));
	                System.setProperty("jdbc.im2.password", pro.getProperty("jdbc.im2.password"));
	                System.setProperty("jdbc.im3.url", pro.getProperty("jdbc.im3.url"));
	                System.setProperty("jdbc.im3.username", pro.getProperty("jdbc.im3.username"));
	                System.setProperty("jdbc.im3.password", pro.getProperty("jdbc.im3.password"));
	                System.setProperty("jdbc.im4.url", pro.getProperty("jdbc.im4.url"));
	                System.setProperty("jdbc.im4.username", pro.getProperty("jdbc.im4.username"));
	                System.setProperty("jdbc.im4.password", pro.getProperty("jdbc.im4.password"));
	                System.setProperty("jdbc.imec1.url", pro.getProperty("jdbc.imec1.url"));
	                System.setProperty("jdbc.imec1.username", pro.getProperty("jdbc.imec1.username"));
	                System.setProperty("jdbc.imec1.password", pro.getProperty("jdbc.imec1.password"));
	                System.setProperty("jdbc.imec2.url", pro.getProperty("jdbc.imec2.url"));
	                System.setProperty("jdbc.imec2.username", pro.getProperty("jdbc.imec2.username"));
	                System.setProperty("jdbc.imec2.password", pro.getProperty("jdbc.imec2.password"));
	                System.setProperty("jdbc.imarch1.url", pro.getProperty("jdbc.imarch1.url"));
	                System.setProperty("jdbc.imarch1.username", pro.getProperty("jdbc.imarch1.username"));
	                System.setProperty("jdbc.imarch1.password", pro.getProperty("jdbc.imarch1.password"));
	                System.setProperty("jdbc.imarch2.url", pro.getProperty("jdbc.imarch2.url"));
	                System.setProperty("jdbc.imarch2.username", pro.getProperty("jdbc.imarch2.username"));
	                System.setProperty("jdbc.imarch2.password", pro.getProperty("jdbc.imarch2.password"));
	                in.close();
	                in = null;
                }else{
                	logger.warn("${NIM_DB_CONF}/im_jdbc.properties not exists, skip load");
                }
                f = new File(configFileRootPath.concat("/jdbc-common.properties"));
                if (f.exists()){
                	Properties pro = new Properties();
	                in = new FileInputStream(configFileRootPath.concat("/jdbc-common.properties"));
	                pro.load(in);
	                System.setProperty("jdbc.im.read.driver", pro.getProperty("jdbc.im.read.driver"));
	                System.setProperty("jdbc.im.max_wait", pro.getProperty("jdbc.im.max_wait"));
	                System.setProperty("jdbc.im.max_active", pro.getProperty("jdbc.im.max_active"));
	                System.setProperty("jdbc.im.initial_size", pro.getProperty("jdbc.im.initial_size"));
	                System.setProperty("jdbc.im.min_idle", pro.getProperty("jdbc.im.min_idle"));
	                System.setProperty("jdbc.im.timeBetweenEvictionRunsMillis", pro.getProperty("jdbc.im.timeBetweenEvictionRunsMillis"));
	                System.setProperty("jdbc.im.minEvictableIdleTimeMillis", pro.getProperty("jdbc.im.minEvictableIdleTimeMillis"));
	                System.setProperty("jbdc.im.validationQuery", pro.getProperty("jbdc.im.validationQuery"));
	                System.setProperty("jdbc.im.testWhileIdle", pro.getProperty("jdbc.im.testWhileIdle"));
	                System.setProperty("jdbc.im.testOnBorrow", pro.getProperty("jdbc.im.testOnBorrow"));
	                System.setProperty("jdbc.im.testOnReturn", pro.getProperty("jdbc.im.testOnReturn"));
	                System.setProperty("jdbc.im.removeAbandoned", pro.getProperty("jdbc.im.removeAbandoned"));
	                System.setProperty("jdbc.im.removeAbandonedTimeout", pro.getProperty("jdbc.im.removeAbandonedTimeout"));
	                System.setProperty("jdbc.im.logAbandoned", pro.getProperty("jdbc.im.logAbandoned"));
	                System.setProperty("jdbc.im.filters", pro.getProperty("jdbc.im.filters"));
                }else{
                	logger.warn("${NIM_DB_CONF}/jdbc-common.properties not exists, skip load");
                }
                f = new File(configFileRootPath.concat("/redis.properties"));
                if (f.exists()){
                	Properties pro = new Properties();
	                in = new FileInputStream(configFileRootPath.concat("/redis.properties"));
	                pro.load(in);
	                System.setProperty("redis.maxIdle", pro.getProperty("redis.maxIdle"));
	                System.setProperty("redis.minIdle", pro.getProperty("redis.minIdle"));
	                System.setProperty("redis.testOnBorrow", pro.getProperty("redis.testOnBorrow"));
                }else{
                	logger.warn("${NIM_DB_CONF}/redis.properties not exists, skip load");
                }
            } catch (IOException e) {
                logger.error("${NIM_DB_CONF}/im_jdbc.properties or ${NIM_DB_CONF}/jdbc-common.properties or ${NIM_DB_CONF}/redis.properties parse error e:{}", e);
            } finally{
            	if (in != null){
            		try{
            			in.close();
            		}catch(Exception e){
            			//swallow it
            		}
            	}
            }
        }
        logger.info("-------- dbserver netty starting! --------");
        SpringApplication app = new SpringApplication(App.class);
        app.setWebEnvironment(false);
        ApplicationContext ctx = app.run(args);
        String[] beanNames = ctx.getBeanDefinitionNames();
        Arrays.sort(beanNames);
        for (String beanName : beanNames) {
        	logger.debug(beanName);
        }
        logger.info("-------- dbserver netty started successful! --------");
    }
}
