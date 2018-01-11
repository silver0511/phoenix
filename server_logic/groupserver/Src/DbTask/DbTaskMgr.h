#ifndef __DB_TASK_GC_GROUP_H__
#define __DB_TASK_GC_GROUP_H__

#include "Task.h"
#include "rjson/JsonParser.h"
#include "const/PackDef.h"


class CGroupListRQ : public CTask
{
public:
	CGroupListRQ():CTask(){};
	virtual ~CGroupListRQ(){};
	virtual void run();

	CGroupListRQ(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(INT group_list_index, INT span)
	{
		m_group_list_index = group_list_index;
		m_span = span;
	}

public:
	INT m_group_list_index;
	INT m_span;
};

class CGroupCreateRQ : public CTask
{
public:
	string m_child_array;
public:
	CGroupCreateRQ():CTask() {};
	virtual ~CGroupCreateRQ(){};
	virtual void run();

	CGroupCreateRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(string group_name, INT big_msg_type, UINT64 msg_time, string operate_user_name,
				UINT64 message_id, UINT64 message_old_id, string child_array, INT group_max_count,
				INT group_add_max_count)
	{
		m_big_msg_type = big_msg_type;
		m_msg_time = msg_time;
		m_operate_user_name = operate_user_name;
		m_message_id = message_id;
		m_message_old_id = message_old_id;
		m_child_array = child_array;
		m_group_max_count = group_max_count;
		m_group_add_max_count = group_add_max_count;
	}

public:
	INT m_big_msg_type;
	UINT64 m_msg_time;
	string m_operate_user_name;
	UINT64 m_message_id;
	UINT64 m_message_old_id;
	INT m_group_max_count;
	INT m_group_add_max_count;
};

class CGroupDetailInfoRQ : public CTask
{
public:
	CGroupDetailInfoRQ():CTask(){};
	virtual ~CGroupDetailInfoRQ(){};
	virtual void run();

	CGroupDetailInfoRQ(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, INT group_member_index, INT span)
	{
		m_group_member_index = group_member_index;
		m_span = span;
		m_group_id = group_id;
	}

public:
	INT m_group_member_index;
	INT m_span;
	UINT64 m_group_id;
};

class CGroupKickUserRQ : public CTask
{
public:
	string m_child_array;
public:
	CGroupKickUserRQ():CTask() {};
	virtual ~CGroupKickUserRQ(){};
	virtual void run();

	CGroupKickUserRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, INT big_msg_type, UINT64 msg_time, string operate_user_name,
				UINT64 message_id, UINT64 message_old_id, string child_array)
	{
		m_group_id = group_id;
		m_msg_time = msg_time;
		m_operate_user_name = operate_user_name;
		m_message_id = message_id;
		m_message_old_id = message_old_id;
		m_child_array = child_array;
		m_big_msg_type = big_msg_type;
	}

public:
	INT m_big_msg_type;
	UINT64 m_msg_time;
	string m_operate_user_name;
	UINT64 m_message_id;
	UINT64 m_message_old_id;
	UINT64 m_group_id;
};

class CGroupChangeRemarkRQ : public CTask
{
public:
	string m_child_array;
public:
	CGroupChangeRemarkRQ():CTask() {};
	virtual ~CGroupChangeRemarkRQ(){};
	virtual void run();

	CGroupChangeRemarkRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, UINT64 msg_time, string operate_user_name,
				UINT64 message_id, UINT64 message_old_id, string child_array)
	{
		m_group_id = group_id;
		m_msg_time = msg_time;
		m_operate_user_name = operate_user_name;
		m_message_id = message_id;
		m_message_old_id = message_old_id;
		m_child_array = child_array;
	}

public:
	UINT64 m_msg_time;
	string m_operate_user_name;
	UINT64 m_message_id;
	UINT64 m_message_old_id;
	UINT64 m_group_id;
};

class CGroupAddUserRQ : public CTask
{
public:
	string m_child_array;
public:
	CGroupAddUserRQ():CTask() {};
	virtual ~CGroupAddUserRQ(){};
	virtual void run();

	CGroupAddUserRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, INT big_msg_type, UINT64 msg_time, string operate_user_name,
				UINT64 message_id, UINT64 message_old_id, string child_array, 
				string str_add_reason, UINT64 user_id_initial)
	{
		m_group_id = group_id;
		m_msg_time = msg_time;
		m_operate_user_name = operate_user_name;
		m_message_id = message_id;
		m_message_old_id = message_old_id;
		m_child_array = child_array;
		m_big_msg_type = big_msg_type;
		m_str_add_reason = str_add_reason;
		m_user_id_initial = user_id_initial;
	}

public:
	INT m_big_msg_type;
	UINT64 m_msg_time;
	string m_operate_user_name;
	UINT64 m_message_id;
	UINT64 m_message_old_id;
	UINT64 m_group_id;
	string m_str_add_reason;
	UINT64 m_user_id_initial;
};

class CGroupChangeNickNameRQ : public CTask
{
public:
	string m_child_array;
public:
	CGroupChangeNickNameRQ():CTask() {};
	virtual ~CGroupChangeNickNameRQ(){};
	virtual void run();

	CGroupChangeNickNameRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, UINT64 msg_time, string operate_user_name,
				UINT64 message_id, UINT64 message_old_id, string child_array)
	{
		m_group_id = group_id;
		m_msg_time = msg_time;
		m_operate_user_name = operate_user_name;
		m_message_id = message_id;
		m_message_old_id = message_old_id;
		m_child_array = child_array;
	}

public:
	UINT64 m_msg_time;
	string m_operate_user_name;
	UINT64 m_message_id;
	UINT64 m_message_old_id;
	UINT64 m_group_id;
};

class CGroupChangeGroupNameRQ : public CTask
{
public:
	string m_child_array;
public:
	CGroupChangeGroupNameRQ():CTask() {};
	virtual ~CGroupChangeGroupNameRQ(){};
	virtual void run();

	CGroupChangeGroupNameRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, UINT64 msg_time, string operate_user_name,
				UINT64 message_id, UINT64 message_old_id, string child_array)
	{
		m_group_id = group_id;
		m_msg_time = msg_time;
		m_operate_user_name = operate_user_name;
		m_message_id = message_id;
		m_message_old_id = message_old_id;
		m_child_array = child_array;
	}

public:
	UINT64 m_msg_time;
	string m_operate_user_name;
	UINT64 m_message_id;
	UINT64 m_message_old_id;
	UINT64 m_group_id;
};

class CGroupAddUserDefaultRQ : public CTask
{
public:
	CGroupAddUserDefaultRQ():CTask() {};
	virtual ~CGroupAddUserDefaultRQ(){};
	virtual void run();

	CGroupAddUserDefaultRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, UINT64 msg_time, string operate_user_name,
				UINT64 message_id, UINT64 message_old_id)
	{
		m_group_id = group_id;
		m_msg_time = msg_time;
		m_operate_user_name = operate_user_name;
		m_message_id = message_id;
		m_message_old_id = message_old_id;
	}

public:
	UINT64 m_msg_time;
	string m_operate_user_name;
	UINT64 m_message_id;
	UINT64 m_message_old_id;
	UINT64 m_group_id;
};

class CGroupAddUserAgreeRQ : public CTask
{
public:
	CGroupAddUserAgreeRQ():CTask() {};
	virtual ~CGroupAddUserAgreeRQ(){};
	virtual void run();

	CGroupAddUserAgreeRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, UINT64 msg_time, string operate_user_name,
				UINT64 message_id, UINT64 message_old_id)
	{
		m_group_id = group_id;
		m_msg_time = msg_time;
		m_operate_user_name = operate_user_name;
		m_message_id = message_id;
		m_message_old_id = message_old_id;
	}

public:
	UINT64 m_msg_time;
	string m_operate_user_name;
	UINT64 m_message_id;
	UINT64 m_message_old_id;
	UINT64 m_group_id;
};

class CGroupMessageStatusRQ : public CTask
{
public:
	CGroupMessageStatusRQ():CTask() {};
	virtual ~CGroupMessageStatusRQ(){};
	virtual void run();

	CGroupMessageStatusRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, BYTE message_status)
	{
		m_group_id = group_id;
		m_message_status = message_status;
	}

public:
	UINT64 m_group_id;
	BYTE m_message_status;
};

class CGroupSaveChangeRQ : public CTask
{
public:
	CGroupSaveChangeRQ():CTask() {};
	virtual ~CGroupSaveChangeRQ(){};
	virtual void run();

	CGroupSaveChangeRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id, BYTE save_type)
	{
		m_group_id = group_id;
		m_save_type = save_type;
	}

public:
	UINT64 m_group_id;
	BYTE m_save_type;
};

class CGroupGetUserRemarkNameRQ : public CTask
{
public:
	CGroupGetUserRemarkNameRQ():CTask() {};
	virtual ~CGroupGetUserRemarkNameRQ(){};
	virtual void run();

	CGroupGetUserRemarkNameRQ(SESSIONID aiCometID, USERID aiUserID, 
				INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 group_id)
	{
		m_group_id = group_id;
	}

public:
	UINT64 m_group_id;
};

class CGroupListIds : public CTask
{
public:
	CGroupListIds():CTask(){};
	virtual ~CGroupListIds(){};
	virtual void run();

	CGroupListIds(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(INT group_list_index, INT span)
	{
		m_group_list_index = group_list_index;
		m_span = span;
	}

public:
	INT m_group_list_index;
	INT m_span;
};

class CGroupScanRQ : public CTask
{
public:
	CGroupScanRQ():CTask(){};
	virtual ~CGroupScanRQ(){};
	virtual void run();

	CGroupScanRQ(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{

	}

	void SetArgvs(UINT64 user_id_share)
	{
		m_user_id_share = user_id_share;
	}

public:
	UINT64 m_user_id_share;
};

#define ClASS_PUBLIC_TASK(Class_Name) class Class_Name : public CTask\
						{\
							public:\
								Class_Name():CTask(){};\
								virtual ~Class_Name(){};\
								virtual void run();\
								Class_Name(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)\
									:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)\
								{\
								}\
						};\

ClASS_PUBLIC_TASK(CGroupLeaderChangeRQ)
ClASS_PUBLIC_TASK(CGroupRemarkDetailRQ)
ClASS_PUBLIC_TASK(CGroupGetBatchListRQ)

#endif