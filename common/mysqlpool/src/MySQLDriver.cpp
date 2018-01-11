// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
// 
// 文件名称: ySQLDriver.cpp
// 创 建 人: 史云杰
// 文件说明:  MySQL C　API封装实现文件
// $_FILEHEADER_END *****************************
#include "common/System.h"
#include "mysqldriver/MySQLDriver.h"


#define MYSQL_RING_STAMP    (60)

//
//void * _DBLinkThread(void* param)
//{
//    CMySQLItem* link = (CMySQLItem*)param;
//    link->checkDBConnection();
//    pthread_detach(pthread_self());
//
//    return NULL;
//}

CMySQLItem::CMySQLItem()
{
	mpMySQL = NULL;
	mpStmt = NULL;
	memset(mszLastErrMsg, 0, MAXERRMSGLENGTH);
    mbConnectting = false ;
    mbIsRun = false ;
    muKeepLiveTimeStamp = time(NULL) ;
}

CMySQLItem::~CMySQLItem()
{
    mbIsRun = false ;
	Close();
}

bool CMySQLItem::IsValid()
{
    if(NULL  == GetMySQL())
    {
        return false ;
    }
    MYSQL_STMT*	lpStmt = StmtInit();
    if (NULL == lpStmt)
    {
        return false ;
    }
    return true ;
}

//
//void CMySQLItem::checkDBConnection() 
//{
//    while (mbIsRun)
//    {
//        uint32 uNow = time(NULL) ;
//        if (uNow - muKeepLiveTimeStamp < MYSQL_RING_STAMP)
//        {
//            usleep(5000) ;
//            continue ;
//        }
//        if (false == mbIsRun)
//        {
//            break ;
//        }
//        muKeepLiveTimeStamp = uNow ;
//        bool lbIsConnctting = true ;
//        if (false == mbConnectting)
//        {
//            lbIsConnctting = ReConnect() ;
//        }
//        if (lbIsConnctting)
//        {
//            Ping() ;
//        }
//        usleep(500) ;
//    }
//}

bool CMySQLItem::ReConnect()
{
    if (NULL != mpMySQL )
    {
        Close() ;
    }
    return Open(modbaddr.c_str(), muPort, modbname.c_str(), modbuser.c_str(), modbPassword.c_str()) ;
}

bool CMySQLItem::Open(const char* apszServerAddr, uint32 auPort,
                         const char* apszDBname, const char* apszUser, const char* apszPassword)
{
	mpMySQL = mysql_init(mpMySQL);
	if(NULL == mpMySQL)
    {
        strcpy(mszLastErrMsg, "Failed to init MYSQL.");
        LOG_TRACE(LOG_ERR, false,"CMySQLItem::Open","Failed to init MYSQL.");
        return false;
    }
    LOG_TRACE(LOG_DEBUG_1, true, "CMySQLItem::Open","Init MYSQL succeed.");
    mbIsRun = true ;

    //pthread_t thread;
    //unsigned int ithread = pthread_create(&thread, NULL, _DBLinkThread, this);
    //if (ithread != 0)
    //{
    //    LOG_TRACE(4, false, "CMySQLItem::Open", "_DBLinkThread Error. IP:" 
    //        );
    //    return false;
    //}
    modbaddr = apszServerAddr ;
    muPort = auPort ;
    modbname = apszDBname ;
    modbuser = apszUser ;
    modbPassword = apszPassword ;

    if(false == Connect(apszServerAddr, apszUser, apszPassword, apszDBname, auPort))
    {
        LOG_TRACE(LOG_ERR, false, "CMySQLItem::Open", "Connect Error. IP:" 
            <<apszServerAddr <<":" <<auPort
            <<", UserName: " <<apszUser);
        return false;
    }
	return true;
}

bool CMySQLItem::Connect(const char *aszHost, const char *aszUser, const char *aszPasswd,
			 const char *aszDb, uint32 auiPort)
{
    if((NULL == aszHost) || (NULL == aszDb)|| (NULL == aszUser))
    {
        LOG_TRACE(LOG_ERR,false, "CMySQLItem::Connect", "Connect "<<aszHost<<" " <<aszUser
            <<" " <<aszDb <<" " <<auiPort <<" failed");
        return false ;
    }

	//为了能调用存储过程，启用CLIENT_MULTI_RESULTS参数
	MYSQL* lpMySQL = mysql_real_connect(mpMySQL, aszHost, aszUser, aszPasswd, 
		aszDb, auiPort, NULL, CLIENT_MULTI_STATEMENTS|CLIENT_MULTI_RESULTS);
	if(NULL == lpMySQL)
	{
		strcpy(mszLastErrMsg, "Failed to connect to database: Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(LOG_ERR, false, "CMySQLItem::Connect", mszLastErrMsg);
		return false;
	}
    mbConnectting = true ;
    LOG_TRACE(LOG_DEBUG, true, "CMySQLItem::Connect","Connect: "<<aszHost<<" " <<aszUser
        <<" " <<auiPort <<" succeed");
	return true;
}

// 关闭连接
int32_t CMySQLItem::Close()									
{
    int32_t liRet = 0;
    //if ((NULL == mpStmt) || (NULL != mpMySQL))
    //{
    //    return -1 ;
    //}
    mbConnectting = false ;
    if( mpStmt != NULL && 0 != (liRet = StmtClose()))
    {
        strcpy(mszLastErrMsg, "Failed to call mysql_stmt_close: Error:");
        strcat(mszLastErrMsg, mysql_error(mpMySQL));
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, mszLastErrMsg);
        //return liRet;
    }
    mpStmt = NULL;
    if(mpMySQL != NULL)
        mysql_close(mpMySQL);
    mpMySQL = NULL;
    memset(mszLastErrMsg, 0, MAXERRMSGLENGTH);
    LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "MYSQL Closed succeed.");
    return liRet;
}

// 回滚
int CMySQLItem::RollBack()								
{
	assert(mpMySQL);
	int liRet = mysql_rollback(mpMySQL); 

	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to RollBack: Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, mszLastErrMsg);
	}
	else
	{
		LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "MYSQL RollBack.");
	}
	return liRet;
}

IRecordSet* CMySQLItem::Query(const char* aszQuery, uint32 auiLength)
{
	int liRet = mysql_real_query(mpMySQL, aszQuery, auiLength);

	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to ExecuteDirect: ");
		strcat(mszLastErrMsg, aszQuery);
		strcat(mszLastErrMsg, " Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, mszLastErrMsg);
        return NULL ;
	}
    IRecordSet* pRecSet = new CMySQLRecordSet(mysql_store_result(mpMySQL)) ;
    return pRecSet; 
}

int CMySQLItem::SelectDB(const char* aszDBName)
{
	int liRet = mysql_select_db(mpMySQL, aszDBName);

	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to SelectDB: ");
		strcat(mszLastErrMsg, aszDBName);
		strcat(mszLastErrMsg, " Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, mszLastErrMsg);
	}
	else
	{
		LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "SelectDB succeed: "<<aszDBName);
	}
	return liRet;
}

// 创建MYSQL_STMT句柄
MYSQL_STMT* CMySQLItem::StmtInit()
{
	mpStmt = mysql_stmt_init(mpMySQL);
	return mpStmt;
}

// 准备绑定参数的语句
bool CMySQLItem::StmtPrepare(const char *aszQuery, uint32 auiLength)
{
    if(NULL == mpStmt)
	{
		strcpy(mszLastErrMsg, "Have not init stmt.");
		LOG_TRACE(LOG_ERR,false,__FUNCTION__,mszLastErrMsg<<"\n");
		return true;
	}

	if(0 != mysql_stmt_prepare(mpStmt, aszQuery, auiLength))
	{
		strcpy(mszLastErrMsg, "Failed to StmtPrepare: Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(LOG_ERR, false, __FUNCTION__,mszLastErrMsg);
        return false ;
    }
    LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "StmtPrepare succeed: "<<aszQuery);
    return true;
}

// 绑定参数,此参数以MYSQL_BIND类型的数组形式传入,数组的个数可以由StmtPrepare中?的数量确定
bool CMySQLItem::StmtBindParam(MYSQL_BIND *apBind)
{
	if(0 != mysql_stmt_bind_param(mpStmt, apBind))
	{
		strcpy(mszLastErrMsg, "Failed to BindParam: Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, mszLastErrMsg);
        return false ;
	}
	return true;
}

bool CMySQLItem::StmtExecute()					// 执行绑定参数的SQL语句
{
	if(0 != mysql_stmt_execute(mpStmt))
	{
		strcpy(mszLastErrMsg, "Failed to StmtExecute: Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, mszLastErrMsg);
        return false ;
	}
	return true ;
}

const char* CMySQLItem::GetLastErrMsg()
{
    return mszLastErrMsg;
}

uint32 CMySQLItem::getLastError()
{
    return (uint32)mysql_errno(mpMySQL);
}

bool CMySQLItem::Execute(const char* aszScript, uint32 auiLength)
{
    if (false == StmtPrepare(aszScript, auiLength))
    {
        return false ;
    }
    return StmtExecute();
}

int CMySQLItem::GetAffectedRows()
{
	int liAffectedRows = mysql_stmt_affected_rows(mpStmt);
	if (liAffectedRows < 0)
	{
		strcpy(mszLastErrMsg, "GetAffectedRows Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(LOG_ERR,false,__FUNCTION__,mszLastErrMsg
			<<" AffectedRows="<<liAffectedRows);
	}
	return liAffectedRows;
};

// 关闭预处理语句
int CMySQLItem::StmtClose()
{
	int liRet = mysql_stmt_close(mpStmt);
	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to StmtClose: Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, mszLastErrMsg);
	}
	else
	{
		LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__,"StmtClose succeed.");
		mpStmt = NULL;
	}
	return liRet;

}

// 返回当前默认字符集名字
const char* CMySQLItem::GetCharacterSetName()
{
	return mysql_character_set_name(mpMySQL); 
}

uint32 CMySQLItem::escapeString(const char* apszSrc, uint32 auSrclen, char* apszDst)
{
    if ((NULL == apszSrc) || (NULL == apszDst) || (0 == auSrclen))
    {
        return 0 ;
    }
    return mysql_real_escape_string(mpMySQL, apszDst, apszSrc, (unsigned long)auSrclen);
}

// 设置当前默认字符集名字
int CMySQLItem::SetCharacterSet(const char *aszCsName)
{
	int liRet = mysql_set_character_set(mpMySQL, aszCsName);
	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to SetCharacterSet: Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(LOG_ERR, false, __FUNCTION__,mszLastErrMsg);
	}
	else
	{
		//LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__,"SetCharacterSet succeed. param:" <<aszCsName);			
	}
	return liRet;
}

bool CMySQLItem::IsDBExist(const char* aszDBName)
{
	MYSQL_RES *lpRes = mysql_list_dbs(mpMySQL, aszDBName);
	if(NULL == lpRes)
    {
        strcpy(mszLastErrMsg, "Failed to IsDBExist: Error:");
        strcat(mszLastErrMsg, mysql_error(mpMySQL));
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, mszLastErrMsg);
        return false ;
    }
    if(lpRes->row_count == 0)
    {
        mysql_free_result(lpRes);
        LOG_TRACE(LOG_ERR, false, __FUNCTION__,aszDBName<<"Database Is not Exist.");
        return false;
    }
    mysql_free_result(lpRes);
    LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__,aszDBName<<"Database Is Exist.");
    return true;
}

int CMySQLItem::Ping()
{
	if(mysql_ping(mpMySQL))
	{
		return -1;
	}

	return 1;
}

//读取剩下的结果集丢掉，主要是考虑到调用存储过程会返回多个结果集
void CMySQLItem::CleanAllNextResult()
{
	for(;!mysql_next_result( mpMySQL );)
	{

		MYSQL_RES * lpResult = mysql_store_result(mpMySQL);
		if(!lpResult)
			continue;
		mysql_free_result(lpResult); 
	}
}

auto_ptr<CMySQLRecordSet> CMySQLItem::StoreResult()
{
	auto_ptr<CMySQLRecordSet> loPtr(new CMySQLRecordSet(mysql_store_result(mpMySQL)));
	return loPtr;
}

/////////////////////////////////////////////////////////////////////////////
//CODBCPool 构造
CMySQLPool::CMySQLPool():
mbActive(false)
{
}

/////////////////////////////////////////////////////////////////////////////
//CODBCPool 析构
CMySQLPool::~CMySQLPool()
{
    mbActive = false;
}


void CMySQLPool::init()
{
    return ;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： Init
// 函数参数： NULL
// 返 回 值： 1代表成功，小于1代表失败
// 函数说明： 打开数据库
// $_FUNCTION_END ********************************
uint32 CMySQLPool::Open(const char* apszServerAddr, uint32 auPort,
                     const char* apszDBname, const char* apszUser, const char* apszPassword, 
                     const uint32 auDBUsertype, const uint32 auIndex, const uint32 auPoolSize)
{
	LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "begin to open.");
	//获取并发连接数
	moMySQLList.clear();
	moMySQLList.init(auPoolSize);

	//建立连接列表
	//创建连接和连接句柄
	CMySQLItem* lpItem = NULL;
    uint32 luPoolSize = 0 ;
	uint32 luIndex = 0;
	for (luIndex = 0; luIndex < auPoolSize; luIndex ++)
	{
		lpItem = new CMySQLItem();
		if(NULL == lpItem)
        {
			continue;
        }
		if (false == lpItem->Open(apszServerAddr, auPort, apszDBname, apszUser, apszPassword))
        {
			return 0;
        }
        luPoolSize++ ;
		lpItem->SetCharacterSet("UTF8");
		FreeItem(lpItem);
	}
    m_uPoolSize = luPoolSize ;
    m_uDBtype = auDBUsertype ;
    m_uIndex = auIndex ;
	mbActive = true;
	return luPoolSize;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： ClosePool
// 函数参数： NULL
// 返 回 值： NULL
// 函数说明： 关闭数据池
// $_FUNCTION_END ********************************
void CMySQLPool::Close()
{
	LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "begin to close MySQLPool.");
	mbActive = false;
	CMySQLItem* lpoItem = NULL;
	while(NULL != (lpoItem = (CMySQLItem*)moMySQLList.GetandDelHead()))
	{
        lpoItem->Close();
        delete lpoItem;
	}
	moMySQLList.clear();
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： GetItem
// 函数参数： NULL
// 返 回 值： 连接指针
// 函数说明： 获取ODCB空闲连接对象
// $_FUNCTION_END ********************************
IConnItem* CMySQLPool::MallocItem()
{
	LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "MallocItem in MySQLPool.");
	IConnItem* lpoItem = NULL;
	while(NULL == (lpoItem = moMySQLList.GetandDelHead()))
	{
		CSystem::Sleep(10);
		if(false == mbActive)
        {
			return NULL;//退出
        }
	}
	return lpoItem;
}


