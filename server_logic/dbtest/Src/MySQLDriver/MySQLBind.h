#ifndef __MYSQLBIND_H
#define __MYSQLBIND_H
#include <mysql.h>
#include <memory>
#include "common/CacheList.h"
#include "common/RefObj.h"
#include "common/Debug_log.h"
#include "MySQLRes.h"

#define DEF_FIELD_MAX	(225)
/////////////////////////////////////////////////////////////////////////////
template<int BindCount>
class CMySQLBind
{
public:
	CMySQLBind()									
	{
		miCount = 0;
		ZeroMemory(&moBind, sizeof(moBind));
	}
	~CMySQLBind()						
	{
	}
	CMySQLBind<BindCount>& Bind(const int64& aiArg)
	{
		U9_CHAR(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_LONGLONG;
		moBind[miCount].buffer= (char *)&aiArg;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const uint64& aiArg)
	{
		U9_CHAR(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_LONGLONG;
		moBind[miCount].buffer= (char *)&aiArg;
		moBind[miCount].is_unsigned = 1;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const GUID & aiArg)
	{
		U9_CHAR(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_LONGLONG;
		moBind[miCount].buffer= (char *)&aiArg;
		moBind[miCount].is_unsigned = 1;
		miCount++;
		return *this;
	}
	CMySQLBind<BindCount>& Bind(const int32& aiArg)
	{
		U9_CHAR(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_LONG;
		moBind[miCount].buffer= (char *)&aiArg;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const uint32& aiArg)
	{
		U9_CHAR(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_LONG;
		moBind[miCount].buffer= (char *)&aiArg;
		moBind[miCount].is_unsigned = 1;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const int16& aiArg)
	{
		U9_CHAR(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_SHORT;
		moBind[miCount].buffer= (char *)&aiArg;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const uint16& aiArg)
	{
		U9_CHAR(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_SHORT;
		moBind[miCount].buffer= (char *)&aiArg;
		moBind[miCount].is_unsigned = 1;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const int8& aiArg)
	{
		U9_CHAR(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_TINY;
		moBind[miCount].buffer= (char *)&aiArg;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const uint8& aiArg)
	{
		U9_CHAR(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_TINY;
		moBind[miCount].buffer= (char *)&aiArg;
		moBind[miCount].is_unsigned = 1;
		miCount++;
		return *this;
	}

	//CMySQLBind<BindCount>& Bind(const U9_CHAR *acArg,const int32 aiLen)
	//{

	//	U9_CHAR(miCount < BindCount);
	//	char lszUTF8[1024+1] = {0};
	//	UNICODE2UTF8(acArg,lszUTF8, 1024+1);
	//	ULONG lulNickNametlength = (ULONG)strlen(lszUTF8);
	//	moBind[miCount].buffer_type= MYSQL_TYPE_VAR_STRING;
	//	moBind[miCount].buffer= lszUTF8;
	//	moBind[miCount].buffer_length= lulNickNametlength;
	//	moBind[miCount].is_null= 0;

	//	miCount++;
	//	return *this;
	//}

	CMySQLBind<BindCount>& Bind(const BYTE *acArg,const int32 aiLen)
	{

		U9_CHAR(miCount < BindCount);

		moBind[miCount].buffer_type= MYSQL_TYPE_BLOB;
		moBind[miCount].buffer= (void*)acArg;
		moBind[miCount].buffer_length= aiLen;
		moBind[miCount].is_null= 0;

		miCount++;
		return *this;
	}

	operator MYSQL_BIND*()
	{
		return moBind;
	}

private:
	MYSQL_BIND  moBind[BindCount];
	int miCount;
};



#endif	// __MYSQLBIND_H