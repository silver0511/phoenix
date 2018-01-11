// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
// 
// 文件名称: DBI.h
// 创 建 人: 史云杰
// 文件说明:  MySQL C　API封装连接池头文件
// $_FILEHEADER_END *****************************

#include "common/System.h"
#include "mysqldriver/DBI.h"
#include "mysqldriver/MySQLDriverPool.h"
#include "common/Debug_log.h"

namespace U9
{
	IDriverPool* GetDBConnectionByName(const DBITYPE& auDBItype)
	{
		switch(auDBItype)
        {
        case MYSQL_DBI:
            {
                return new CMySQLDriverPool() ;
            }
            break ;

        default :
            break ;
        }
		return NULL;
	}

	void FreeDBConnection(IDriverPool* apDBIConn)
	{
		apDBIConn->Release() ;
	}

    //////////////////////////////////////////////////////////////////////////
    AutoReleaseConnItemExt::AutoReleaseConnItemExt()
    {
        mpConnItem = NULL ;
    }

    AutoReleaseConnItemExt::AutoReleaseConnItemExt(uint64 auid, uint32 auDBindex, IDriverPool* apDriverPool)
    {
        muid = auid;
        muDBindex = auDBindex ;
        mpDriverPool = apDriverPool ;
        mpConnItem = apDriverPool->MallocItem(auid, auDBindex);
    }

    AutoReleaseConnItemExt::~AutoReleaseConnItemExt()
    {
        if(NULL != mpConnItem)
        {		
            mpConnItem->StmtClose();
            mpDriverPool->FreeItem(muid, muDBindex, mpConnItem);
            mpConnItem = NULL ;
        }
    }

    IConnItem& AutoReleaseConnItemExt::operator *()
    {
        return *mpConnItem;
    }

    IConnItem* AutoReleaseConnItemExt::operator ->()
    {
        return mpConnItem;
    }

    bool AutoReleaseConnItemExt::IsValid()
    {
        return (bool)(NULL == mpConnItem);
    }

    IConnItem* AutoReleaseConnItemExt::GetConnItem()
    {
        return mpConnItem;
    }

    //////////////////////////////////////////////////////////////////////////
    AutoReleaseRecordSet::AutoReleaseRecordSet()
    {
        mpRecSet = NULL ;
    }

    AutoReleaseRecordSet::AutoReleaseRecordSet(IRecordSet* apRecSet)
    {
        mpRecSet = apRecSet ;
    }

    AutoReleaseRecordSet::~AutoReleaseRecordSet()
    {
        if (NULL != mpRecSet)
        {
            delete mpRecSet ;
            mpRecSet = NULL ;
        }    
    }

    IRecordSet& AutoReleaseRecordSet::operator *()
    {
        return *mpRecSet;
    }

    IRecordSet* AutoReleaseRecordSet::operator ->()
    {
        return mpRecSet;
    }

    bool AutoReleaseRecordSet::IsValid()
    {
        return (bool)(NULL != mpRecSet);
    }

    IRecordSet* AutoReleaseRecordSet::GetRecordSet()
    {
        return mpRecSet;
    }

    //////////////////////////////////////////////////////////////////////////
    
}

