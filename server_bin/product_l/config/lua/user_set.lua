local function get_platform(msg, mod)
		local sum = 0
		for i = 1,string.len(msg) do
				local asc = string.byte(msg,i)
				sum = sum+asc
		end
		local platform = sum % mod
		return platform
end

local msg_platform = get_platform(KEYS[2],KEYS[1])
local table_str = 'nim_userinfo_lst_' .. tostring(msg_platform)
redis.call('hset',table_str,KEYS[2], KEYS[6])
if(KEYS[3] ~= '0' )
then
		local platform = get_platform(KEYS[3],KEYS[1])
		local table_name = 'nim_mobile_lst_' .. tostring(platform)
    redis.call('hset',table_name,KEYS[3],KEYS[2])
end
if(KEYS[4] ~= '0' )
then
		local platform = get_platform(KEYS[4],KEYS[1])
		local table_name = 'nim_name_lst_' .. tostring(platform)
    redis.call('hset',table_name,KEYS[4],KEYS[2])
end
if(KEYS[5] ~= '0' )
then
		local platform = get_platform(KEYS[5],KEYS[1])
		local table_name = 'nim_mail_lst_' .. tostring(platform)
    redis.call('hset',table_name,KEYS[5],KEYS[2])
end
return "0k"