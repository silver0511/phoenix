// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
// 
// 文件名称: MySQLBind.h
// 创 建 人: 史云杰
// 文件说明:  MySQL C　API封装连接池头文件
// $_FILEHEADER_END *****************************

#ifndef __MYSQL_BIND_H_
#define __MYSQL_BIND_H_
#include <mysql/mysql.h>
#include <memory>
#include "common\CacheList.h"
#include "common\RefObj.h"
#include "common\Debug_log.h"
#include ".\MySQLRes.h"

#define DEF_FIELD_MAX	225

/////////////////////////////////////////////////////////////////////////////
// CMySQLBind 类
template<int BindCount>
class CMySQLBind
{
public:
	CMySQLBind()									
	{
		miCount = 0;
		memset(&moBind, 0, sizeof(moBind));
	}
	virtual ~CMySQLBind()
	{
		miCount = 0;
		memset(&moBind, 0, sizeof(moBind));
	}
	CMySQLBind<BindCount>& Bind(const int64_t& aiArg)
	{
		assert(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_LONGLONG;
		moBind[miCount].buffer= (char *)&aiArg;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const uint64& aiArg)
	{
		assert(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_LONGLONG;
		moBind[miCount].buffer= (char *)&aiArg;
		moBind[miCount].is_unsigned = 1;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const int32_t& aiArg)
	{
		assert(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_LONG;
		moBind[miCount].buffer= (char *)&aiArg;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const uint32& aiArg)
	{
		assert(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_LONG;
		moBind[miCount].buffer= (char *)&aiArg;
		moBind[miCount].is_unsigned = 1;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const int16_t& aiArg)
	{
		assert(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_SHORT;
		moBind[miCount].buffer= (char *)&aiArg;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const uint16_t& aiArg)
	{
		assert(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_SHORT;
		moBind[miCount].buffer= (char *)&aiArg;
		moBind[miCount].is_unsigned = 1;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const int8_t& aiArg)
	{
		assert(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_TINY;
		moBind[miCount].buffer= (char *)&aiArg;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const uint8_t& aiArg)
	{
		assert(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_TINY;
		moBind[miCount].buffer= (char *)&aiArg;
		moBind[miCount].is_unsigned = 1;
		miCount++;
		return *this;
	}

	CMySQLBind<BindCount>& Bind(const char *acArg,const int32 aiLen)
	{

		assert(miCount < BindCount);
		moBind[miCount].buffer_type= MYSQL_TYPE_VAR_STRING;
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



#endif	// __MYSQLDRIVER_H_
