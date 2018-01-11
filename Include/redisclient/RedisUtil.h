#ifndef __REDISUTIL_H
#define __REDISUTIL_H
#include "redisclient/RedisClient.hpp"
#include "const/redisdef.h"
#include "common/BaseConfig.h"
#include "common/Debug_log.h"
#include "LuaShaManager.h"
#include "../public/server_common/ServerUtilMgr.h"

#define	 DEF_REDIS_INI					"redis.ini"

struct STRU_REDIS_INFO
{
    std::string mstrIP;
	WORD		mwPort;
    INT         miTimeOut;
    INT         miMaxCnt;
    STRU_REDIS_INFO()
    {
		mstrIP = "";
        mwPort = 0;
        miTimeOut = 20;
        miMaxCnt = 1;
    }

	INT Check()
	{
		if(mstrIP == "" ||
				mwPort <= 0 ||
				miTimeOut <= 0 ||
				miMaxCnt <= 0)
		{
			return -1;
		}
		return 1;
	}
};

class RedisConfig:public CBaseConfig
{
public:
	RedisConfig();
	~RedisConfig();

	virtual BOOL Read();
	virtual BOOL DynamicRead();

private:

public:
	std::vector<STRU_REDIS_INFO> moRedisInfoList;
};
class RedisUtil
{
public:
	enum ENUM_REDIS_TYPE
	{
		REDIS_LOGIN = 0,
		REDIS_REG,
		REDIS_USER,
		REDIS_MESSAGE,
		REDIS_TRADE,
		REDIS_ASYNC_DB,
		REDIS_FRIEND,
		REDIS_GROUP_DB,

		REDIS_COUNT = REDIS_GROUP_DB + 1
	};
public:
	RedisUtil();
	~RedisUtil();

protected:
	CRedisClient *GetRedisClient(ENUM_REDIS_TYPE redis_type);
	bool ScriptLoadFile(ENUM_REDIS_TYPE redis_type, const string &config_file);
public:
	INT Open(std::vector<ENUM_REDIS_TYPE> open_redis_list);
	INT Close();
	//@REDIS_REG begin
	INT SetAccountVerityCode(uint64 account_type, const std::string target_user_id, std::string verify_info);
	std::string GetAccountVerityInfo(uint64 account_type, const std::string target_user_id);
	INT HSetAccountUser(uint64 account_type, const std::string target_user_id, USERID user_id);
	string HGetAccountUser(uint64 account_type, const std::string target_user_id);
	INT ExpireAaccountVerityCode(uint64 account_type, const std::string target_user_id, long expire_time);
	string SPopUserID();
	INT HSetRegPassWord(USERID user_id, string password_info);
	string HGetRegPassWord(USERID user_id);
	//@Redis REG end
    //@REDIS_LOGIN begin
    INT HSetLoginInfo(USERID user_id, int8 platform, const std::string &session);
	INT HDelLoginInfo(USERID user_id, int8 platform);
	std::string HGetLoginInfo(USERID user_id, int8 platform);
	INT HGetLoginInfo(std::vector<std::string>& userids, std::map<std::string, std::string>&values, int mod_index, int8 platform);
	INT HGetLoginInfo(std::vector<std::string>& userids, std::vector<std::string>&values, int mod_index, int8 platform);
	INT RPushLoginDetail(USERID user_id, std::string str_login);
	string LuaGetLoginServer(const string &json_user_ids, int8 platform);
	INT HsanLogininfo(int mod_index, int platform, std::string strCursor, long nCount, std::map<std::string, std::string> *pmapVal);
	//@Redis LOGIN end

	//@REDIS_USER begin
	std::string HGetUserInfo(USERID user_id, int platform);
	std::string HGetUserComplaintInfo(USERID user_id, int platform);
	INT HSetUserComplaintInfo(USERID user_id, int platform, const std::string &session);
	std::string LuaCheckUserMsg(const std::string &mobile,const std::string &name,const std::string &mail);
	std::string LuaHGetUserInfo(const std::string &user_msg);
	int LuaHGetUserLst(const std::vector<std::string> &user_lst, std::vector<std::string> *pvecVal);
	int LuaHGetUserLstByMobile(const std::vector<std::string> &user_lst, std::vector<std::string> *pvecVal);
	int LuaHAddUserInfo(USERID user_id, const std::string &session,const std::string &mobile ,const std::string &name,const std::string &mail);
	int LuaChangeMobile(USERID user_id, const std::string &session,const std::string &new_mobile,const std::string &old_mobile);
	int LuaChangeMail(USERID user_id, const std::string &session,const std::string &new_mail,const std::string &old_mail);
	INT HSetTokenInfo(const std::string &data_id , const std::string &session,int platform);
	INT HSetUserInfo(USERID user_id, int platform, const std::string &session);
	INT HSetDeviceToken(USERID user_id, const std::string &device_token);
	INT HDelDeviceToken(USERID user_id);
	std::string HGetDeviceToken(USERID user_id);
	std::string HGetDeviceUser(const std::string &device_token);
	INT HSetDeviceUser(const std::string &device_token, USERID user_id);
	INT HDelDeviceUser(const std::string &device_token);
	//@REDIS_USER end

	//@REDIS_MESSAGE begin
	INT DelUserOffLineChatInfo(USERID user_id);
	INT HDelUserOffLineChatInfo(USERID user_id, string strKey);
	INT DelUserOffLineListChatIds(USERID user_id);
	INT DelUserWebOffLineListChatIds(USERID user_id);
	INT HsetUserOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem, LONG& result);
	INT HMgetUserOffLineChatInfo(USERID user_id, const std::vector<std::string> &vecField, std::vector<std::string> *pvecVal);
	INT LRrangeOffLineChatInfo(USERID user_id, long nStart, long nStop, std::vector<std::string> *pvecVal);
	INT LRrangeWebOffLineChatInfo(USERID user_id, long nStart, long nStop, std::vector<std::string> *pvecVal);
	INT LremUserOfflineMsgId(USERID user_id, TEXTMSGID message_id);
	INT LremUserWebOfflineMsgId(USERID user_id, TEXTMSGID message_id);
	INT RpushUserOfflineMsgId(USERID user_id, TEXTMSGID message_id, LONG& nRedisResult);
	INT LpopUserOfflineMsgId(USERID user_id, string& strRedisResult);
	INT LRangeUserOfflineMsgId(USERID user_id, std::vector<std::string>& vector_list_ids, INT count_del);
	INT LTrimUserOfflineMsgId(USERID user_id, INT count_del);
	INT ZRrangeGroupOffLineChatInfoByScoreToMax(USERID user_id, unsigned long long message_id, INT nSpan, std::vector<std::string> *pvecVal);
	INT ZaddGroupOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem);
	INT ZremrangebyrankGroupOffLineChatInfo(USERID user_id, long nStart, long nStop);
	INT HUpdateUserTimeByType(USERID user_id, INT timetype, UINT64 time_update);
	INT HGetUserTimeByType(USERID user_id, INT timetype, std::string& str_time);
	INT SmembersGroupUserId(const UINT64& group_id, std::map<std::string, std::string>& pvecVal);
	INT HexistGroupUserId(const UINT64& group_id, UINT64& user_id, long& result);
	INT RPushScChatInfoToDbBack(const INT& partition, const std::string& strChat, LONG& nRedisResult);
	INT RPushGcChatInfoToDbBack(const INT& partition, const std::string& strChat, LONG& nRedisResult);
	INT HsetEcOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem, LONG& result);
	INT HMGetEcOffLineChatInfo(USERID user_id, const std::vector<std::string> &vecField, std::vector<std::string> *pvecVal);
	INT LlenGETEcOffLineCnt(USERID user_id, long & msgcnt);
	INT RpushEcOfflineMsgId(USERID user_id, TEXTMSGID message_id, LONG& nRedisResult);
	INT LRrangeEcOfflineMsgId(USERID user_id, long nStart, long nStop, std::vector<std::string> *pvecVal);
	INT DelEcOffLineChatInfo(USERID user_id);
	INT HExistEcOfflineMsg(USERID user_id, LONG& result);
	INT DelEcOfflineMsgId(USERID user_id);
	INT DelEcMutex(USERID user_id);
	INT EcOfflineMutexUnLock(USERID user_id);
	INT EcOfflineMutexLock(USERID user_id);
	INT LremEcOfflineMsgId(USERID user_id, TEXTMSGID message_id);
	INT HSetUserSingleChatStatus(USERID user_id, USERID op_user_id);
	INT HDelUserSingleChatStatus(USERID user_id, USERID op_user_id);
	INT HKeysUserSingleChatStatus(USERID user_id, vector<string>& vector_list_str_user_id);
	INT HGetUserSingleChatStatus(USERID user_id, USERID op_user_id, string& strResult);
	INT HgetGroupMsgId(UINT64& group_id, UINT64& user_id, string& strResult);
	INT HMsetGroupMsgId(UINT64& group_id, map<string, string>& mapGroupMsgId);
	INT RPushEcChatInfoToDbBack(const INT& partition, const std::string& strChat, LONG& nRedisResult);	
	//@REDIS_MESSAGE end

	//@REDIS_TRADE begin
	INT GetTradeLastWid(USERID ulcid, USERID ulbid, std::string& szwid);
	INT GetTradeLastSessionId(USERID ulcid, USERID ulbid, std::string & szsessionid);
	INT SetTradeLastWid(USERID ulcid, USERID ulbid, USERID ulwid, int sessionid);
	INT GetRecentCidList(const std::string &strKey, std::vector<std::string>& recentcidlist);	
    INT SetBusinessInfo(USERID ulbid, const std::string& strinfo);
    INT GetBusinessInfo(USERID ulbid, std::string& strinfo);
    INT SetBusinessPrompts(const std::map<std::string, std::string>& filed_value);
    INT GetBusinessPrompts(USERID ulbid, int ipromptstype, std::string & strprompts);
    INT GetBusinessPrompts(const std::set<std::string> &setField, std::map<std::string, std::string>& mapVal);
	//@REDIS_TRADE end

	//@REDIS_FRIEND begin
	INT ZaddFdMsg(USERID user_id, unsigned long long ultoken, const std::string &strElem);
	INT ZRrangeFdMsgByScore(USERID user_id, unsigned long long ultoken, INT nSpan, std::map<std::string, std::string> *pmapVal);
	INT ZcountFdMsg(USERID user_id, long & nCount);
	INT ZremrangebyrankFdMsg(USERID user_id, long nStart, long nStop);
	INT SetFdRequestTime(USERID user_id, USERID friend_id,const std::string & expiretime);
	INT GetFdRequestTime(USERID user_id, USERID friend_id,std::string & expiretime);
	INT DelFdRequestTime(USERID user_id,USERID friend_id);
	//@REDIS_FRIEND end

	//@REDIS_OFFCIAL begin
	INT ZremFansOffLineChatInfobyscore(USERID user_id, unsigned long long dMin, unsigned long long dMax);
	INT ZRrangeOffLineFanInfo(USERID user_id, long nStart, long nStop, std::vector<std::string> *pvecVal);
	INT LpushOffcialMsgdb(USERID user_id,const std::string &strElem, LONG& nRedisResult);
	INT ZaddOffcialOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem);
	INT ZaddFansOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem);
	INT ZRrangeOffcialOffLineChatInfoByScoreToMax(USERID user_id, unsigned long long message_id, INT nSpan, std::vector<std::string> *pvecVal);
	INT LpushFanMsgdb(USERID user_id, const std::string &strElem, LONG& nRedisResult);
	INT ZaddOffcialSysChatInfo(unsigned long long dScore, const std::string &strElem);
	INT ZRrangeOffcialSysMsgByScoreToMax(unsigned long long message_id, INT nSpan, std::vector<std::string> *pmapVal);
	INT ZaddFansSysMsgInfo(USERID user_id, unsigned long long dScore, const std::string &strElem);
	INT ZRrangeOffcialOneSysMsgByScoreToMax(USERID user_id, unsigned long long message_id, INT nSpan, std::vector<std::string> *pvecVal);
	INT HSetFansLst(const std::string &data_id ,const std::map<std::string, std::string> &mapFv);
	INT HSetFansLst(USERID user_id,const std::string &data_id ,const std::map<std::string, std::string> &mapFv);
	std::string HCheckFansId(const std::string &data_id ,USERID fans_id);
	std::string HCheckFansId(USERID user_id,const std::string &data_id ,USERID fans_id);
	INT ZaddOffcialOffLinePrivateChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem);
	INT ZRrangeOffcialOffLinePrivateChatInfo(USERID user_id, unsigned long long message_id, INT nSpan, std::vector<std::string> *pvecVal);
	INT ZremFansOffLinePrivateChatInfobyscore(USERID user_id, unsigned long long dMin, unsigned long long dMax);
	INT ZremFansOneSysMsgbyscore(USERID user_id, unsigned long long dMin, unsigned long long dMax);
	INT ZremrangebyrankSysMsg(const std::string &strKey,long nStart, long nStop);
	//@REDIS_OFFCIAL end
private:
	RedisConfig moRedisConfig;
    CRedisClient* mRedisClientList[REDIS_COUNT];
	LuaShaManager	mLuaShaManager;
	std::map<ENUM_REDIS_TYPE, vector<string>> mLuaFileMap;
	typedef std::map<ENUM_REDIS_TYPE, vector<string>>::iterator ITER_LUA_FILE;
};
#endif