#ifndef __SYSPACKDEF_H
#define __SYSPACKDEF_H

#include "./CommonPackDef.h"
#include "const/SvrConstDef.h"
#include "network/packet/PackTypeDef.h"
#include "network/packet/BasePackDef.h"
#include "Package/hfbs/syspack/fb_login_rq_generated.h"
#include "Package/hfbs/syspack/fb_login_rs_generated.h"
#include "Package/hfbs/syspack/fb_time_sync_rq_generated.h"
#include "Package/hfbs/syspack/fb_time_sync_rs_generated.h"
#include "Package/hfbs/syspack/fb_server_discon_id_generated.h"
#include "Package/hfbs/syspack/fb_sms_valid_rq_generated.h"
#include "Package/hfbs/syspack/fb_sms_valid_rs_generated.h"
#include "Package/hfbs/syspack/fb_reg_rq_generated.h"
#include "Package/hfbs/syspack/fb_reg_rs_generated.h"

using namespace syspack;

//登陆请求包
struct STRU_LOGIN_RQ : public STRU_PACKAGE_RQ<T_LOGIN_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_LOGIN_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = syspack::VerifyT_LOGIN_RQBuffer(verifier);
        if(!is_fbs)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "in validate");
            return -2;
        }

        t_fb_data = (syspack::T_LOGIN_RQ*)syspack::GetT_LOGIN_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        if(NULL == t_fb_data->cookie() || t_fb_data->cookie()->Length() <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "cookie is empty:" << t_fb_data->s_rq_head()->user_id());
            return -1;
        }

        if(NULL == t_fb_data->tgt() || NULL == t_fb_data->tgt()->c_str())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "tgt is empty:" << t_fb_data->s_rq_head()->user_id());
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_LOGIN_RS : public STRU_PACKAGE_RS<T_LOGIN_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_LOGIN_RS);

    STRU_LOGIN_RS() : STRU_PACKAGE_RS<T_LOGIN_RS>(NEW_DEF_LOGIN_RS)
    {

    }
};

//心跳RQ
struct STRU_HEART_RS
{
public:
public:
    MAPPING_PACKTYPE(NEW_DEF_HEART_RS)
    STRU_HEART_RS()
    {

    };

    INT Serialize(ISerialize & aoSerialize)
    {
        return 1;
    }

    WORD GetPackType()
    {
        return NEW_DEF_HEART_RS;
    }
};

struct STRU_ACK_ID
{
public:
public:
    MAPPING_PACKTYPE(NEW_DEF_ACK_ID)
    STRU_ACK_ID()
    {

    };

    INT Serialize(ISerialize & aoSerialize)
    {
        return 1;
    }
};

//踢人ID包
struct STRU_SERVER_DISCON_ID : public STRU_PACKAGE_RS<T_SERVER_DISCON_ID>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_SERVER_DISCON_ID)

    STRU_SERVER_DISCON_ID() : STRU_PACKAGE_RS<T_SERVER_DISCON_ID>(NEW_DEF_SERVER_DISCON_ID)
    {

    }
};

//转发包
struct STRU_TRANS_RS
{
public:
    BYTE*   mpBuffer;
    WORD    mwLen;
public:
    WORD ENUM_TYPE;
    STRU_TRANS_RS()
    {
        mpBuffer = NULL;
        mwLen = 0;
        ENUM_TYPE = 0;
    };

    INT Serialize(ISerialize & aoSerialize)
    {
        return aoSerialize.Serialize(mpBuffer, mwLen, mwLen);
    }
};


//时间同步包
struct STRU_TIME_SYNC_RQ : public STRU_PACKAGE_RQ<T_TIME_SYNC_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_TIME_SYNC_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = syspack::VerifyT_TIME_SYNC_RQBuffer(verifier);
        if(!is_fbs)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "in validate");
            return -2;
        }

        t_fb_data = (syspack::T_TIME_SYNC_RQ*)syspack::GetT_TIME_SYNC_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_TIME_SYNC_RS : public STRU_PACKAGE_RS<T_TIME_SYNC_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_TIME_SYNC_RS);

    STRU_TIME_SYNC_RS() : STRU_PACKAGE_RS<T_TIME_SYNC_RS>(NEW_DEF_TIME_SYNC_RS)
    {

    }
};

//路由更新ID包[comet-->route[pcroute]]
struct STRU_ROUTE_SYNC_ID
{
    BYTE    mbType;
public:
    MAPPING_PACKTYPE(NEW_DEF_ROUTE_SYNC_ID);

    STRU_ROUTE_SYNC_ID()
    {
        mbType = 0;
    }

    INT Serialize(ISerialize & aoSerialize)
    {
        aoSerialize.Serialize(mbType);
        return 1;
    }
};


//短信获取验证码包
struct STRU_SMS_VALID_RQ : public STRU_PACKAGE_RQ<T_SMS_VALID_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_SMS_VALID_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = syspack::VerifyT_SMS_VALID_RQBuffer(verifier);
        if(!is_fbs)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "in validate");
            return -2;
        }

        t_fb_data = (syspack::T_SMS_VALID_RQ*)syspack::GetT_SMS_VALID_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            if(t_fb_data->nation_code() <= 0)
            {
                LOG_TRACE(LOG_ERR, false, __FUNCTION__, "nation_code invalid");
                return -1;
            }

            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_SMS_VALID_RS : public STRU_PACKAGE_RS<T_SMS_VALID_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_SMS_VALID_RS);

    STRU_SMS_VALID_RS() : STRU_PACKAGE_RS<T_SMS_VALID_RS>(NEW_DEF_SMS_VALID_RS)
    {

    }
};

//注册包
struct STRU_REG_RQ : public STRU_PACKAGE_RQ<T_REG_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_REG_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = syspack::VerifyT_REG_RQBuffer(verifier);
        if(!is_fbs)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "in validate");
            return -2;
        }

        t_fb_data = (syspack::T_REG_RQ*)syspack::GetT_REG_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            if(NULL == t_fb_data->account_info())
            {
                LOG_TRACE(LOG_ERR, false, __FUNCTION__, "account_info invalid");
                return -1;
            }

            if(NULL == t_fb_data->account_info()->target_user_token())
            {
                LOG_TRACE(LOG_ERR, false, __FUNCTION__, "target_user_token null");
                return -1;
            }


            if(NULL == t_fb_data->account_info()->target_user_token()->c_str() ||
                    "" == t_fb_data->account_info()->target_user_token()->c_str())
            {
                LOG_TRACE(LOG_ERR, false, __FUNCTION__, "target_user_token invalid");
                return -1;
            }
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
};

struct STRU_REG_RS : public STRU_PACKAGE_RS<T_REG_RS>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_REG_RS);

    STRU_REG_RS() : STRU_PACKAGE_RS<T_REG_RS>(NEW_DEF_REG_RS)
    {

    }
};

#endif //__SYSPACKETDEF_H_
