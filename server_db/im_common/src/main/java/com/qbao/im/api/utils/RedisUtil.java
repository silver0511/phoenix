package com.qbao.im.api.utils;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.Tuple;
import redis.clients.jedis.exceptions.JedisConnectionException;
import redis.clients.jedis.exceptions.JedisException;

import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Created by tangxiaojun on 2017/6/27.
 */
public class RedisUtil {

    private Logger logger = LoggerFactory.getLogger(RedisUtil.class);

    private JedisPool pool;

    public void hset(String key, String field, String value) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            jedis.hset(key, field, value);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);

        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    public long hincrBy(String key, String field, long value) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            return jedis.hincrBy(key, field, value);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);

        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    public void hdel(String key, String... fields) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            jedis.hdel(key, fields);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    public JedisPool getPool() {
        return pool;
    }

    public void setPool(JedisPool pool) {
        this.pool = pool;
    }

    /**
     * 向key赋值
     *
     * @param key
     * @param value
     */
    public void set(String key, String value) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            jedis.set(key, value);

        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取key的值
     *
     * @param key
     * @return
     */
    public String get(String key) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            String value = jedis.get(key);
            return value;
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 将多个field - value(域-值)对设置到哈希表key中。
     *
     * @param key
     * @param map
     */
    public void hmset(String key, Map<String, String> map) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            jedis.hmset(key, map);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    public void sset(String key, String value) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            jedis.sadd(key, value);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    public Set<String> sget(String key) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            return jedis.smembers(key);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    public long sdel(String key, String value) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            return jedis.srem(key, value);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 将多个field - value(域-值)对设置到哈希表key中。
     *
     * @param key
     * @param fields
     */
    public List<String> hmget(String key, String... fields) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            return jedis.hmget(key, fields);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 给key赋值，并生命周期设置为seconds
     *
     * @param key
     * @param seconds
     *            生命周期 秒为单位
     * @param value
     */

    public void setex(String key, int seconds, String value) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            jedis.setex(key, seconds, value);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 给key赋值，并生命周期设置为seconds
     *
     * @param key
     * @param seconds
     *            生命周期 秒为单位
     * @param value
     */

    public Long setnx(String key, int seconds, String value) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            Long nx = jedis.setnx(key, value);
            jedis.expire(key, seconds);
            return nx;
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 为给定key设置生命周期
     *
     * @param key
     * @param seconds
     *            生命周期 秒为单位
     */
    public void expire(String key, int seconds) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            jedis.expire(key, seconds);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @param member
     * @return
     */
    public Double zscore(String key, String member) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            return jedis.zscore(key, member);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @param member
     * @return
     */
    public Long zrank(String key, String member) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            return jedis.zrank(key, member);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @param member
     * @return
     */
    public Long zrank(int dbIndex, String key, String member) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            return jedis.zrank(key, member);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @return
     */
    public Long zrevrank(int idx, String key, String member) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            if (idx != 0) {
                jedis.select(idx);
            }
            return jedis.zrevrank(key, member);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @return
     */
    public Set<Tuple> zrangeWithScores(int idx, String key, long start, long end) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            if (idx != 0) {
                jedis.select(idx);
            }
            return jedis.zrangeWithScores(key, start, end);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @return
     */
    public Set<Tuple> zrevrangeWithScores(int idx, String key, long start, long end) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            if (idx != 0) {
                jedis.select(idx);
            }
            return jedis.zrevrangeWithScores(key, start, end);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @return
     */
    public Set<Tuple> zrevrangeByScoreWithScores(int idx, String key, double max, double min, int offset, int count) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            if (idx != 0) {
                jedis.select(idx);
            }
            return jedis.zrevrangeByScoreWithScores(key, max, min, offset, count);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @return
     */
    public Set<Tuple> zrangeByScoreWithScores(int idx, String key, double min, double max, int offset, int count) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            if (idx != 0) {
                jedis.select(idx);
            }
            return jedis.zrangeByScoreWithScores(key, min, max, offset, count);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @return
     */
    public Long zcard(int idx, String key) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            if (idx != 0) {
                jedis.select(idx);
            }
            return jedis.zcard(key);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @return
     */
    public Double zscore(int idx, String key, String member) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            if (idx != 0) {
                jedis.select(idx);
            }
            return jedis.zscore(key, member);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 Sorted-Sets的值
     *
     * @param key
     * @return
     */
    public Long zcount(int idx, String key, double min, double max) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            if (idx != 0) {
                jedis.select(idx);
            }
            return jedis.zcount(key, min, max);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 List的值
     *
     * @param key
     * @return
     */
    public List<String> lrange(String key, long start, long end) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            return jedis.lrange(key, start, end);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 获取 List的值
     *
     * @param key
     * @return
     */
    public List<String> lrange(int dbIndex, String key, long start, long end) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            jedis.select(dbIndex);
            return jedis.lrange(key, start, end);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 从哈希表key中获取field的value
     *
     * @param key
     * @param field
     */

    public String hget(String key, String field) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            String value = jedis.hget(key, field);
            return value;
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 从哈希表key中获取field的value
     *
     * @param key
     * @param field
     */
    public String hget(int dbIndex, String key, String field) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            jedis.select(dbIndex);
            String value = jedis.hget(key, field);
            return value;
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    /**
     * 如果锁空闲立即返回 获取失败 一直等待,使用该方法一定要在使用完调用del方法解锁
     */
    public void lock(String key) {
        Jedis jedis = null;
        try {
            jedis = pool.getResource();
            do {
                logger.debug("lock key: " + key);
                Long i = jedis.setnx(key, key);
                if (i == 1) {
                    return;
                } else {
                    if (logger.isInfoEnabled()) {
                        String desc = jedis.get(key);
                        logger.info("key: " + key + " locked by another business：" + desc);
                    }
                }
                Thread.sleep(300);
            } while (true);
        } catch (JedisConnectionException je) {
            logger.error(je.getMessage(), je);
            pool.returnBrokenResource(jedis);
        } catch (Exception e) {
            logger.error(e.getMessage(), e);
        } finally {
            pool.returnResource(jedis);
        }
    }

    public void addList(String key,int seconds,String ... value){
        Jedis jedis = null;
        try{
            jedis = pool.getResource();
            expire(key,seconds);
        }catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }

    public void addList(String key,String ... value){
        Jedis jedis = null;
        try{
            jedis = pool.getResource();
            jedis.lpush(key,value);
        }catch (Exception e) {
            e.printStackTrace();
            logger.error("e:{}",e);
            if (jedis != null) {
                pool.returnBrokenResource(jedis);
            }
            throw new JedisException(e);
        } finally {
            if (pool != null) {
                pool.returnResource(jedis);
            }
        }
    }
}
