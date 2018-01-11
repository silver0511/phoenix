// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
// 
// 文件名称: MySQLDriverPool.cpp
// 创 建 人: 史云杰
// 文件说明:  MySQL C　API封装连接池实现文件
// $_FILEHEADER_END *****************************
#include "common/System.h"
#include "mysqldriver/MySQLDriverPool.h"

/////////////////////////////////////////////////////////////////////////////
//CODBCPool 构造
CMySQLDriverPool::CMySQLDriverPool()
{
    mbIsRun = false ;
    muKeepLiveTimeStamp = time(NULL) ;
}
/////////////////////////////////////////////////////////////////////////////
//CODBCPool 析构
CMySQLDriverPool::~CMySQLDriverPool()
{
    Close();
}

void CMySQLDriverPool::Release() 
{
    mbIsRun = false ;

    delete this ;
}

void CMySQLDriverPool::init()
{
    Close() ;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： Init
// 函数参数： NULL
// 返 回 值： 0代表成功，小于0代表失败
// 函数说明： 打开数据库
// $_FUNCTION_END ********************************
uint32 CMySQLDriverPool::Open(const CDBInfoList& aDBList)
{
    LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "Begin Open Driver Pool:" <<aDBList.size());
    for(uint32 i =0;i < aDBList.size(); i++)
    {
       Open(aDBList[i]);
    }
    return moMySQLPoolList.size();
}

uint32 CMySQLDriverPool::Open(const CDBinfo& aDBInfo)
{
    LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "Begin Open Driver Pool:");
    //moCriticalSection.Lock();
	CRITICAL_SECTION_HELPER(moCriticalSection);
    CMySQLPoolListIter it = moMySQLPoolList.find(aDBInfo.mudbindex) ;
    if (it != moMySQLPoolList.end())
    {
        return 0 ;
    }
 

    CMySQLPool *lpMySQLPool = new CMySQLPool;
    if(NULL ==lpMySQLPool)
    {
        return -1;
    }
    if(0 == lpMySQLPool->Open(aDBInfo.modbip.c_str(), aDBInfo.mudbport, 
        aDBInfo.modbname.c_str(), aDBInfo.modbuser.c_str(), 
        aDBInfo.modbpwd.c_str(), aDBInfo.muDBtype, aDBInfo.mudbindex, aDBInfo.mupoolsize))
    {
        LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error Open fail:"
            << " IP:("<<aDBInfo.modbip.c_str()
            <<","<< aDBInfo.mudbport<<")"
            <<" Index:"<<aDBInfo.mudbindex
            <<" DB:"<<aDBInfo.modbname.c_str()
            <<" user:"<<aDBInfo.modbuser.c_str()
            <<" pwd:"<<aDBInfo.modbpwd.c_str()
            <<" DBtype:"<<aDBInfo.muDBtype
            <<" poolsize:"<<aDBInfo.mupoolsize
            );	
        delete lpMySQLPool;
        lpMySQLPool = NULL;
        return -1;
    }

    std::pair<CMySQLPoolListIter, bool> pr = moMySQLPoolList.insert(make_pair(aDBInfo.mudbindex, lpMySQLPool)) ;
    if(pr.second == false)
    {
        LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error dbpool exist:"
            << " IP:("<<aDBInfo.modbip.c_str()
            <<","<< aDBInfo.mudbport<<")"
            <<" Index:"<<aDBInfo.mudbindex
            <<" DB:"<<aDBInfo.modbname.c_str()
            <<" user:"<<aDBInfo.modbuser.c_str()
            <<" pwd:"<<aDBInfo.modbpwd.c_str()
            <<" DBtype:"<<aDBInfo.muDBtype
            <<" poolsize:"<<aDBInfo.mupoolsize
            );	
        lpMySQLPool->Close();
        delete lpMySQLPool;
        lpMySQLPool = NULL;
        return 0 ;
    }
    lpMySQLPool->IncreaseRef();
    return 1;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： ClosePool
// 函数参数： NULL
// 返 回 值： NULL
// 函数说明： 关闭数据池
// $_FUNCTION_END ********************************
void CMySQLDriverPool::Close()
{
    LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "Begin Close Driver Pool.");
    CRITICAL_SECTION_HELPER(moCriticalSection);
    //清除连接
    for(CMySQLPoolListIter it = moMySQLPoolList.begin() ;
    it != moMySQLPoolList.end();++it)
    {
        CMySQLPool *lpMySQLPool = it->second;
        if(NULL != lpMySQLPool)
        {
            lpMySQLPool->Close();
            delete lpMySQLPool;
            lpMySQLPool = NULL;
        }
        //AutoReleaseMySQLPool(it->second);
    }
    moMySQLPoolList.clear() ;
    mbIsRun = false ;
}

void CMySQLDriverPool::Close(uint32 aiDBindex)
{
    LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "Begin Close Driver Pool");
    CMySQLPool *lpMySQLPool = NULL;
    CRITICAL_SECTION_HELPER(moCriticalSection);
    CMySQLPoolListIter it = moMySQLPoolList.find(aiDBindex) ;
    if (it == moMySQLPoolList.end())
    {
        return ;
    }
    lpMySQLPool = it->second;
    moMySQLPoolList.erase(it) ;
    AutoReleaseMySQLPool lpMySQLPoolItem(lpMySQLPool);
}

CMySQLPool *CMySQLDriverPool::getMySQLPool(const uint32 aiIndex)
{
    CRITICAL_SECTION_HELPER(moCriticalSection);
    CMySQLPoolListIter it = moMySQLPoolList.find(aiIndex) ;
    if (it == moMySQLPoolList.end())
    {
        LOG_TRACE(LOG_DEBUG_1,false, __FUNCTION__, " DBindex ERROR: index="<< aiIndex ) ;
        return NULL ;
    }
    CMySQLPool *lpMySQLPool = it->second;
    lpMySQLPool->IncreaseRef();
    return lpMySQLPool;
}
void CMySQLDriverPool::putMYSQLPool(CMySQLPool * apObj)
{
     apObj->DecreaseRef();
     if(!apObj->CanRelease())
     {
         //不能被释放
         return ;
     }
     apObj->Close();
     delete apObj;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： GetItem
// 函数参数： auid 需要将相同的ID的事务放置到同一个线程中处理
// 返 回 值： 连接指针
// 函数说明： 获取ODCB空闲连接对象
// $_FUNCTION_END ********************************
IConnItem* CMySQLDriverPool::MallocItem(const uint64& aiID, const uint32 aiDBindex)
{
    AutoReleaseMySQLPool lpMySQLPool(this,aiDBindex);
    if(!lpMySQLPool.IsValid())
    {
        LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error FreeItem not found the index db" 
            <<" ID:"<<aiID
            <<" Index:"<< aiDBindex
            );
        return 0;
    }
    
    IConnItem *lpItem = lpMySQLPool->MallocItem();
     if ((NULL == lpItem )|| (false == lpItem->IsValid()))
    {
        LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error FreeItem not found the index db" 
            <<" ID:"<<aiID
            <<" Index:"<< aiDBindex
        );
        return NULL ;
    }
     LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, " " 
         <<" ID:"<<aiID
         <<" Index:"<< aiDBindex
         <<" ConnItemPtr:"<<lpItem);
    return lpItem;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： FreeItem
// 函数参数： apItem	连接指针
// 返 回 值： 0代表成功，小于0代表失败
// 函数说明： 释放ODBC连接
// $_FUNCTION_END ********************************
int CMySQLDriverPool::FreeItem(const uint64 & aiID, const uint32 aiDBindex, IConnItem* apItem)
{
    AutoReleaseMySQLPool lpMySQLPool(this,aiDBindex);
    if(!lpMySQLPool.IsValid())
    {
        LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error FreeItem not found the index db" 
            <<" ID:"<<aiID
            <<" Index:"<<aiDBindex
            <<" ConnItemPtr:"<<apItem
            );
        return 0;
    }
    lpMySQLPool->FreeItem(apItem);
    LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, " " 
        <<" ID:"<<aiID
        <<" Index:"<< aiDBindex
        <<" ConnItemPtr:"<<apItem);
    return 0;
}

