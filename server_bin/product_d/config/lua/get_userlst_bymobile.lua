local function get_platform(msg, mod)
		local sum = 0
		for i = 1,string.len(msg) do
				local asc = string.byte(msg,i)
				sum = sum+asc
		end
		local platform = sum % mod
		return platform
end

local usern = 1
local userlst = {}
for i = 3,KEYS[1] do
		local msg_platform = get_platform(KEYS[i],KEYS[2])
		local table_name = 'nim_mobile_lst_' .. msg_platform
		local userinfo = redis.call('hget',table_name,KEYS[i])
		if(userinfo and userinfo ~= nil and type(userinfo) == "string")
		then
				local platform = get_platform(userinfo,KEYS[2])
				local table_name = 'nim_userinfo_lst_' .. tostring(platform)
				userlst[usern] = redis.call('hget',table_name,tostring(userinfo))
				usern = usern+1
		end
end
return userlst