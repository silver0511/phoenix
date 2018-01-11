#ifndef __USERPACKPACKDEF_H
#define __USERPACKPACKDEF_H

#include "./CommonPackDef.h"
#include "network/packet/PackTypeDef.h"
#include "network/packet/BasePackDef.h"
#include "Package/hfbs/common/fb_base_rs_generated.h"
#include "Package/hfbs/user/fb_get_user_info_rq_generated.h"
#include "Package/hfbs/user/fb_get_user_info_rs_generated.h"
#include "Package/hfbs/user/fb_get_userlst_info_rq_generated.h"
#include "Package/hfbs/user/fb_get_userlst_info_rs_generated.h"
#include "Package/hfbs/user/fb_update_user_info_rq_generated.h"
#include "Package/hfbs/user/fb_update_user_info_rs_generated.h"
#include "Package/hfbs/user/fb_register_apns_rq_generated.h"
#include "Package/hfbs/user/fb_register_apns_rs_generated.h"
#include "Package/hfbs/user/fb_user_complaint_rq_generated.h"
#include "Package/hfbs/user/fb_user_complaint_rs_generated.h"
#include "Package/hfbs/user/fb_get_me_info_rq_generated.h"
#include "Package/hfbs/user/fb_get_me_info_rs_generated.h"
#include "Package/hfbs/user/fb_change_mail_rq_generated.h"
#include "Package/hfbs/user/fb_change_mail_rs_generated.h"
#include "Package/hfbs/user/fb_change_mobile_rq_generated.h"
#include "Package/hfbs/user/fb_change_mobile_rs_generated.h"

using namespace userpack;

//获取自己的信息
struct STRU_GET_ME_INFO_RQ : public STRU_PACKAGE_RQ<T_GET_ME_INFO_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_ME_INFO_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_GET_ME_INFO_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_GET_ME_INFO_RQ*)GetT_GET_ME_INFO_RQ(aoSerialize.GetBuffer());
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
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "base check failed result = " << result);
			return result;
		}

		return 1;
	}
};

struct STRU_GET_ME_INFO_RS : public STRU_PACKAGE_RS<T_GET_ME_INFO_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_ME_INFO_RS);

	STRU_GET_ME_INFO_RS() : STRU_PACKAGE_RS<T_GET_ME_INFO_RS>(NEW_DEF_ME_INFO_RS)
	{

	}
};
//获取用户信息
struct STRU_GET_USER_INFO_RQ : public STRU_PACKAGE_RQ<T_GET_USER_INFO_RQ>
{
public:
    MAPPING_PACKTYPE(NEW_DEF_USER_INFO_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_GET_USER_INFO_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_GET_USER_INFO_RQ*)GetT_GET_USER_INFO_RQ(aoSerialize.GetBuffer());
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
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "base check failed result = " << result);
			return result;
		}

		if(!t_fb_data->user_msg() || !t_fb_data->user_msg()->c_str() || t_fb_data->user_msg()->c_str() == "")
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "user_msg is nil");
			return -1;
		}

		return 1;
	}
};

struct STRU_GET_USER_INFO_RS : public STRU_PACKAGE_RS<T_GET_USER_INFO_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_USER_INFO_RS);

	STRU_GET_USER_INFO_RS() : STRU_PACKAGE_RS<T_GET_USER_INFO_RS>(NEW_DEF_USER_INFO_RS)
	{
		
	}
};

//获取多个用户信息
struct STRU_GET_USERLST_INFO_RQ : public STRU_PACKAGE_RQ<T_GET_USERLST_INFO_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_USERLST_INFO_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_GET_USERLST_INFO_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_GET_USERLST_INFO_RQ*)GetT_GET_USERLST_INFO_RQ(aoSerialize.GetBuffer());
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
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "base check failed result = " << result);
			return result;
		}

		if(NULL == t_fb_data->userLst() && NULL == t_fb_data->mobileLst())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "userLst and mobileLs is nil");
			return -1;
		}

		if(NULL != t_fb_data->userLst() && NULL != t_fb_data->mobileLst())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "userLst or mobileLst is not nil");
			return -1;
		}

		return 1;
	}

};

struct STRU_GET_USERLST_INFO_RS : public STRU_PACKAGE_RS<T_GET_USERLST_INFO_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_USERLST_INFO_RS);

	STRU_GET_USERLST_INFO_RS() : STRU_PACKAGE_RS<T_GET_USERLST_INFO_RS>(NEW_DEF_USERLST_INFO_RS)
	{

	}
};

//修改用户信息
struct STRU_UPDATE_USER_INFO_RQ : public STRU_PACKAGE_RQ<T_UPDATE_USER_INFO_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_USER_CHANGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_UPDATE_USER_INFO_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}
		t_fb_data = (T_UPDATE_USER_INFO_RQ*)GetT_UPDATE_USER_INFO_RQ(aoSerialize.GetBuffer());
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
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "base check failed result = " << result);
			return result;
		}
		if(t_fb_data == NULL || t_fb_data->key_lst_info() == NULL || t_fb_data->key_lst_info()->size() <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "s_msg is nil");
			return -1;
		}
		return 1;
	}
};

struct STRU_T_UPDATE_USER_INFO_RS : public STRU_PACKAGE_RS<T_UPDATE_USER_INFO_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_USER_CHANGE_RS);

	STRU_T_UPDATE_USER_INFO_RS() : STRU_PACKAGE_RS<T_UPDATE_USER_INFO_RS>(NEW_DEF_USER_CHANGE_RS)
	{

	}
};


//添加device_token[IOS用]
struct STRU_REGISTER_APNS_RQ : public STRU_PACKAGE_RQ<T_REGISTER_APNS_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_REGISTER_APNS_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_REGISTER_APNS_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_REGISTER_APNS_RQ*)GetT_REGISTER_APNS_RQ(aoSerialize.GetBuffer());
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
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "base check failed result = " << result);
			return result;
		}

		if(!t_fb_data->device_token())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "device token is nil");
			return -1;
		}
		return 1;
	}
};

struct STRU_REGISTER_APNS_RS : public STRU_PACKAGE_RS<T_REGISTER_APNS_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_REGISTER_APNS_RS);

	STRU_REGISTER_APNS_RS() : STRU_PACKAGE_RS<T_REGISTER_APNS_RS>(NEW_DEF_REGISTER_APNS_RS)
	{

	}
};

//用户举报信息包
struct STRU_USER_COMPLAINT_RQ : public STRU_PACKAGE_RQ<T_USER_COMPLAINT_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_USER_COMPLAINT_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_USER_COMPLAINT_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_USER_COMPLAINT_RQ*)GetT_USER_COMPLAINT_RQ(aoSerialize.GetBuffer());
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
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "base check failed result = " << result);
			return result;
		}

		if(!t_fb_data->user_id() && !t_fb_data->type() )
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "user_id is nil");
			return -1;
		}

		return 1;
	}
};

struct STRU_USER_COMPLAINT_RS : public STRU_PACKAGE_RS<T_USER_COMPLAINT_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_USER_COMPLAINT_RS);

	STRU_USER_COMPLAINT_RS() : STRU_PACKAGE_RS<T_USER_COMPLAINT_RS>(NEW_DEF_USER_COMPLAINT_RS)
	{

	}
};

//修改手机号码包
struct STRU_CHANGE_MOBILE_RQ : public STRU_PACKAGE_RQ<T_CHANGE_MOBILE_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CHANGE_MOBILE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CHANGE_MOBILE_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CHANGE_MOBILE_RQ*)GetT_CHANGE_MOBILE_RQ(aoSerialize.GetBuffer());
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
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "base check failed result = " << result);
			return result;
		}

		if(!t_fb_data->new_moblie())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "user_id is nil");
			return -1;
		}

		return 1;
	}
};

struct STRU_CHANGE_MOBILE_RS : public STRU_PACKAGE_RS<T_CHANGE_MOBILE_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CHANGE_MOBILE_RS);

	STRU_CHANGE_MOBILE_RS() : STRU_PACKAGE_RS<T_CHANGE_MOBILE_RS>(NEW_DEF_CHANGE_MOBILE_RS)
	{

	}
};

//修改邮箱包
struct STRU_CHANGE_MAIL_RQ : public STRU_PACKAGE_RQ<T_CHANGE_MAIL_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CHANGE_MAIL_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CHANGE_MAIL_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CHANGE_MAIL_RQ*)GetT_CHANGE_MAIL_RQ(aoSerialize.GetBuffer());
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
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "base check failed result = " << result);
			return result;
		}

		if(!t_fb_data->old_mail())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "old_mail is nil");
			return -1;
		}

		if(!t_fb_data->new_mail())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new_mail is nil");
			return -1;
		}

		return 1;
	}
};

struct STRU_CHANGE_MAIL_RS : public STRU_PACKAGE_RS<T_CHANGE_MAIL_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CHANGE_MAIL_RS);

	STRU_CHANGE_MAIL_RS() : STRU_PACKAGE_RS<T_CHANGE_MAIL_RS>(NEW_DEF_CHANGE_MAIL_RS)
	{

	}
};

#endif //__USERPACKPACKDEF_H
