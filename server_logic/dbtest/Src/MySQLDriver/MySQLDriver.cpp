#include "stdafx.h"
#include "MySQLDriver.h"
#include "common/System.h"
CMySQLItem::CMySQLItem()
{
	mpMySQL = NULL;
	mpStmt = NULL;
	memset(mszLastErrMsg, 0, MAXERRMSGLENGTH);
}

CMySQLItem::~CMySQLItem()
{
	Close();
}

INT CMySQLItem::Open(const STRU_MYSQL_CONNECT_INFO &aoConnectInfo)
{
	mpMySQL = mysql_init(mpMySQL);
	if(NULL == mpMySQL)
	{
		strcpy(mszLastErrMsg, "Failed to init MYSQL.");
		LOG_TRACE(2,false,__FUNCTION__,"Failed to init MYSQL.\n");
		return -1;
	}
	else
	{
		LOG_TRACE(5,false,__FUNCTION__,"Init MYSQL succeed.\n");
	}
	
	if(1 != Connect(aoConnectInfo.mstrIP.c_str(), aoConnectInfo.mstrUserName.c_str(),
		aoConnectInfo.mstrUserPwd.c_str(),	aoConnectInfo.mstrDBName.c_str(), aoConnectInfo.mwPort))
	{
		LOG_TRACE(3, true, __FUNCTION__, "Connect Error. IP:" 
			<<GetIPAddress(aoConnectInfo.mstrIP.c_str()) <<":" <<ntohs(aoConnectInfo.mwPort)
			<<", UserName: " <<aoConnectInfo.mstrUserName <<"\n");
		return -1;
	}
	


	return 1;
}

INT CMySQLItem::Connect(const char *aszHost, const char *aszUser, const char *aszPasswd,
			 const char *aszDBName, unsigned int auiPort)
{	
	my_bool reconnect = true;
	mysql_options(mpMySQL, MYSQL_OPT_RECONNECT, &reconnect);
	MYSQL* lpMySQL = mysql_real_connect(mpMySQL, aszHost, aszUser, aszPasswd, 
		aszDBName, auiPort, NULL, CLIENT_MULTI_STATEMENTS|CLIENT_MULTI_RESULTS);
	//U9_ASSERT(lpMySQL != NULL && lpMySQL == mpMySQL);
	if(NULL == lpMySQL || lpMySQL != mpMySQL)
	{
		LOG_TRACE(3,false,__FUNCTION__,"Error mysql_real_connect fail "<<aszHost<<" " <<aszUser
				<<" " <<aszDBName <<" " <<auiPort <<" succeed");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
		return -1;
	}
	else
	{
		if(aszDBName != NULL)
		{
			////???????¨μ?êy?Y?aê§°ü
			//if ( mysql_select_db(mpMySQL, aszDBName ) < 0 ) 
			//{
			//	LOG_TRACE(3,false,__FUNCTION__,"Error mysql_select_db fail "<<aszHost<<" " <<aszUser
			//	<<" " <<aszDBName <<" " <<auiPort <<" succeed");
			//	mysql_close(mpMySQL) ;
			//	return -1 ;
			//}
			LOG_TRACE(3,false,__FUNCTION__,"Connect "<<aszHost<<" " <<aszUser
				<<" " <<aszDBName <<" " <<auiPort <<" succeed"<<"\n");
		}
		else
		{
			LOG_TRACE(3,false,__FUNCTION__,"Connect "<<aszHost<<" " <<aszUser
										<<" " <<aszDBName <<" " <<auiPort <<" succeed");
		}
	}
	return 1;
}


INT CMySQLItem::Close()									// 1?±?á??ó
{
	INT liRet = 0;
	if(mpStmt != NULL)
	{
		liRet = StmtClose();
		if(liRet != 0)
		{
			strcpy(mszLastErrMsg, "Failed to call mysql_stmt_close: Error:");
			strcat(mszLastErrMsg, mysql_error(mpMySQL));
			LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
			return liRet;
		}
	}
	if(mpMySQL != NULL)
	{
		mysql_close(mpMySQL);
	}
	memset(mszLastErrMsg, 0, MAXERRMSGLENGTH);

	//mysql_library_end();

	LOG_TRACE(3,false,__FUNCTION__,"MYSQL Closed succeed."<<"\n");
	return liRet;
}


INT CMySQLItem::RollBack()								// ??1?
{
	U9_ASSERT(mpMySQL);
	mysql_ping(mpMySQL);
	INT liRet = mysql_rollback(mpMySQL); 

	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to RollBack: Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
	}
	else
	{
		LOG_TRACE(5,false,__FUNCTION__,"MYSQL RollBack.\n");
	}
	return liRet;
}

INT CMySQLItem::ExecuteDirect(const char* aszQuery, unsigned long auiLength)
{
	U9_ASSERT(mpMySQL != NULL);
	U9_ASSERT(aszQuery != NULL);

	mysql_ping(mpMySQL);

	int liRet = mysql_real_query(mpMySQL, aszQuery, auiLength);

	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to ExecuteDirect: ");
		strcat(mszLastErrMsg, aszQuery);
		strcat(mszLastErrMsg, " Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
	}
	else
	{
		LOG_TRACE(5,false,__FUNCTION__,"ExecuteDirect succeed: "<<aszQuery <<"\n");
	}
	return liRet;
}

INT CMySQLItem::SelectDB(const char* aszDBName)
{
	U9_ASSERT(mpMySQL != NULL);
	U9_ASSERT(aszDBName != NULL);

	mysql_ping(mpMySQL);

	int liRet = mysql_select_db(mpMySQL, aszDBName);

	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to SelectDB: ");
		strcat(mszLastErrMsg, aszDBName);
		strcat(mszLastErrMsg, " Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
	}
	else
	{
		LOG_TRACE(5,false,__FUNCTION__,"SelectDB succeed: "<<aszDBName <<"\n");
	}
	return liRet;
}

// ′′?¨MYSQL_STMT??±ú
MYSQL_STMT* CMySQLItem::StmtInit()
{
	U9_ASSERT(mpMySQL != NULL);
	mpStmt = mysql_stmt_init(mpMySQL);
	return mpStmt;
}

// ×?±?°ó?¨2?êyμ?ó???
INT CMySQLItem::StmtPrepare(const char *aszQuery, unsigned long auiLength)
{
	U9_ASSERT(mpMySQL != NULL);
	U9_ASSERT(aszQuery != NULL);

	if(NULL == mpStmt)
	{
		strcpy(mszLastErrMsg, "Have not init stmt.");
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
		return 1;
	}
	//LOG_TRACE(5,false,__FUNCTION__,"StmtInit succeed.\n");
	int liRet = mysql_stmt_prepare(mpStmt, aszQuery, auiLength);

	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to StmtPrepare: Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
	}
	else
	{
		//LOG_TRACE(5,false,__FUNCTION__,"StmtPrepare succeed: "<<aszQuery <<"\n");
	}
	return liRet;
}

// °ó?¨2?êy,′?2?êyò?MYSQL_BINDààDíμ?êy×éD?ê?′?è?,êy×éμ???êy?éò?óéStmtPrepare?D?μ?êyá?è·?¨
INT CMySQLItem::StmtBindParam(MYSQL_BIND *apBind)
{
	U9_ASSERT(mpMySQL != NULL);
	U9_ASSERT(mpStmt != NULL);
	U9_ASSERT(apBind != NULL);

	INT liRet = mysql_stmt_bind_param(mpStmt, apBind);
	
	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to BindParam: Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
	}
	else
	{
		//LOG_TRACE(5,false,__FUNCTION__,"StmtBindParam succeed: "<<"\n");
	}
	return liRet;
}

INT CMySQLItem::StmtExecute()					// ?′DD°ó?¨2?êyμ?SQLó???
{
	U9_ASSERT(mpMySQL != NULL);
	U9_ASSERT(mpStmt != NULL);
	mysql_ping(mpMySQL);
	INT liRet = mysql_stmt_execute(mpStmt);
	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to StmtExecute: Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
	}
	else
	{
		LOG_TRACE(5,false,__FUNCTION__,"StmtExecute succeed. \n");
	}
	return liRet;
}
INT CMySQLItem::GetAffectedRows()
{
	U9_ASSERT(mpMySQL != NULL);
	U9_ASSERT(mpStmt != NULL);
	INT liAffectedRows = mysql_stmt_affected_rows(mpStmt);
	if (liAffectedRows < 0)
	{
		strcpy(mszLastErrMsg, "GetAffectedRows Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg
			<<" AffectedRows="<<liAffectedRows
			<<"\n");
	}
	return liAffectedRows;
};

// 1?±??¤′|àíó???
INT CMySQLItem::StmtClose()
{
	U9_ASSERT(mpStmt != NULL);
	INT liRet = mysql_stmt_close(mpStmt);
	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to StmtClose: Error:");
		strcat(mszLastErrMsg, mysql_stmt_error(mpStmt));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
	}
	else
	{
		//LOG_TRACE(5,false,__FUNCTION__,"StmtClose succeed.\n");
		mpStmt = NULL;
	}
	return liRet;

}

// ·μ??μ±?°??è?×?·??ˉ??×?
const char* CMySQLItem::GetCharacterSetName()
{
	U9_ASSERT(mpMySQL != NULL);
	return mysql_character_set_name(mpMySQL); 
}

// éè??μ±?°??è?×?·??ˉ??×?
INT CMySQLItem::SetCharacterSet(const char *aszCsName)
{
	U9_ASSERT(mpMySQL != NULL);
	INT liRet = mysql_set_character_set(mpMySQL, aszCsName);
	if(liRet != 0)
	{
		strcpy(mszLastErrMsg, "Failed to SetCharacterSet: Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
	}
	else
	{
		LOG_TRACE(5,false,__FUNCTION__,"SetCharacterSet succeed. param:" <<aszCsName<<"\n");			
	}
	return liRet;
}

bool CMySQLItem::IsDBExist(const char* aszDBName)
{
	U9_ASSERT(mpMySQL != NULL);
	mysql_ping(mpMySQL);
	MYSQL_RES *lpRes = mysql_list_dbs(mpMySQL, aszDBName);
	if(NULL == lpRes)
	{
		strcpy(mszLastErrMsg, "Failed to IsDBExist: Error:");
		strcat(mszLastErrMsg, mysql_error(mpMySQL));
		LOG_TRACE(2,false,__FUNCTION__,mszLastErrMsg<<"\n");
	}
	else
	{
		if(lpRes->row_count == 0)
		{
			mysql_free_result(lpRes);
			LOG_TRACE(2,false,__FUNCTION__,aszDBName<<"Database Is not Exist.\n");
			return false;
		}
		else
		{
			mysql_free_result(lpRes);
			LOG_TRACE(4,false,__FUNCTION__,aszDBName<<"Database Is Exist.\n");
			return true;
		}
	}
	return false;
}

INT CMySQLItem::Ping()
{
	U9_ASSERT(mpMySQL != NULL);

	if(mysql_ping(mpMySQL))
	{
		return -1;
	}

	return 1;
}

auto_ptr<CMySQLRes> CMySQLItem::StoreResult()
{
	auto_ptr<CMySQLRes> loPtr(new CMySQLRes(mysql_store_result(mpMySQL)));
	return loPtr;
}

//?áè?ê￡??μ??á1??ˉ?aμ?￡??÷òaê?????μ?μ÷ó?′?′￠1y3ì?á·μ???à???á1??ˉ
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

// $_FUNCTION_BEGIN ******************************
// oˉêy??3?￡o Init
// oˉêy2?êy￡o NULL
// ·μ ?? ?μ￡o 1′ú±í3é1|￡?D?óú1′ú±íê§°ü
// oˉêy?μ?÷￡o ′ò?aêy?Y?a
// $_FUNCTION_END ********************************
int CMySQLPool::Open(const STRU_MYSQL_CONNECT_INFO &aoConnInfo)
{
	LOG_TRACE(3,false,__FUNCTION__,"begin to open.\n");
	//??è?2￠·￠á??óêy
	moMySQLList.clear();
	moMySQLList.init(aoConnInfo.miConnectCount);

	//?¨á￠á??óáD±í
	//′′?¨á??óoíá??ó??±ú
	CMySQLItem * lpItem = NULL;
	INT liResult = 0;
	INT i = 0;
	for (i = 0; i < aoConnInfo.miConnectCount; i ++)
	{
		lpItem = new CMySQLItem();
		if(NULL == lpItem)
			continue;

		liResult = lpItem->Open(aoConnInfo);
		if (liResult <= 0)
			return liResult;
		lpItem->SetCharacterSet("UTF8");
		FreeItem(lpItem);
	}
	miDBIndex = aoConnInfo.miDBIndex;
	mbActive=TRUE;
	
	return 1;
}

// $_FUNCTION_BEGIN ******************************
// oˉêy??3?￡o ClosePool
// oˉêy2?êy￡o NULL
// ·μ ?? ?μ￡o NULL
// oˉêy?μ?÷￡o 1?±?êy?Y3?
// $_FUNCTION_END ********************************
void CMySQLPool::Close()
{
	LOG_TRACE(3,false,__FUNCTION__,"begin to close MySQLPool.\n");
	mbActive=FALSE;
	moMySQLList.clear();
}

// $_FUNCTION_BEGIN ******************************
// oˉêy??3?￡o GetItem
// oˉêy2?êy￡o NULL
// ·μ ?? ?μ￡o á??ó????
// oˉêy?μ?÷￡o ??è?ODCB???Dá??ó???ó
// $_FUNCTION_END ********************************
CMySQLItem* CMySQLPool::MallocItem()
{
	//LOG_TRACE(5,false,__FUNCTION__,"MallocItem in MySQLPool.\n");
	CMySQLItem* lpoItem=NULL;
	while(NULL == (lpoItem =moMySQLList.GetandDelHead()))
	{
		CSystem::Sleep(10);
		if(FALSE == mbActive)
			return NULL;//í?3?
	}
	return lpoItem;
}



/////////////////////////////////////////////////////////////////////////////
//CODBCPool 11?ì
CMySQLPool::CMySQLPool():
mbActive(FALSE),
	miDBIndex(0)
{
}

/////////////////////////////////////////////////////////////////////////////
//CODBCPool ??11
CMySQLPool::~CMySQLPool()
{
	mbActive=FALSE;
}

