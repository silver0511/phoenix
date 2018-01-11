#ifndef __DATABASEMGR_H
#define __DATABASEMGR_H
#include "../MySQLDriver/MySQLDriverList.h"
#include "struct/SvrBaseStruct.h"
#include <map>
using namespace std;

struct STRFRIENDINFO
{
	std::string szuserid;
	std::string szremarkname;	
	std::string szsourcetype;
};

class CConfig;
class CDatabaseMgr 
{
public:
	CDatabaseMgr();
	~CDatabaseMgr();
public:
	INT open(CConfig &aoConfig);
	void close();
public:
	
private:
	
	std::string GetTableIndex(USERID aiUserID);
	uint32 GetContactDBPoolIndex(USERID aiUserID);
	
public:
	int32 GetFriendList(const std::string& szuserid, std::vector<STRFRIENDINFO>& friendlist);
	int32 GetFriendRemark(const std::string& szuserid, const std::string& szpeeruserid, const std::string& szremarkname);
	int32 GetFriendDel(const std::string& szuserid, const std::string& szpeeruserid);
	int32 GetFriendAdd(const std::string& szuserid, STRFRIENDINFO& peerinfo);
	
private:
	
	int32 IsFriend(const std::string& szuserid, const std::string& szpeeruserid);
	CMySQLDriverPool moContactDBPool;	//Contact���ݿ��	
};

//////////////////////////////////////////////////////////////////////////
//inline 
//�õ�centersys���ݿ������

#endif //__DATABASEMGR_H
