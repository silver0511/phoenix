local keytmp = tostring(KEYS[1])..tostring("_")..tostring(KEYS[2])
local lastwidkey = string.format("nim_ec_lw_%s",tostring(keytmp))
local recentkey = string.format("nim_ec_rc_%s",tostring(KEYS[2]))

redis.call("hset",lastwidkey, "waitid", tostring(KEYS[3]))
redis.call("hset",lastwidkey, "sessionid", tostring(KEYS[4]))
redis.call("expire", lastwidkey,3600)

redis.call("lrem", recentkey, 0, tostring(KEYS[1]))
redis.call("lpush", recentkey, tostring(KEYS[1]))
redis.call("ltrim", recentkey, 0, 100)
return
