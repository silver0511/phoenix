//
// Created by shiyunjie on 2018/1/8.
//

#ifndef __SMS_VERIFY_TASK_H__
#define __SMS_VERIFY_TASK_H__

#include "Task.h"

class SmsVerifyTask : public CTask
{
public:
    SmsVerifyTask():CTask()
    {

    }

    virtual ~SmsVerifyTask()
    {

    }

    SmsVerifyTask(const SmsVerifyTask& rhs):CTask(rhs)
    {

    }

    SmsVerifyTask& operator=(const SmsVerifyTask& rhs)
    {
        if ( this == &rhs)
        {
            return *this;
        }

        CTask::operator=(rhs);
        return *this;
    }

    virtual void run();
    void DoForwardPackageFail();

public:
    string m_nation_code;
    string m_mobile;
    string verify_code;
    string valid_time;
};


#endif //__SMS_VERIFY_TASK_H__
