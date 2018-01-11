//
// Created by shiyunjie on 2018/1/11.
//

#include "stdafx.h"
#include "RegUserMgr.h"

RegUserMgr::RegUserMgr()
{

}

RegUserMgr::~RegUserMgr()
{
    clear();
}

void RegUserMgr::init()
{
}

void RegUserMgr::clear()
{
    CRITICAL_SECTION_HELPER(m_reg_lock);
    m_reg_user_map.clear();
}

BOOL RegUserMgr::addUser(CUser *apUser)
{
    U9_ASSERT(NULL != apUser);
    if(apUser->miTempUserID <= 0)
    {
        LOG_TRACE(LOG_ERR,true, __FUNCTION__, " invalid user"
                <<" UserPtr="<<(void*)apUser
                <<" Socket="<<apUser->mhSocket
                <<" RegUserID="<<apUser->miTempUserID
                <<" UserID="<<apUser->miUserID
                << " IP="<< GetIPString(apUser->miIPAddr)
                << ":"<< ntohs(apUser->miIPPort)
                <<" RefCount="<<apUser->GetRefCount()
        );
        return FALSE;
    }
    CRITICAL_SECTION_HELPER(m_reg_lock);
    RegUserIter iter = m_reg_user_map.find(apUser->miTempUserID);
    if(iter != m_reg_user_map.end())
    {
        CUser *lpOldUser = iter->second;
        //有老用户
        LOG_TRACE(LOG_ERR,true, __FUNCTION__, " change user"
                <<" UserPtr="<<(void*)apUser
                <<" Socket="<<apUser->mhSocket
                <<" RegUserID="<<apUser->miTempUserID
                <<" UserID="<<apUser->miUserID
                << " IP="<< GetIPString(apUser->miIPAddr)
                << ":"<< ntohs(apUser->miIPPort)
                <<" RefCount="<<apUser->GetRefCount()
                <<" Old UserPtr="<<(void*)lpOldUser
                <<" Old Socket="<<lpOldUser->mhSocket
                <<" Old IP="<< GetIPString(lpOldUser->miIPAddr)
                << ":"<< ntohs(lpOldUser->miIPPort)
                <<" Old RefCount="<<lpOldUser->GetRefCount()
        );
        m_reg_user_map.erase(iter);
        lpOldUser->DecreaseRef();
    }

    m_reg_user_map.insert(make_pair(apUser->miTempUserID, apUser));
    apUser->IncreaseRef();
    LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " add user"
            <<" UserPtr="<<(void*)apUser
            <<" Socket="<<apUser->mhSocket
            <<" RegUserID="<<apUser->miTempUserID
            <<" UserID="<<apUser->miUserID
            << " IP="<< GetIPString(apUser->miIPAddr)
            << ":"<< ntohs(apUser->miIPPort)
            <<" RefCount="<<apUser->GetRefCount()
    );

    return TRUE;
}

CUser* RegUserMgr::delUser(USERID user_id)
{
    if(user_id <= 0)
    {
        return FALSE;
    }

    CRITICAL_SECTION_HELPER(m_reg_lock);
    RegUserIter iter = m_reg_user_map.find(user_id);
    if(iter == m_reg_user_map.end())
    {
        return FALSE;
    }
    CUser *lp_user = iter->second;
    LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " delete user"
            <<" UserPtr="<<(void*)lp_user
            <<" Socket="<<lp_user->mhSocket
            <<" RegUserID="<<lp_user->miTempUserID
            <<" UserID="<<lp_user->miUserID
            << " IP="<< GetIPString(lp_user->miIPAddr)
            << ":"<< ntohs(lp_user->miIPPort)
            <<" RefCount="<<lp_user->GetRefCount()
    );
    m_reg_user_map.erase(iter);
    lp_user->DecreaseRef();
    return lp_user;
}


CUser* RegUserMgr::getUser(USERID user_id)
{
    if(user_id <= 0)
    {
       return NULL;
    }

    CRITICAL_SECTION_HELPER(m_reg_lock);
    RegUserIter iter = m_reg_user_map.find(user_id);
    if(iter == m_reg_user_map.end())
    {
        return NULL;
    }

    return iter->second;
}