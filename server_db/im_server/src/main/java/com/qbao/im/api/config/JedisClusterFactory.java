package com.qbao.im.api.config;

import org.apache.commons.pool2.impl.GenericObjectPoolConfig;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.FactoryBean;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.core.io.Resource;
import redis.clients.jedis.HostAndPort;
import redis.clients.jedis.JedisCluster;

import java.io.FileInputStream;
import java.util.HashSet;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.util.regex.Pattern;

/**
 * Created by tangxiaojun on 2017/6/28.
 */
public class JedisClusterFactory implements FactoryBean<JedisCluster>, InitializingBean {

    private Logger logger = LoggerFactory.getLogger(JedisClusterFactory.class);

    private Resource addressConfig;
    private String addressKeyPrefix;

    private JedisCluster jedisCluster;
    private Integer timeout;
    private Integer maxRedirections;
    private GenericObjectPoolConfig genericObjectPoolConfig;

    private Pattern p = Pattern.compile("^.+[:]\\d{1,5}\\s*$");

    @Override
    public void afterPropertiesSet() throws Exception {
        Set<HostAndPort> haps = this.parseHostAndPort();

        jedisCluster = new JedisCluster(haps, timeout, maxRedirections, genericObjectPoolConfig);
    }

    @Override
    public JedisCluster getObject() throws Exception {
        return jedisCluster;
    }

    @Override
    public Class<?> getObjectType() {
        return (this.jedisCluster != null ? this.jedisCluster.getClass() : JedisCluster.class);
    }

    @Override
    public boolean isSingleton() {
        return true;
    }

    private Set<HostAndPort> parseHostAndPort() throws Exception {
        try {
            Properties prop = new Properties();
            Map<String, String> map = System.getenv();
            if (map.get("NIM_DB_CONF") != null) {
                try {
                    prop.load(new FileInputStream((map.get("NIM_DB_CONF").concat("/redis.properties"))));
                } catch (Exception e) {
                    logger.error("env redis file not found e:{}", e);
                    e.printStackTrace();
                }
            } else if (System.getProperty("NIM_DB_CONF") != null){
            	try {
                    prop.load(new FileInputStream((System.getProperty("NIM_DB_CONF").concat("/redis.properties"))));
                } catch (Exception e) {
                    logger.error("properties redis file not found e:{}", e);
                    e.printStackTrace();
                }
            }else {
                try {
                    prop.load(this.addressConfig.getInputStream());
                } catch (Exception e) {
                    logger.error("redis file not found e:{}", e);
                    e.printStackTrace();
                }
            }

            Set<HostAndPort> haps = new HashSet<HostAndPort>();
            for (Object key : prop.keySet()) {

                if (!((String) key).startsWith(addressKeyPrefix)) {
                    continue;
                }


                String val = (String) prop.get(key);
                String[] addressArr = val.split(",");
                for (String add : addressArr) {
                    boolean isIpPort = p.matcher(add).matches();
                    if (!isIpPort) {
                        throw new IllegalArgumentException("ip 或 port 不合法");
                    }
                    String[] ipAndPort = add.split(":");

                    HostAndPort hap = new HostAndPort(ipAndPort[0], Integer.parseInt(ipAndPort[1]));
                    haps.add(hap);
                }
            }

            return haps;
        } catch (IllegalArgumentException ex) {
            throw ex;
        } catch (Exception ex) {
            throw new Exception("解析 jedis 配置文件失败", ex);
        }
    }

    public void setAddressConfig(Resource addressConfig) {
        this.addressConfig = addressConfig;
    }

    public void setTimeout(int timeout) {
        this.timeout = timeout;
    }

    public void setMaxRedirections(int maxRedirections) {
        this.maxRedirections = maxRedirections;
    }

    public void setAddressKeyPrefix(String addressKeyPrefix) {
        this.addressKeyPrefix = addressKeyPrefix;
    }

    public void setGenericObjectPoolConfig(GenericObjectPoolConfig genericObjectPoolConfig) {
        this.genericObjectPoolConfig = genericObjectPoolConfig;
    }
}
