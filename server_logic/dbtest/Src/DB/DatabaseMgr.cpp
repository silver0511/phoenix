
#include "stdafx.h"
#include "const/ErrorCodeDef.h"
#include "DBTableName.h"
#include "DatabaseMgr.h"
#include "../Config.h"
#include "MySQLDriver/MySQLBind.h"

#ifndef WIN32 

#include <dlfcn.h>
#endif //WIN32

UINT64 STR2UINT64(const std::string& szStr)
{
	UINT64 dst = 0;
	std::stringstream ss;
	ss << szStr;
	ss >> dst;

	return dst;
}


CDatabaseMgr::CDatabaseMgr()
{


}
CDatabaseMgr::~CDatabaseMgr()
{

}

std::string CDatabaseMgr::GetTableIndex(USERID aiUserID)
{
	uint32 tableindex = aiUserID % 32;
	std::string szTableIndex = std::to_string(tableindex);
	if (1 == szTableIndex.size())
	{
		szTableIndex = "0" + szTableIndex;
	}

	return szTableIndex;	
}

uint32 CDatabaseMgr::GetContactDBPoolIndex(USERID aiUserID)
{
	uint32 uidbpoolindex = aiUserID % 4;
	return uidbpoolindex;
}

INT CDatabaseMgr::open(CConfig &aoConfig)
{
	if (1 != moContactDBPool.open(aoConfig.mpContactDBList,aoConfig.miContactDBCount))
	{
		LOG_TRACE(2,false,__FUNCTION__," initialize  contact db poll error.");
		return -1;
	}

	return 1;

}

////////////////////////////////////////////////////////////////////////////////////////
void CDatabaseMgr::close()
{
	moContactDBPool.close();
}

int32 CDatabaseMgr::GetFriendList(const std::string& szuserid, std::vector<STRFRIENDINFO>& friendlist)
{
	UINT64 aiuserid = STR2UINT64(szuserid);	
	AutoReleaseMySQLItemExt loMySQL(moContactDBPool,GetContactDBPoolIndex(aiuserid));

	if (loMySQL.Invalid())
	{
		LOG_TRACE(3,0,__FUNCTION__," AutoReleaseMySQLItemExt CreateError.\n");
		return -1;
	}

	
	std::string sztableindex = GetTableIndex(aiuserid);

	std::string lszSQL = "select friendid,sourcetype,remarkname from nim_friend_" 
                          + sztableindex 
                          + " where userid=" 
                          + szuserid;

	
	LOG_TRACE(3,0,__FUNCTION__,"DB:im_db" <<  GetContactDBPoolIndex(aiuserid) << " Exect sql:" << lszSQL);

	try
	{
		if(loMySQL->ExecuteDirect(lszSQL.c_str(), lszSQL.size()))
		{
			LOG_TRACE(3,0,__FUNCTION__,"Error ExecuteDirect fail"
									  << " error=" << loMySQL->GetLastErrMsg()
									  << " SQL="<<lszSQL);
			return -1;
		}

		auto_ptr<CMySQLRes> loMySQLRes = loMySQL->StoreResult();
		if(loMySQLRes.get() == NULL)
		{
			LOG_TRACE(3,0,__FUNCTION__,"Get MYSQL Result Error:"
									  << " error=" <<loMySQL->GetLastErrMsg()
				                      << " SQL="<<lszSQL);
			return -1;
		}

		uint64 liResultRow = loMySQLRes->GetNumOfRow();

		MYSQL_ROW loRow;
		int32 liLastUpdateTime  = 0;
		for(int i = 0; i < liResultRow && (loRow = loMySQLRes->FetchRow()); i++)
		{
			STRFRIENDINFO friendinfo;
			if (NULL != loRow[0])
			{
				friendinfo.szuserid = loRow[0];
			}

            if (NULL != loRow[1])
			{
				friendinfo.szsourcetype = loRow[1];
			}
	
			if (NULL != loRow[2])
			{
				friendinfo.szremarkname = loRow[2];
			}			
		
			friendlist.push_back(friendinfo);
		}
	}
	catch (...)
	{
		LOG_TRACE(3,0,__FUNCTION__,"Error Exception Error:"
			                      << " error=" << loMySQL->GetLastErrMsg()
			                      << " SQL=" << lszSQL);
		return -1;	
	}

	return 0;	
}

int32 CDatabaseMgr::GetFriendRemark(const std::string& szuserid, const std::string& szpeeruserid, const std::string& szremarkname)
{
	UINT64 aiuserid = STR2UINT64(szuserid);	
	AutoReleaseMySQLItemExt loMySQL(moContactDBPool,GetContactDBPoolIndex(aiuserid));

	if (loMySQL.Invalid())
	{
		LOG_TRACE(3,0,__FUNCTION__," AutoReleaseMySQLItemExt CreateError.\n");
		return -1;
	}

	
	std::string sztableindex = GetTableIndex(aiuserid);

	std::string lszSQL = "update nim_friend_" + sztableindex
                          + " set remarkname=\"" + szremarkname + "\""
						  + " where friendid=" + szpeeruserid
						  + " and userid=" + szuserid;
	
	LOG_TRACE(3,0,__FUNCTION__,"DB:im_db" <<  GetContactDBPoolIndex(aiuserid) << " Exect sql:" << lszSQL);

	try
	{
		if(loMySQL->StmtPrepare(lszSQL.c_str(), lszSQL.size()))
		{
			LOG_TRACE(4,false,__FUNCTION__," Error StmtPrepare fail"
										  << " error="<<loMySQL->GetLastErrMsg()
										  << " SQL="<<lszSQL);
			return -1;
		}

		if(loMySQL->ExecuteDirect(lszSQL.c_str(), lszSQL.size()))
		{
			LOG_TRACE(3,0,__FUNCTION__,"Error ExecuteDirect fail"
									  << " error="<<loMySQL->GetLastErrMsg()
									  << " SQL="<<lszSQL);
			return -1;
		}
	}	
	catch (...)
	{
		LOG_TRACE(3,0,__FUNCTION__,"Error Exception Error:"
			                      << " error=" << loMySQL->GetLastErrMsg()
			                      << " SQL=" << lszSQL);
		return -1;	
	}
	
	return 0;
}

int32 CDatabaseMgr::GetFriendDel(const std::string& szuserid, const std::string& szpeeruserid)
{
	UINT64 aiuserid = STR2UINT64(szuserid);	
	AutoReleaseMySQLItemExt loMySQL(moContactDBPool,GetContactDBPoolIndex(aiuserid));

	if (loMySQL.Invalid())
	{
		LOG_TRACE(3,0,__FUNCTION__," AutoReleaseMySQLItemExt CreateError.\n");
		return -1;
	}

	
	std::string sztableindex = GetTableIndex(aiuserid);

	std::string lszSQL = "delete from nim_friend_" + sztableindex                       
						  + " where friendid=" + szpeeruserid
						  + " and userid=" + szuserid;
	
	LOG_TRACE(3,0,__FUNCTION__,"DB:im_db" <<  GetContactDBPoolIndex(aiuserid) << " Exect sql:" << lszSQL);

	try
	{
		if(loMySQL->StmtPrepare(lszSQL.c_str(), lszSQL.size()))
		{
			LOG_TRACE(4,false,__FUNCTION__," Error StmtPrepare fail"
										  << " error="<<loMySQL->GetLastErrMsg()
										  << " SQL="<<lszSQL);
			return -1;
		}

		if(loMySQL->ExecuteDirect(lszSQL.c_str(), lszSQL.size()))
		{
			LOG_TRACE(3,0,__FUNCTION__,"Error ExecuteDirect fail"
									  << " error="<<loMySQL->GetLastErrMsg()
									  << " SQL="<<lszSQL);
			return -1;
		}
	}	
	catch (...)
	{
		LOG_TRACE(3,0,__FUNCTION__,"Error Exception Error:"
			                      << " error=" << loMySQL->GetLastErrMsg()
			                      << " SQL=" << lszSQL);
		return -1;	
	}
	
	return 0;
}

int32 CDatabaseMgr::GetFriendAdd(const std::string& szuserid, STRFRIENDINFO& peerinfo)
{
	if (1 == IsFriend(szuserid, peerinfo.szuserid))
	{
        LOG_TRACE(LOG_ERR,0,__FUNCTION__," userid:" << szuserid 
                                 << " peeruserid:" << peerinfo.szuserid 
                                 << " already be friend");
		return 0;
	}

	UINT64 aiuserid = STR2UINT64(szuserid);	
	AutoReleaseMySQLItemExt loMySQL(moContactDBPool,GetContactDBPoolIndex(aiuserid));

	if (loMySQL.Invalid())
	{
		LOG_TRACE(3,0,__FUNCTION__," AutoReleaseMySQLItemExt CreateError.\n");
		return -1;
	}

	
	std::string sztableindex = GetTableIndex(aiuserid);
    std::string colum = " (userid,friendid,sourcetype,remarkname)";
    std::string lszSQL = "insert into nim_friend_" + sztableindex
                          + colum
						  + " values(" + szuserid
						  + ", " + peerinfo.szuserid
                          + ", " + peerinfo.szsourcetype
						  + ", \"" + peerinfo.szremarkname + "\")";
	
	LOG_TRACE(3,0,__FUNCTION__,"DB:im_db" <<  GetContactDBPoolIndex(aiuserid) + 1<< " Exect sql:" << lszSQL);

	try
	{
		if(loMySQL->StmtPrepare(lszSQL.c_str(), lszSQL.size()))
		{
			LOG_TRACE(4,false,__FUNCTION__," Error StmtPrepare fail"
										  << " error="<<loMySQL->GetLastErrMsg()
										  << " SQL="<<lszSQL);
			return -1;
		}

		if(loMySQL->ExecuteDirect(lszSQL.c_str(), lszSQL.size()))
		{
			LOG_TRACE(3,0,__FUNCTION__,"Error ExecuteDirect fail"
									  << " error="<<loMySQL->GetLastErrMsg()
									  << " SQL="<<lszSQL);
			return -1;
		}
	}	
	catch (...)
	{
		LOG_TRACE(3,0,__FUNCTION__,"Error Exception Error:"
			                      << " error=" << loMySQL->GetLastErrMsg()
			                      << " SQL=" << lszSQL);
		return -1;	
	}
	
	return 0;

	
}

int32 CDatabaseMgr::IsFriend(const std::string& szuserid, const std::string& szpeeruserid)
{
	UINT64 aiuserid = STR2UINT64(szuserid);	
	AutoReleaseMySQLItemExt loMySQL(moContactDBPool,GetContactDBPoolIndex(aiuserid));

	if (loMySQL.Invalid())
	{
		LOG_TRACE(3,0,__FUNCTION__," AutoReleaseMySQLItemExt CreateError.\n");
		return -1;
	}

	
	std::string sztableindex = GetTableIndex(aiuserid);

	std::string lszSQL = "select 1 from nim_friend_" 
                          + sztableindex 
                          + " where userid=" + szuserid
						  + " and friendid=" + szpeeruserid
                          + " limit 1";

	
	LOG_TRACE(3,0,__FUNCTION__,"DB:im_db" <<  GetContactDBPoolIndex(aiuserid) << " Exect sql:" << lszSQL);

	try
	{
		if(loMySQL->ExecuteDirect(lszSQL.c_str(), lszSQL.size()))
		{
			LOG_TRACE(3,0,__FUNCTION__,"Error ExecuteDirect fail"
									  << " error=" << loMySQL->GetLastErrMsg()
									  << " SQL="<<lszSQL);
			return -1;
		}

		auto_ptr<CMySQLRes> loMySQLRes = loMySQL->StoreResult();
		if(loMySQLRes.get() == NULL)
		{
			LOG_TRACE(3,0,__FUNCTION__,"Get MYSQL Result Error:"
									  << " error=" <<loMySQL->GetLastErrMsg()
				                      << " SQL="<<lszSQL);
			return -1;
		}

		uint64 liResultRow = loMySQLRes->GetNumOfRow();

		if (liResultRow != 0)
		{
			return 1;
		}
	}
	catch (...)
	{
		LOG_TRACE(3,0,__FUNCTION__,"Error Exception Error:"
			                      << " error=" << loMySQL->GetLastErrMsg()
			                      << " SQL=" << lszSQL);
		return -1;	
	}

	return 0;
}




