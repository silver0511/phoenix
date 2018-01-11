#ifndef __FDSERVERPACKDEF_H
#define __FDSERVERPACKDEF_H
#include "./CommonPackDef.h"
#include "Package/hfbs/fd/fb_friend_info_generated.h"
#include "Package/hfbs/fd/fb_friend_list_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_list_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_remark_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_remark_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_del_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_del_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_client_add_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_client_add_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_server_add_confirm_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_server_add_confirm_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_server_add_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_server_add_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_client_add_confirm_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_client_add_confirm_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_server_del_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_server_del_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_client_blacklist_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_client_blacklist_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_server_blacklist_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_update_rq_generated.h"
#include "Package/hfbs/fd/fb_friend_update_rs_generated.h"
#include "Package/hfbs/fd/fb_friend_server_restor_rq_generated.h"



using namespace friendpack;

struct STRU_FRIEND_LIST_RQ : public STRU_PACKAGE_RQ<T_FRIEND_LIST_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_FRIEND_LIST_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_FRIEND_LIST_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_FRIEND_LIST_RQ*)GetT_FRIEND_LIST_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};


struct STRU_FRIEND_REMARK_RQ : public STRU_PACKAGE_RQ<T_FRIEND_REMARK_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_FRIEND_REMARK_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_FRIEND_REMARK_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_FRIEND_REMARK_RQ*)GetT_FRIEND_REMARK_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_FRIEND_DEL_RQ : public STRU_PACKAGE_RQ<T_FRIEND_DEL_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_FRIEND_DEL_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_FRIEND_DEL_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_FRIEND_DEL_RQ*)GetT_FRIEND_DEL_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};


struct STRU_FRIEND_CLIENT_ADD_RQ : public STRU_PACKAGE_RQ<T_FRIEND_CLIENT_ADD_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_CLIENT_FRIEND_ADD_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_FRIEND_CLIENT_ADD_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_FRIEND_CLIENT_ADD_RQ*)GetT_FRIEND_CLIENT_ADD_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_FRIEND_CLIENT_CONFIRM_RQ : public STRU_PACKAGE_RQ<T_FRIEND_CLIENT_CONFIRM_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_CLIENT_FRIEND_CONFIRM_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_FRIEND_CLIENT_CONFIRM_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_FRIEND_CLIENT_CONFIRM_RQ*)GetT_FRIEND_CLIENT_CONFIRM_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_CLIENT_FRIEND_BLACKLIST_RQ : public STRU_PACKAGE_RQ<T_CLIENT_FRIEND_BLACKLIST_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_CLIENT_FRIEND_BLACKLIST_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_CLIENT_FRIEND_BLACKLIST_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_CLIENT_FRIEND_BLACKLIST_RQ*)GetT_CLIENT_FRIEND_BLACKLIST_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_FRIEND_UPDATE_RQ : public STRU_PACKAGE_RQ<T_FRIEND_UPDATE_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_FRIEND_UPDATE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_FRIEND_UPDATE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_FRIEND_UPDATE_RQ*)GetT_FRIEND_UPDATE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};




#endif

