#include "stdafx.h"
#include "UserConMgr.h"
#include "network/packet/ClientPack.h"

#define MAX_DELAY_DESTROY_TIME (60)

UserConMgr::UserConMgr()
{
    init();
}
UserConMgr::~UserConMgr()
{
    clear();
}

CUser* UserConMgr::MallocRefObj()
{
    CUser* p_user = NULL;
    CRITICAL_SECTION_HELPER(moCriticalSection);
    p_user = m_cache_list.malloc();
    if(NULL ==p_user)
    {
        LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, "!!moCacheList.malloc Error\n");
        return NULL;
    }
    p_user->init();
    return p_user;
}


bool UserConMgr::DeleteRefObj(CUser *apUser)
{
    //因为是在delUser中调用所以这里不加锁
    apUser->DecreaseRef();
    if(!apUser->CanRelease())
    {
        LOG_TRACE(LOG_ERR,true, __FUNCTION__, " can not release user"
                <<" Socket="<<apUser->mhSocket
                <<" miCometID="<<apUser->miCometID
                << " IP="<< GetIPString(apUser->miIPAddr)
                << ":"<< ntohs(apUser->miIPPort)
                <<" RefCount="<<apUser->GetRefCount());
    }
    apUser->clear();
    m_cache_list.free(apUser);
    return true;
}

//按一个连接服2个tcp连接计算
void UserConMgr::init()
{
    vec_max_size = 1000;
    m_cache_list.init(1000/2);
}

void UserConMgr::init(int max_size)
{
    vec_max_size = max_size;
    m_cache_list.init((max_size / 2) + 1);
}

void UserConMgr::clear()
{
    CSERVERMAP::iterator server_iter;
    for(server_iter = moServerMap.begin(); server_iter != moServerMap.end(); server_iter++)
    {
        server_iter->second.clear();
    }
    moServerMap.clear();
    m_cache_list.clear();
}

BOOL UserConMgr::addUser(CUser *apUser)
{
    CRITICAL_SECTION_HELPER(moCriticalSection);
    U9_ASSERT(NULL != apUser);
    CSERVERMAP::iterator server_iter = moServerMap.find(apUser->miCometID);
    if(server_iter == moServerMap.end())
    {
        CSERVERVEC moServerList;
        moServerList.reserve(vec_max_size);
        moServerList.push_back(apUser);
        moServerMap.insert(CSERVERMAP::value_type(apUser->miCometID, moServerList));
        LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " first insert"
                <<" reserve="<<vec_max_size
                <<" Socket="<<apUser->mhSocket
                <<" miCometID="<<apUser->miCometID
                << " IP="<< GetIPString(apUser->miIPAddr)
                << ":"<< ntohs(apUser->miIPPort)
                <<" RefCount="<<apUser->GetRefCount()
        );
    }
    else
    {
        auto p_server_list = &server_iter->second;
        for(int s_index = 0; s_index < p_server_list->size(); s_index++)
        {
            if(NULL == (*p_server_list)[s_index])
            {
                continue;
            }

            if((*p_server_list)[s_index]->mhSocket == apUser->mhSocket &&
               (*p_server_list)[s_index]->miIPAddr == apUser->miIPAddr &&
               (*p_server_list)[s_index]->miIPPort == apUser->miIPPort)
            {
                LOG_TRACE(LOG_ERR,true, __FUNCTION__, " user already exsist"
                        <<" UserPtr="<<(void*)apUser
                        <<" Socket="<<apUser->mhSocket
                        <<" miCometID="<<apUser->miCometID
                        << " IP="<< GetIPString(apUser->miIPAddr)
                        << ":"<< ntohs(apUser->miIPPort)
                        <<" RefCount="<<apUser->GetRefCount()
                );
                return FALSE;
            }
        }

        p_server_list->push_back(apUser);
    }

    LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " add user"
            <<" UserPtr="<<(void*)apUser
            <<" Socket="<<apUser->mhSocket
            <<" miCometID="<<apUser->miCometID
            << " IP="<< GetIPString(apUser->miIPAddr)
            << ":"<< ntohs(apUser->miIPPort)
            <<" RefCount="<<apUser->GetRefCount()
    );
    apUser->mbIsinMgr = TRUE;
    apUser->IncreaseRef();

    return TRUE;
}

BOOL UserConMgr::delUser(CUser *apUser)
{
    CRITICAL_SECTION_HELPER(moCriticalSection);
    STRU_DESTROY_USER destroy_user;
    destroy_user.p_user = apUser;
    destroy_user.start_time = CSystem::GetTime();
    destroy_queue.push(destroy_user);

	if(!apUser->mbIsinMgr)
    {
        return FALSE;
    }

    CSERVERMAP::iterator server_iter = moServerMap.find(apUser->miCometID);
    if(server_iter == moServerMap.end())
    {
        LOG_TRACE(LOG_ERR,true, __FUNCTION__, " server not in map"
                <<" UserPtr="<<(void*)apUser
                <<" Socket="<<apUser->mhSocket
                <<" miCometID="<<apUser->miCometID
                << " IP="<< GetIPString(apUser->miIPAddr)
                << ":"<< ntohs(apUser->miIPPort)
                <<" RefCount="<<apUser->GetRefCount()
        );
        return FALSE;
    }

    auto p_server_list = &server_iter->second;
    CSERVERVEC::iterator value_iter = p_server_list->begin();
    for(; value_iter != p_server_list->end(); value_iter++)
    {
        if((*value_iter)->mhSocket == apUser->mhSocket &&
           (*value_iter)->miIPAddr == apUser->miIPAddr &&
           (*value_iter)->miIPPort == apUser->miIPPort)
        {
            LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " delete user"
                    <<" UserPtr="<<(void*)apUser
                    <<" Socket="<<apUser->mhSocket
                    <<" UserID="<<apUser->miUserID
                    << " IP="<< GetIPString(apUser->miIPAddr)
                    << ":"<< ntohs(apUser->miIPPort)
                    <<" RefCount="<<apUser->GetRefCount()
            );
            p_server_list->erase(value_iter);
            apUser->mbIsinMgr = FALSE;
            apUser->DecreaseRef();
            return TRUE;
        }
    }

    LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " delete not in list"
            <<" UserPtr="<<(void*)apUser
            <<" Socket="<<apUser->mhSocket
            <<" UserID="<<apUser->miUserID
            << " IP="<< GetIPString(apUser->miIPAddr)
            << ":"<< ntohs(apUser->miIPPort)
            <<" RefCount="<<apUser->GetRefCount()
    );
    return FALSE;
}

CUser* UserConMgr::getUser(const SESSIONID &s_id, const USERID &u_id)
{
    CSERVERMAP::iterator server_iter = moServerMap.find(s_id);
    if(server_iter == moServerMap.end())
    {
        LOG_TRACE(LOG_ERR,true, __FUNCTION__, " server not in map"
                <<" s_id="<<s_id);
        return NULL;
    }

    auto p_server_list = &server_iter->second;
    if(p_server_list->size() <= 0)
    {
        LOG_TRACE(LOG_ERR,true, __FUNCTION__, " server not in map"
                <<" s_id="<<s_id);
        return NULL;
    }

    INT index = u_id % p_server_list->size();

    return (*p_server_list)[index];
}

void UserConMgr::CheckEvent()
{
    if(destroy_queue.empty())
    {
        return;
    }

    CRITICAL_SECTION_HELPER(moCriticalSection);
    time_t cur_time = CSystem::GetTime();
    const STRU_DESTROY_USER &destroy_user = destroy_queue.front();
    if((cur_time - destroy_user.start_time) < MAX_DELAY_DESTROY_TIME)
    {
        return;
    }

    LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " delete user impl"
            <<" UserPtr="<<(void*)destroy_user.p_user
            <<" Socket="<<destroy_user.p_user->mhSocket
            <<" UserID="<<destroy_user.p_user->miUserID
            << " IP="<< GetIPString(destroy_user.p_user->miIPAddr)
            << ":"<< ntohs(destroy_user.p_user->miIPPort)
            <<" RefCount="<<destroy_user.p_user->GetRefCount()
    );
    destroy_user.p_user->GetCriticalSection().Enter();
    ClientPack::GetInstance().DelRelationPtr(destroy_user.p_user->mpSocketInfo, destroy_user.p_user);
    destroy_user.p_user->GetCriticalSection().Leave();
    DeleteRefObj(destroy_user.p_user);
    destroy_queue.pop();
}

void UserConMgr::dumpUser()
{
    CSERVERMAP::iterator server_iter;
    for(server_iter = moServerMap.begin(); server_iter != moServerMap.end(); server_iter++)
    {
        auto server_list = server_iter->second;
        CSERVERVEC::iterator value_iter = server_list.begin();
        LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " ======dump key====:" << server_iter->first);
        for(; value_iter != server_list.end(); value_iter++)
        {
            LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " dump value: "
                    <<" UserPtr="<<(void*)(*value_iter)
                    <<" Socket="<<(*value_iter)->mhSocket
                    <<" UserID="<<(*value_iter)->miUserID
                    << " IP="<< GetIPString((*value_iter)->miIPAddr)
                    << ":"<< ntohs((*value_iter)->miIPPort)
                    <<" RefCount="<<(*value_iter)->GetRefCount());
        }
    }
}