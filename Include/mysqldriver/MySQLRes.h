// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
// 
// 文件名称: MySQLRes.h
// 创 建 人: 马剑阁
// 文件说明:  MySQL C　API封装连接池头文件
// $_FILEHEADER_END *****************************

#ifndef __MYSQL_RES_H_
#define __MYSQL_RES_H_

#include <mysql/mysql.h>
#include "common/Debug_log.h"
#include "common/CacheList.h"
#include "common/RefObj.h"

#include "DBI.h"

/////////////////////////////////////////////////////////////////////////////
// CMYSQLRes 类

class CMySQLRecordSet:public U9::IRecordSet
{
public:
    CMySQLRecordSet(MYSQL_RES* result);
    virtual ~CMySQLRecordSet();
    virtual uint32 getRecordCount();
    virtual uint32 getFieldCount();
    virtual bool getRecord();
    virtual const char* GetField(uint32 index);
    virtual uint32 getFieldLength(uint32 index);

private:
    MYSQL_RES* _result;
    MYSQL_ROW _row;
};

#endif	// __MYSQL_RES_H_
