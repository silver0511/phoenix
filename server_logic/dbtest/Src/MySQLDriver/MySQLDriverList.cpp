#include "stdafx.h"
#include "MySQLDriverList.h"

/////////////////////////////////////////////////////////////////////////////
//CODBCPool ����
CMySQLDriverPool::CMySQLDriverPool():
muiCount(0)
,mpMySQLPoolList(NULL)
{

}
/////////////////////////////////////////////////////////////////////////////

CMySQLDriverPool::~CMySQLDriverPool()
{
}
int CMySQLDriverPool::open(STRU_MYSQL_CONNECT_INFO * apList,uint32 auiCount)
{
	LOG_TRACE(3,false,__FUNCTION__,"Begin Open Driver Pool:" <<auiCount <<"\n");			
	CMySQLPool *lpoMySQLPool=NULL;
	CRITICAL_SECTION_HELPER(moCriticalSection);
	close();
	mpMySQLPoolList = new PMySQLPool[auiCount];
	if(NULL == mpMySQLPoolList )
		throw;

	for(uint32 i =0;i<auiCount;i++)
	{
		lpoMySQLPool= new CMySQLPool;

		if(NULL ==lpoMySQLPool)
			throw;
		if(1 !=lpoMySQLPool->Open(apList[i]))
			return -1;
		mpMySQLPoolList[i]=lpoMySQLPool;
	}
	muiCount = auiCount;
	return 1;
}

void CMySQLDriverPool::close()
{
	LOG_TRACE(3,false,__FUNCTION__,"Begin Close Driver Pool:\n");
	CMySQLPool *lpoMySQLPool=NULL;
	moCriticalSection.Enter();
	//�������
	for (uint32 i = 0; i < muiCount; i ++)
	{
		lpoMySQLPool = mpMySQLPoolList[i];
		lpoMySQLPool->IncreaseRef();
		lpoMySQLPool->NeedRelease();
		lpoMySQLPool->Close();
		lpoMySQLPool->DecreaseRef();
		mpMySQLPoolList[i] = NULL;

	}
	if (mpMySQLPoolList)
	{
		delete [] mpMySQLPoolList;
		mpMySQLPoolList = NULL;
	}
	muiCount=0;
	moCriticalSection.Leave();

}
int CMySQLDriverPool::FreeItem(uint32 auiIndex,CMySQLItem* apItem)
{
	LOG_TRACE(5,false,__FUNCTION__,"FreeItem :" <<auiIndex <<"\n");
	if (NULL == apItem )
		return -1;

	CMySQLPool *lpoMySQLPool=NULL;
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		uint32 i=0;
		for (uint32 i=0;i<muiCount;i++)
		{
			if (mpMySQLPoolList[i]->GetDBIndex() == auiIndex)
			{
				lpoMySQLPool=mpMySQLPoolList[i];
				lpoMySQLPool->IncreaseRef();
				break;
			}
		}
		if (i >=  muiCount)
		{
			return NULL;
		}
	}

	lpoMySQLPool->FreeItem(apItem);
	lpoMySQLPool->DecreaseRef();

	return 0;
}
CMySQLItem* CMySQLDriverPool::MallocItem(uint32 auiIndex)
{
	LOG_TRACE(5,false,__FUNCTION__,"MallocItem :" <<auiIndex <<"\n");


	CMySQLPool *lpoMySQLPool=NULL;
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		uint32 i=0;
		for (uint32 i=0;i<muiCount;i++)
		{
			if (mpMySQLPoolList[i]->GetDBIndex() == auiIndex)
			{
				lpoMySQLPool=mpMySQLPoolList[i];
				lpoMySQLPool->IncreaseRef();
				break;
			}
		}
		if (i >=  muiCount)
		{
			return NULL;
		}
	}
	CMySQLItem*lpItem=lpoMySQLPool->MallocItem();
	lpoMySQLPool->DecreaseRef();
	return lpItem;
}
