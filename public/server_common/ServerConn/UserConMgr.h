//
// Created by shiyunjie on 17/1/9.
//

#ifndef QBIM_USERCONMGR_H
#define QBIM_USERCONMGR_H

#include "User.h"
#include<queue>

class UserConMgr
{
private:
    struct STRU_DESTROY_USER
    {
        CUser *p_user;
        time_t start_time;
        STRU_DESTROY_USER()
        {
            p_user = NULL;
            start_time = 0;
        }
    };

public:
    UserConMgr();
    ~UserConMgr();
    CUser* MallocRefObj();
    void init();
    void init(int max_size);
    void clear();
    BOOL addUser(CUser *apUser);
    BOOL delUser(CUser *apUser);
    CUser* getUser(const SESSIONID &s_id, const USERID &u_id);
    void CheckEvent();
    void dumpUser();
private:
    bool DeleteRefObj(CUser *apUser);
private:
    typedef std::vector<CUser *> CSERVERVEC;
    typedef std::map<SESSIONID, CSERVERVEC> CSERVERMAP;
    CSERVERMAP moServerMap;
    int vec_max_size;
    CCriticalSection    moCriticalSection;
    CCachePool<CUser>	m_cache_list;

    typedef std::queue<STRU_DESTROY_USER> CDESTROYQUEUE;
    CDESTROYQUEUE destroy_queue;
};


#endif //QBIM_USERCONMGR_H
