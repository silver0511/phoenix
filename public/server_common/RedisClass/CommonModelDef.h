#ifndef __COMMON_MODEL_DEF_H
#define __COMMON_MODEL_DEF_H
#include "common/System.h"

class CLogicToDBCommon 
{
public:
    static char* GetCmd()
    {
        return "cmd";
    }
    
    static char* GetUserID()
    {
        return "user_id";
    }
};

class CDBToLogicCommon
{
public:
    static char* GetResponseCode()
    {
        return "responseCode";
    }

    static char* GetData()
    {
        return "data";
    }
};


#endif