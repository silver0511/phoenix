--
-- Created by IntelliJ IDEA.
-- User: shiyunjie
-- Date: 2017/11/2
-- Time: 下午3:56
-- To change this template use File | Settings | File Templates.
--

local pre_table_name = KEYS[1]
local platform = KEYS[2]
local mod_count = KEYS[3]
local str_user_ids = KEYS[4]

local function get_table_name(user_id)
    local mod_index = user_id % mod_count
    local l_table_name = pre_table_name .. mod_index .. '_' .. platform
    return l_table_name
end

if(not pre_table_name or not platform or not mod_count or not str_user_ids) then
    return ""
end

local user_id_list = cjson.decode(str_user_ids)

if(user_id_list == nil or _G.next(user_id_list) == nil) then
    return ""
end

local online_user_map = {}
local online_server_list = {}
local offline_user_list = {}
for _, user_id in pairs(user_id_list) do
    local l_table_name = get_table_name(user_id)
    local l_server_id = redis.call('hget', l_table_name, user_id)
    if(l_server_id and l_server_id ~= nil) then
        if(online_user_map[l_server_id] == nil) then
            online_user_map[l_server_id] = {}
            table.insert(online_server_list, l_server_id)
        end
        table.insert(online_user_map[l_server_id], user_id)
    else
        table.insert(offline_user_list, user_id)
    end
end

if(_G.next(online_server_list) == nil) then
    table.insert(online_server_list, "-1")
end
if(_G.next(online_user_map) == nil) then
    online_user_map["-1"] = {}
    table.insert(online_user_map["-1"], -1)
end
if(_G.next(offline_user_list) == nil) then
    table.insert(offline_user_list, -1)
end
local ret_map = {}
table.insert(ret_map, online_server_list)
table.insert(ret_map, online_user_map)
table.insert(ret_map, offline_user_list)

return cjson.encode(ret_map)




