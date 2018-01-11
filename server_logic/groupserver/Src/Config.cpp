#include "stdafx.h"
#include "Config.h"
#include "rjson/JsonParser.h"

CConfig::CConfig():CServerConfigExt()
{
	// 讨论组默认最多可以拉50人
}

CConfig::~CConfig()
{

}


BOOL CConfig::Read()
{
	CServerConfigExt::Read();
	string lstrTemp="";

	//读取本机监听IP和服务器
//	moIniFile.GetValue("ServerIP","LocalServer",lstrTemp,"127.0.0.1");
	goLocalServerInfo.mlServerIP=inet_addr(GetIntranetIP().c_str());
//	moIniFile.GetValue("ServerPort","LocalServer",goLocalServerInfo.mwServerPort,0);
//	goLocalServerInfo.mwServerPort=htons(goLocalServerInfo.mwServerPort);
	moIniFile.GetValue("TaskThreadCnt", "LocalServer", m_taskthreadcnt, 50);
    moIniFile.GetValue("statinterval","Server",m_statinterval,0);
	
	
	goLocalServerInfo.mwServerType = GC_SERVER;
	goLocalServerInfo.GenServerID();

	RJ_CREATE_EMPTY_DOCUMENT(l_document)
    bool result = JsonParse::parse_file(l_document, "./config/group_type_info.json");
    if(!result || l_document.IsNull())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "read json file invalid");
        return false;
    }

    RJsonValue &json_data_list = JsonParse::get(l_document, "data");
    if(json_data_list.IsNull())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "read json data not exist");
        return false;
    }

	RJ_SIZE_TYPE data_count = JsonParse::count(json_data_list);
	if(data_count <= 0)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "read json data not exist");
        return false;
    }

    for (RJ_SIZE_TYPE index = 0; index < data_count; ++index)
    {
        RJsonValue& group_type_info = JsonParse::at(json_data_list, index);
        
		GroupTypeInfo group_info;
		JsonParse::get<int>(group_type_info, "group_add_max_count", group_info.group_add_max_count);
		JsonParse::get<int>(group_type_info, "group_type", group_info.group_type);
		JsonParse::get<int>(group_type_info, "group_is_show", group_info.group_is_show);
		JsonParse::get<int>(group_type_info, "group_max_count", group_info.group_max_count);
		
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "group_count = " << group_info.group_max_count);
		mapGroupTypeInfo[group_info.group_type] = group_info;
    }

	//后台服务器
	return true;
}

BOOL CConfig::DynamicRead()
{
	int32 liLogLevel = 7;
	moIniFile.GetValue("LogLevel","Server",liLogLevel,7);
	moIniFile.GetValue("statinterval","Server",m_statinterval,0);
	LOG_SET_LEVEL(liLogLevel);
	return TRUE;
}


