#ifndef __MYSQLDRIVER_H
#define __MYSQLDRIVER_H
#include <mysql.h>
#include <memory>
#include "common/CacheList.h"
#include "common/RefObj.h"
#include "common/Debug_log.h"
#include "MySQLRes.h"
struct STRU_MYSQL_CONNECT_INFO
{
	string mstrIP;
	WORD  mwPort;
	string mstrUserName;
	string mstrUserPwd;
	string mstrDBName;
	INT	 miConnectCount;
	uint32 miDBIndex;
};

/////////////////////////////////////////////////////////////////////////////
// CMySQLItem ��
class CMySQLItem
{
public:
	CMySQLItem();
	virtual ~CMySQLItem();

	//��ʼ������
	INT Open(const STRU_MYSQL_CONNECT_INFO &aoConnectInfo);	

	INT Close();

	INT Ping();

	INT RollBack();

	bool IsDBExist(const char* aszDBName);

	MYSQL_STMT* StmtInit();

	INT ExecuteDirect(const char* aszQuery, unsigned long auiLength);

	INT SelectDB(const char* aszDBName);

	INT StmtPrepare(const char* aszQuery, unsigned long auiLength);

	INT StmtBindParam(MYSQL_BIND* aoBind);

	INT StmtExecute();

	INT StmtClose();

	const char* GetCharacterSetName();

	INT SetCharacterSet(const char* aszCsName);

	const char* GetLastErrMsg(){return mszLastErrMsg;}

	MYSQL* GetMySQL(){return mpMySQL;}
	MYSQL_STMT* GetStmt(){return mpStmt;}

	INT GetAffectedRows();

	auto_ptr<CMySQLRes> StoreResult();

	void CleanAllNextResult();

private:

	INT Connect(const char *aszHost, const char *aszUser, const char *aszPasswd,
		const char *aszDb, unsigned int auiPort);

	MYSQL* mpMySQL;
	MYSQL_STMT * mpStmt;

	static const uint MAXERRMSGLENGTH = 4096;
	char mszLastErrMsg[MAXERRMSGLENGTH];
};

/////////////////////////////////////////////////////////////////////////////
class CMySQLPool:public CRefObject
{
public:
	CMySQLPool();
	virtual ~CMySQLPool();
	INT Open(const STRU_MYSQL_CONNECT_INFO &aoConnectInfo);	

	void Close();

	CMySQLItem*	MallocItem();


	inline void FreeItem(CMySQLItem* apItem)
	{
		moMySQLList.AddTail(apItem);
	}

	inline INT Size(){return moMySQLList.size();}

	inline uint32 GetDBIndex(){return miDBIndex;}
	
private:
	CSafeStaticQueue<CMySQLItem> moMySQLList;

	BOOL mbActive;
	uint32 miDBIndex;
};


typedef CMySQLPool* PMySQLPool;

class AutoReleaseMySQLItem
{
public:
	inline AutoReleaseMySQLItem(CMySQLPool& aoPool)
	{
		mpoPool = &aoPool;
		mpInstance = mpoPool->MallocItem();
	}

	inline ~AutoReleaseMySQLItem()
	{
		if(NULL != mpInstance)
			mpoPool->FreeItem(mpInstance);
	}

	inline operator CMySQLItem*()
	{
		return mpInstance;
	}

	inline BOOL Invalid()
	{
		return (BOOL)(mpInstance==NULL);
	}
private:
	CMySQLPool*		mpoPool;
	CMySQLItem*		mpInstance;
};


#endif	// __MYSQLDRIVER_H_