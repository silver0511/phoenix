// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
// 
// 文件名称: MySQLDriverList.h
// 创 建 人: 史云杰
// 文件说明:  MySQL C　API封装连接池头文件
// $_FILEHEADER_END *****************************
#ifndef __MYSQLDRIVERPOOL_H_
#define __MYSQLDRIVERPOOL_H_

#include <vector>
#include "common/Debug_log.h"
#include "MySQLDriver.h"
#include "DBI.h"

/////////////////////////////////////////////////////////////////////////////
// CMySQLPool 类
class CMySQLDriverPool:public U9::IDriverPool
{
public:
    typedef std::map<uint32, PMySQLPool> CMySQLPoolList ;
    typedef std::map<uint32, PMySQLPool>::iterator CMySQLPoolListIter ;
public:
	CMySQLDriverPool();
	virtual ~CMySQLDriverPool();

    void init() ;
    void Release() ;
 
	uint32 Open(const CDBInfoList& aDBList);	
    uint32 Open(const CDBinfo& aDBInfo);
	void Close();	
    void Close(uint32 auDBindex) ;

	IConnItem*	MallocItem(const uint64& aiID, const uint32 aindex);	          // 获取MySQL空闲连接对象
	int FreeItem(const uint64& aiID, const uint32 auDBtype, IConnItem* apItem);	  // 释放MySQL连接
	
	uint32 size(){return moMySQLPoolList.size();}
    void KeepLive() ;
public:
    void checkDBConnection() ;

private:
    CMySQLDriverPool(const CMySQLDriverPool& pobj) ;
private:
    CMySQLPool *getMySQLPool(const uint32 aiIndex);
    void putMYSQLPool(CMySQLPool * apObj);
    class AutoReleaseMySQLPool
    {
    public:
        AutoReleaseMySQLPool()
        {
            mpMySQLPoolList = NULL ;
        }

        AutoReleaseMySQLPool(CMySQLDriverPool* lpMySQLPoolList,const uint32 aiIndex) 
        {
            mpMySQLPoolList = lpMySQLPoolList ;
            mpMySqlPool = lpMySQLPoolList->getMySQLPool(aiIndex);
        }

        AutoReleaseMySQLPool(CMySQLPool* apMySqlPool) 
        {
            mpMySqlPool = apMySqlPool;
            mpMySQLPoolList = NULL ;
        }

        ~AutoReleaseMySQLPool() 
        {
            if((NULL == mpMySqlPool) 
                || (NULL == mpMySQLPoolList))
            {
                return ;
            }
            mpMySQLPoolList->putMYSQLPool(mpMySqlPool);
        }
        inline CMySQLPool* operator ->(){return mpMySqlPool;} 
        bool IsValid() {return (bool)(mpMySqlPool != NULL) ;}
        inline CMySQLPool* GetPtr(){return mpMySqlPool;}
    private:
        CMySQLPool*	     mpMySqlPool;
        CMySQLDriverPool*  mpMySQLPoolList ; 
    };
private:
	U9::CCriticalSection         moCriticalSection;
	CMySQLPoolList                     moMySQLPoolList ;                    // 空闲连接列表

    bool mbIsRun ;
    uint8_t muKeepLiveTimeStamp ;
};



#endif	//__MYSQLDRIVERPOOL_H_
