#ifndef __MYSQLDRIVERLIST_H
#define __MYSQLDRIVERLIST_H

#include "MySQLDriver.h"
#include "common/Debug_log.h"
/////////////////////////////////////////////////////////////////////////////
// CMySQLPool ��
class CMySQLDriverPool
{
public:
	CMySQLDriverPool();
	~CMySQLDriverPool();

	INT open(STRU_MYSQL_CONNECT_INFO * apList,uint32 auiCount);			
	void close();							

	CMySQLItem*	MallocItem(uint32 auiIndex);
	INT FreeItem(uint32 auiIndex,CMySQLItem* apItem);
	
	uint32 Size(){return muiCount;}

private:
	U9::CCriticalSection moCriticalSection;
	PMySQLPool* mpMySQLPoolList;
	uint32 muiCount;
};

class AutoReleaseMySQLItemExt
{
public:
	AutoReleaseMySQLItemExt(CMySQLDriverPool& aoPool,uint32 auiIndex)
	{
		muiIndex = auiIndex;
		mpPool = &aoPool;
		mpInstance = mpPool->MallocItem(muiIndex);


		if(NULL != mpInstance) 
		{
			if(NULL  == mpInstance->GetMySQL())
			{
				LOG_TRACE(3,0,__FUNCTION__,"mpInstance MySQL Is NULL "
					"Index="<<auiIndex);
			}
			else
			{
				MYSQL_STMT*	lpStmt =  mpInstance->StmtInit();
				if (NULL == lpStmt)
				{
					LOG_TRACE(3,0,__FUNCTION__," StmtInit fault "
						"Index="<<auiIndex);
					mpPool->FreeItem(muiIndex,mpInstance);
					mpInstance=NULL;
				}
			}
		}
		else
		{
			LOG_TRACE(3,0,__FUNCTION__,"mpInstance Is NULL "
				"Index="<<auiIndex);
		}
	}

	~AutoReleaseMySQLItemExt()
	{
		if(NULL != mpInstance)
		{	
			mpInstance->CleanAllNextResult();
			mpInstance->StmtClose();
			mpPool->FreeItem(muiIndex,mpInstance);
		}

	}
	inline  CMySQLItem& operator *()
	{
		return *mpInstance;
	}
	inline  CMySQLItem* operator ->()
	{
		return mpInstance;
	}
	inline BOOL Invalid()
	{
		return (BOOL)(NULL==mpInstance);
	}
	inline CMySQLItem* GetMySQLItem()
	{
		return mpInstance;
	}
private:
	CMySQLDriverPool*		mpPool;
	CMySQLItem*				mpInstance;
	uint32					muiIndex;
};
#endif	//__MYSQLDRIVERLIST_H