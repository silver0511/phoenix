local function get_platform(msg, mod)
		local sum = 0
		for i = 1,string.len(msg) do
				local asc = string.byte(msg,i)
				sum = sum+asc
		end
		local platform = sum % mod
		return platform
end

local msg_platform = get_platform(KEYS[3],KEYS[1])
local table_str = 'nim_mobile_lst_' .. tostring(msg_platform)
local userinfo = redis.call('hget',table_str,KEYS[3])
if(userinfo and userinfo ~= nil and type(userinfo) == "string")
then
		local platform = get_platform(userinfo,KEYS[1])
		local table_name = 'nim_userinfo_lst_' .. tostring(platform)
		local str = redis.call('hget',table_name,tostring(userinfo))
		if(str and str ~= nil and type(str) == "string") then
				return "1"
		end
end

redis.call('hdel',table_str,KEYS[4])
msg_platform = get_platform(KEYS[2],KEYS[1])
table_str = 'nim_userinfo_lst_' .. tostring(msg_platform)
redis.call('hset',table_str,KEYS[2], KEYS[5])
if(KEYS[3] ~= '0' )
then
		local platform = get_platform(KEYS[3],KEYS[1])
		local table_name = 'nim_mobile_lst_' .. tostring(platform)
    redis.call('hset',table_name,KEYS[3],KEYS[2])
end
return "2"
