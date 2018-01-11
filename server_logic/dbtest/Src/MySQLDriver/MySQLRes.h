#ifndef __MYSQLRES_H
#define __MYSQLRES_H
#include <mysql.h>
#include "common/CacheList.h"
#include "common/RefObj.h"
#include "common/Debug_log.h"

/////////////////////////////////////////////////////////////////////////////
// CMYSQLRes ��
class CMySQLRes
{
public:
	CMySQLRes( MYSQL_RES* apMySQLRes)
	{
		mpMySQLRes = apMySQLRes;
		ZeroMemory(mszLastErrMsg, sizeof(mszLastErrMsg));
	}

	~CMySQLRes()
	{
		mysql_free_result(mpMySQLRes);
	}

	MYSQL_ROW FetchRow()
	{
		return mysql_fetch_row(mpMySQLRes);
	}

	unsigned long* FetchLengths()
	{
		return mysql_fetch_lengths(mpMySQLRes);
	}

	uint64 GetNumOfRow()
	{
		return mysql_num_rows(mpMySQLRes);
	}

	uint GetNumFields()
	{
		return mysql_num_fields(mpMySQLRes);
	}


private:
	MYSQL_RES* mpMySQLRes;
	static const uint MAXERRMSGLENGTH = 4096;
	char mszLastErrMsg[MAXERRMSGLENGTH];
};

#endif	// __MYSQL_RES_H_