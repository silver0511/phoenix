#ifndef __ECSERVERPACKDEF_H
#define __ECSERVERPACKDEF_H

#include "./CommonPackDef.h"
#include "Package/hfbs/ec/fb_ec_getfreewaiter_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_getfreewaiter_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_getrecentmsgs_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_getrecentmsgs_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_getlastmsgs_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_getlastmsgs_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_get_offline_message_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_get_offline_message_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_get_offline_cnt_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_get_offline_cnt_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_getrecentusers_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_getrecentusers_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_set_loginstatus_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_set_loginstatus_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_msginfo_generated.h"
#include "Package/hfbs/ec/fb_ec_msginfo_generated.h"
#include "Package/hfbs/ec/fb_ec_getrecentorders_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_getrecentorders_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_orderinfo_generated.h"
#include "Package/hfbs/ec/fb_ec_goodsinfotoims_generated.h"
#include "Package/hfbs/ec/fb_ec_isuserbusy_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_isuserbusy_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_setbusiness_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_setbusiness_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_getbusiness_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_getbusiness_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_setprompts_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_setprompts_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_getprompts_rq_generated.h"
#include "Package/hfbs/ec/fb_ec_getprompts_rs_generated.h"
#include "Package/hfbs/ec/fb_ec_promptmsginfo_generated.h"

using namespace ecpack;

struct STRU_EC_GETFREEWAITER_RQ : public STRU_PACKAGE_RQ<T_EC_GETFREEWAITER_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_GETFREEWAITER_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_GETFREEWAITER_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_GETFREEWAITER_RQ*)GetT_EC_GETFREEWAITER_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_EC_GETRECENTMSGS_RQ : public STRU_PACKAGE_RQ<T_EC_GETRECENTMSGS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_GETRECENTMSGS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_GETRECENTMSGS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_GETRECENTMSGS_RQ*)GetT_EC_GETRECENTMSGS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_EC_GETRECENTORDERS_RQ : public STRU_PACKAGE_RQ<T_EC_GETRECENTORDERS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_GETRECENTORDERS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_GETRECENTORDERS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_GETRECENTORDERS_RQ*)GetT_EC_GETRECENTORDERS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_EC_ISUSERBUSY_RQ : public STRU_PACKAGE_RQ<T_EC_ISUSERBUSY_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_ISUSERBUSY_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_ISUSERBUSY_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_ISUSERBUSY_RQ*)GetT_EC_ISUSERBUSY_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};



struct STRU_EC_GETLASTMSGS_RQ : public STRU_PACKAGE_RQ<T_EC_GETLASTMSGS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_GETLASTMSGS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_GETLASTMSGS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_GETLASTMSGS_RQ*)GetT_EC_GETLASTMSGS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_EC_GETOFFLINEMSGS_RQ : public STRU_PACKAGE_RQ<T_EC_GET_OFFLINE_MESSAGE_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_GETOFFLINEMSGS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_GET_OFFLINE_MESSAGE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_GET_OFFLINE_MESSAGE_RQ*)GetT_EC_GET_OFFLINE_MESSAGE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};


struct STRU_EC_GETOFFLINECNT_RQ : public STRU_PACKAGE_RQ<T_EC_GET_OFFLINE_CNT_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_GETOFFLINECNT_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_GET_OFFLINE_CNT_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_GET_OFFLINE_CNT_RQ*)GetT_EC_GET_OFFLINE_CNT_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};


struct STRU_EC_GETRECENTUSERS_RQ : public STRU_PACKAGE_RQ<T_EC_GETRECENTUSERS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_GETRECENTUSERS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_GETRECENTUSERS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_GETRECENTUSERS_RQ*)GetT_EC_GETRECENTUSERS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_EC_SETLOGINSTATUS_RQ : public STRU_PACKAGE_RQ<T_EC_SET_LOGINSTATUS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_SETLOGINSTATUS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_SET_LOGINSTATUS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_SET_LOGINSTATUS_RQ*)GetT_EC_SET_LOGINSTATUS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};


//登录更新ID包[wscomet使用]
struct STRU_BUSINESS_UPDATE_LOGIN_ID
{
public:
    BYTE    mbType;
    uint64	miTime;
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_LOGIN_ID);

    STRU_BUSINESS_UPDATE_LOGIN_ID()
    {
        mbType = 0;
        miTime = 0;
    }

    INT Serialize(ISerialize & aoSerialize)
    {
        aoSerialize.Serialize(mbType);
        aoSerialize.Serialize(miTime);
        return 1;
    }
};

struct STRU_EC_GETBUSINESS_RQ : public STRU_PACKAGE_RQ<T_EC_GETBUSINESS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_GETINFO_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_GETBUSINESS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_GETBUSINESS_RQ*)GetT_EC_GETBUSINESS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_EC_SETBUSINESS_RQ : public STRU_PACKAGE_RQ<T_EC_SETBUSINESS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_SETINFO_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_SETBUSINESS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_SETBUSINESS_RQ*)GetT_EC_SETBUSINESS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_EC_SETPROMPTS_RQ : public STRU_PACKAGE_RQ<T_EC_SETPROMPTS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_SETPROMPTS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_SETPROMPTS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_SETPROMPTS_RQ*)GetT_EC_SETPROMPTS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }

    INT Check()
	{   
		int result = STRU_PACKAGE_RQ::Check();
		if(result < 0)
		{
			return -1;
		}

        return result;
    }
};

struct STRU_EC_GETPROMPTS_RQ : public STRU_PACKAGE_RQ<T_EC_GETPROMPTS_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_BUSINESS_GETPROMPTS_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_EC_GETPROMPTS_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_EC_GETPROMPTS_RQ*)GetT_EC_GETPROMPTS_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }

    INT Check()
	{   
		int result = STRU_PACKAGE_RQ::Check();
		if(result < 0)
		{
			return -1;
		}

        return result;
    }
};




#endif

