local function get_platform(msg, mod)
		local sum = 0
		for i = 1,string.len(msg) do
				local asc = string.byte(msg,i)
				sum = sum+asc
		end
		local platform = sum % mod
		return platform
end

local userinfo = 1
local userlst = {}
for i = 3,KEYS[1] do
		local msg_platform = get_platform(KEYS[i],KEYS[2])
		local table_name = 'nim_userinfo_lst_' .. msg_platform
		userlst[userinfo] = redis.call('hget',table_name,KEYS[i])
		userinfo = userinfo+1
end
return userlst