local function get_platform(msg, mod)
		local sum = 0
		for i = 1,string.len(msg) do
				local asc = string.byte(msg,i)
				sum = sum+asc
		end
		local platform = sum % mod
		return platform
end

local msg_platform = get_platform(ARGV[1],KEYS[1])
local table_str = 'nim_userinfo_lst_' .. msg_platform
local userinfo = redis.call('hget',table_str,ARGV[1])
if(userinfo and userinfo ~= nil and type(userinfo) == "string") then
		userinfo = string.gsub(userinfo, " ", "")
		if(userinfo ~= "") then
				local val2 = cjson.decode(userinfo)
				if(type(val2) == "table") then
						return userinfo
				end
		end
end
table_str = 'nim_mobile_lst_' .. msg_platform
userinfo = redis.call('hget',table_str,ARGV[1])
if(userinfo and userinfo ~= nil and type(userinfo) == "string")
then
		local platform = get_platform(userinfo,KEYS[1])
		local table_name = 'nim_userinfo_lst_' .. tostring(platform)
		local str = redis.call('hget',table_name,tostring(userinfo))
		return str
end
table_str = 'nim_name_lst_' .. msg_platform
userinfo = redis.call('hget',table_str,ARGV[1])
if(userinfo and userinfo ~= nil and type(userinfo) == "string")
then
		local platform = get_platform(userinfo,KEYS[1])
		local table_name = 'nim_userinfo_lst_' .. tostring(platform)
		local str = redis.call('hget',table_name,tostring(userinfo))
		return str
end
table_str = 'nim_mail_lst_' .. msg_platform
userinfo = redis.call('hget',table_str,ARGV[1])
if(userinfo and userinfo ~= nil and type(userinfo) == "string")
then
		local platform = get_platform(userinfo,KEYS[1])
		local table_name = 'nim_userinfo_lst_' .. tostring(platform)
		local str = redis.call('hget',table_name,tostring(userinfo))
		return str
end
return userinfo
