#include "common/System.h"
#include "mysqldriver/MySQLRes.h"

CMySQLRecordSet::CMySQLRecordSet(MYSQL_RES* result)
{
    _result = result;
    _row = NULL;
}

CMySQLRecordSet::~CMySQLRecordSet()
{
    if(_result)
    {
        mysql_free_result(_result);
    }
}

uint32 CMySQLRecordSet::getRecordCount()
{
    return (uint32)mysql_num_rows(_result);
}

uint32 CMySQLRecordSet::getFieldCount()
{
    return (uint32)mysql_num_fields(_result);
}

bool CMySQLRecordSet::getRecord()
{
    _row = mysql_fetch_row(_result);
    if(_row == NULL)
    {
        return false;
    }
    return true;
}

const char* CMySQLRecordSet::GetField(uint32 index)
{
    if(index >= getFieldCount()
        || _row == NULL)
    {
        return NULL;
    }

    return _row[index];
}

uint32 CMySQLRecordSet::getFieldLength(uint32 index)
{
    if(index >= getFieldCount())
    {
        return 0;
    }

    unsigned long* lengths = mysql_fetch_lengths(_result);
    return (uint32)lengths[index];
}
