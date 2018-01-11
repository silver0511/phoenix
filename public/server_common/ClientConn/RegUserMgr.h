//
// Created by shiyunjie on 2018/1/11.
//

#ifndef __REGUSERMGR_H__
#define __REGUSERMGR_H__

#include "User.h"
class RegUserMgr
{
public:
    RegUserMgr();
    ~RegUserMgr();
    void init();
    virtual void clear();
    //加入信息
    BOOL addUser(CUser *apUser);
    CUser* delUser(USERID user_id);
    CUser* getUser(USERID user_id);
public:
    friend class CServerMgr;

private:
    CCriticalSection            m_reg_lock;
    u9_hash_map<USERID, CUser*> m_reg_user_map;
    typedef typename u9_hash_map<USERID, CUser*>::iterator RegUserIter;
};


#endif //__REGUSERMGR_H__
