#ifndef __OFFCIALRSERVERPACKDEF_H
#define __OFFCIALRSERVERPACKDEF_H
#include "./CommonPackDef.h"

#include "Package/hfbs/offcial/fb_server_offcial_message_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_message_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_message_rs_generated.h"
#include "Package/hfbs/offcial/fb_client_fans_get_offline_msg_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_fans_get_offline_msg_rs_generated.h"
#include "Package/hfbs/offcial/fb_fans_offline_msg_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_get_offline_msg_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_get_offline_msg_rs_generated.h"
#include "Package/hfbs/offcial/fb_offcial_offline_msg_generated.h"
#include "Package/hfbs/offcial/fb_offcial_msg_generated.h"
#include "Package/hfbs/offcial/fb_server_fans_message_rq_generated.h"
#include "Package/hfbs/offcial/fb_server_fans_message_rs_generated.h"
#include "Package/hfbs/offcial/fb_client_fans_send_message_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_fans_send_message_rs_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_sys_msg_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_sys_msg_rs_generated.h"
#include "Package/hfbs/offcial/fb_client_fans_get_sys_msg_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_fans_get_sys_msg_rs_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_one_msg_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_one_msg_rs_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_some_msg_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_some_msg_rs_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_some_sys_msg_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_some_sys_msg_rs_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_one_sys_msg_rq_generated.h"
#include "Package/hfbs/offcial/fb_client_offcial_send_one_sys_msg_rs_generated.h"
#include "Package/hfbs/offcial/fb_server_offcial_private_message_rq_generated.h"
#include "Package/hfbs/offcial/fb_server_offcial_private_message_rs_generated.h"

using namespace offcialpack;

struct STRU_CLIENT_FANS_SEND_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CLIENT_FANS_SEND_MESSAGE_RQ>
{
public:

    MAPPING_PACKTYPE(NEW_DEF_CLIENT_FANS_SEND_MESSAGE_RQ);

    INT Serialize(ISerialize & aoSerialize)
    {
        flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
        bool is_fbs = VerifyT_CLIENT_FANS_SEND_MESSAGE_RQBuffer(verifier);
        if(!is_fbs)
        {
            return -2;
        }

        t_fb_data = (T_CLIENT_FANS_SEND_MESSAGE_RQ*)GetT_CLIENT_FANS_SEND_MESSAGE_RQ(aoSerialize.GetBuffer());
        if(Check() <= 0)
        {
            return -1;
        }

        return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
    }
    
    INT Check()
    {
        int result = STRU_PACKAGE_RQ<T_CLIENT_FANS_SEND_MESSAGE_RQ>::Check();
        if(result <= 0)
        {
            return result;
        }

        if(!t_fb_data->s_msg())
        {
            return -1;
        }

        if(!t_fb_data->s_msg()->msg_content())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->msg_content()->c_str())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content str is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->send_user_name())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name is nil");
            return -1;
        }

        if(!t_fb_data->s_msg()->send_user_name()->c_str())
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name str is nil");
            return -1;
        }

        if(!t_fb_data->offcial_id())
        {
            return -1;
        }

        if(!t_fb_data->message_id())
        {
            return -1;
        }

        return 1;
    }
};

struct STRU_CLIENT_FANS_SEND_MESSAGE_RS : public STRU_PACKAGE_RS<T_CLIENT_FANS_SEND_MESSAGE_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_FANS_SEND_MESSAGE_RS);

	STRU_CLIENT_FANS_SEND_MESSAGE_RS() : STRU_PACKAGE_RS<T_CLIENT_FANS_SEND_MESSAGE_RS>(NEW_DEF_CLIENT_FANS_SEND_MESSAGE_RS)
	{

	}
};


struct STRU_SERVER_FANS_MESSAGE_RQ : public STRU_PACKAGE_RS<T_SERVER_FANS_MESSAGE_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_SERVER_FANS_MESSAGE_RQ);

	STRU_SERVER_FANS_MESSAGE_RQ() : STRU_PACKAGE_RS<T_SERVER_FANS_MESSAGE_RQ>(NEW_DEF_SERVER_FANS_MESSAGE_RQ)
	{

	}
};

struct STRU_SERVER_FANS_MESSAGE_RS : public STRU_SERVER_PACKAGE_RS<T_SERVER_FANS_MESSAGE_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_SERVER_FANS_MESSAGE_RS);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_SERVER_FANS_MESSAGE_RSBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_SERVER_FANS_MESSAGE_RS*)GetT_SERVER_FANS_MESSAGE_RS(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{   
		int result = STRU_SERVER_PACKAGE_RS::Check();
		if(result < 0)
		{
			return -1;
		}

		if(!t_fb_data->s_msg())
		{
			return -1;
		}

		if(!t_fb_data->s_msg()->msg_content())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
			return -1;
		}

		if(!t_fb_data->s_msg()->msg_content()->c_str())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content str is nil");
			return -1;
		}

		if(!t_fb_data->s_msg()->send_user_name())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name is nil");
			return -1;
		}

		if(!t_fb_data->s_msg()->send_user_name()->c_str())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name str is nil");
			return -1;
		}

		if(!t_fb_data->fans_id())
		{
			return -1;
		}

		if(!t_fb_data->offcial_id())
		{
			return -1;
		}

		return 1;
	}
};

struct STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_MESSAGE_RQ>
{
public:

	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_MESSAGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CLIENT_OFFCIAL_SEND_MESSAGE_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CLIENT_OFFCIAL_SEND_MESSAGE_RQ*)GetT_CLIENT_OFFCIAL_SEND_MESSAGE_RQ(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{
		int result = STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_MESSAGE_RQ>::Check();
		if(result <= 0)
		{
			return result;
		}

		return 1;
	}
};

struct STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS : public STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_MESSAGE_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_MESSAGE_RS);

	STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS() : STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_MESSAGE_RS>(NEW_DEF_CLIENT_OFFCIAL_SEND_MESSAGE_RS)
	{

	}
};

struct STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RQ : public STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_SOME_MSG_RQ>
{
public:

	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_MESSAGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CLIENT_OFFCIAL_SEND_SOME_MSG_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CLIENT_OFFCIAL_SEND_SOME_MSG_RQ*)GetT_CLIENT_OFFCIAL_SEND_SOME_MSG_RQ(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{
		int result = STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_SOME_MSG_RQ>::Check();
		if(result <= 0)
		{
			return result;
		}

		return 1;
	}
};

struct STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RS : public STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_SOME_MSG_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_MESSAGE_RS);

	STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RS() : STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_SOME_MSG_RS>(NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_MESSAGE_RS)
	{

	}
};

struct STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RQ : public STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_ONE_MSG_RQ>
{
public:

	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_MESSAGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CLIENT_OFFCIAL_SEND_ONE_MSG_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CLIENT_OFFCIAL_SEND_ONE_MSG_RQ*)GetT_CLIENT_OFFCIAL_SEND_ONE_MSG_RQ(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{
		int result = STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_ONE_MSG_RQ>::Check();
		if(result <= 0)
		{
			return result;
		}

		return 1;
	}
};

struct STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RS : public STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_ONE_MSG_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_MESSAGE_RS);

	STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RS() : STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_ONE_MSG_RS>(NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_MESSAGE_RS)
	{

	}
};

struct STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_SYS_MSG_RQ>
{
public:

	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_SYS_MESSAGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CLIENT_OFFCIAL_SEND_SYS_MSG_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CLIENT_OFFCIAL_SEND_SYS_MSG_RQ*)GetT_CLIENT_OFFCIAL_SEND_SYS_MSG_RQ(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{
		int result = STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_SYS_MSG_RQ>::Check();
		if(result <= 0)
		{
			return result;
		}

		return 1;
	}
};

struct STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RS : public STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_SYS_MSG_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_SYS_MESSAGE_RS);

	STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RS() : STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_SYS_MSG_RS>(NEW_DEF_CLIENT_OFFCIAL_SEND_SYS_MESSAGE_RS)
	{

	}
};

struct STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_SOME_SYS_MSG_RQ>
{
public:

	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_SYS_MESSAGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CLIENT_OFFCIAL_SEND_SOME_SYS_MSG_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CLIENT_OFFCIAL_SEND_SOME_SYS_MSG_RQ*)GetT_CLIENT_OFFCIAL_SEND_SOME_SYS_MSG_RQ(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{
		int result = STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_SOME_SYS_MSG_RQ>::Check();
		if(result <= 0)
		{
			return result;
		}

		return 1;
	}
};

struct STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS : public STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_SOME_SYS_MSG_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_SYS_MESSAGE_RS);

	STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS() : STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_SOME_SYS_MSG_RS>(NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_SYS_MESSAGE_RS)
	{

	}
};

struct STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_ONE_SYS_MSG_RQ>
{
public:

	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_SYS_MESSAGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CLIENT_OFFCIAL_SEND_ONE_SYS_MSG_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CLIENT_OFFCIAL_SEND_ONE_SYS_MSG_RQ*)GetT_CLIENT_OFFCIAL_SEND_ONE_SYS_MSG_RQ(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{
		int result = STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_SEND_ONE_SYS_MSG_RQ>::Check();
		if(result <= 0)
		{
			return result;
		}

		return 1;
	}
};

struct STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RS : public STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_ONE_SYS_MSG_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_SYS_MESSAGE_RS);

	STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RS() : STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_SEND_ONE_SYS_MSG_RS>(NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_SYS_MESSAGE_RS)
	{

	}
};

struct STRU_CLIENT_FANS_GET_SYS_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CLIENT_FANS_GET_SYS_MSG_RQ>
{
public:

	MAPPING_PACKTYPE(NEW_DEF_CLIENT_FANS_GET_SYS_MESSAGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CLIENT_FANS_GET_SYS_MSG_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CLIENT_FANS_GET_SYS_MSG_RQ*)GetT_CLIENT_FANS_GET_SYS_MSG_RQ(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}


	INT Check()
	{   
		int result = STRU_PACKAGE_RQ<T_CLIENT_FANS_GET_SYS_MSG_RQ>::Check();
		if(result < 0)
		{
			return -1;
		}

		return 1;
	}

};

struct STRU_CLIENT_FANS_GET_SYS_MESSAGE_RS : public STRU_PACKAGE_RS<T_CLIENT_FANS_GET_SYS_MSG_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_FANS_GET_SYS_MESSAGE_RS);

	STRU_CLIENT_FANS_GET_SYS_MESSAGE_RS() : STRU_PACKAGE_RS<T_CLIENT_FANS_GET_SYS_MSG_RS>(NEW_DEF_CLIENT_FANS_GET_SYS_MESSAGE_RS)
	{

	}
};


struct STRU_CLIENT_OFFCIALR_GET_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RQ>
{
public:

	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RQ*)GetT_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RQ(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{   
		int result = STRU_PACKAGE_RQ<T_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RQ>::Check();
		if(result < 0)
		{
			return -1;
		}

		return 1;
	}


};

struct STRU_CLIENT_OFFCIALR_GET_MESSAGE_RS : public STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RS);

	STRU_CLIENT_OFFCIALR_GET_MESSAGE_RS() : STRU_PACKAGE_RS<T_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RS>(NEW_DEF_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RS)
	{

	}
};


struct STRU_CLIENT_FANS_GET_MESSAGE_RQ : public STRU_PACKAGE_RQ<T_CLIENT_FANS_GET_OFFLINE_MESSAGE_RQ>
{
public:

	MAPPING_PACKTYPE(NEW_DEF_CLIENT_FANS_GET_OFFLINE_MESSAGE_RQ);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_CLIENT_FANS_GET_OFFLINE_MESSAGE_RQBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_CLIENT_FANS_GET_OFFLINE_MESSAGE_RQ*)GetT_CLIENT_FANS_GET_OFFLINE_MESSAGE_RQ(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{   
		int result = STRU_PACKAGE_RQ<T_CLIENT_FANS_GET_OFFLINE_MESSAGE_RQ>::Check();
		if(result < 0)
		{
			return -1;
		}

		if(!t_fb_data->list_offcial_offline_msg_request())
		{
			return -1;
		}

		return 1;
	}

};

struct STRU_CLIENT_FANS_GET_MESSAGE_RS : public STRU_PACKAGE_RS<T_CLIENT_FANS_GET_OFFLINE_MESSAGE_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_CLIENT_FANS_GET_OFFLINE_MESSAGE_RS);

	STRU_CLIENT_FANS_GET_MESSAGE_RS() : STRU_PACKAGE_RS<T_CLIENT_FANS_GET_OFFLINE_MESSAGE_RS>(NEW_DEF_CLIENT_FANS_GET_OFFLINE_MESSAGE_RS)
	{

	}
};

struct STRU_SERVER_OFFCIALR_MESSAGE_RQ : public STRU_PACKAGE_RS<T_SERVER_OFFCIAL_MESSAGE_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_SERVER_OFFCIAL_MESSAGE_ID);

	STRU_SERVER_OFFCIALR_MESSAGE_RQ() : STRU_PACKAGE_RS<T_SERVER_OFFCIAL_MESSAGE_RQ>(NEW_DEF_SERVER_OFFCIAL_MESSAGE_ID)
	{

	}
};



struct STRU_SERVER_OFFCIALR_PRIVATE_MESSAGE_RQ : public STRU_PACKAGE_RS<T_SERVER_OFFCIAL_PRIVATE_MESSAGE_RQ>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_SERVER_OFFCIAL_PRIVATE_MESSAGE_RQ);

	STRU_SERVER_OFFCIALR_PRIVATE_MESSAGE_RQ() : STRU_PACKAGE_RS<T_SERVER_OFFCIAL_PRIVATE_MESSAGE_RQ>(NEW_DEF_SERVER_OFFCIAL_PRIVATE_MESSAGE_RQ)
	{

	}
};

struct STRU_SERVER_OFFCIALR_PRIVATE_MESSAGE_RS : public STRU_SERVER_PACKAGE_RS<T_SERVER_OFFCIAL_PRIVATE_MESSAGE_RS>
{
public:
	MAPPING_PACKTYPE(NEW_DEF_SERVER_OFFCIAL_PRIVATE_MESSAGE_RS);

	INT Serialize(ISerialize & aoSerialize)
	{
		flatbuffers::Verifier verifier(aoSerialize.GetBuffer(), aoSerialize.getBufferLen() - aoSerialize.getDataLen());
		bool is_fbs = VerifyT_SERVER_OFFCIAL_PRIVATE_MESSAGE_RSBuffer(verifier);
		if(!is_fbs)
		{
			return -2;
		}

		t_fb_data = (T_SERVER_OFFCIAL_PRIVATE_MESSAGE_RS*)GetT_SERVER_OFFCIAL_PRIVATE_MESSAGE_RS(aoSerialize.GetBuffer());
		if(Check() <= 0)
		{
			return -1;
		}

		return aoSerialize.getBufferLen() - aoSerialize.getDataLen();
	}

	INT Check()
	{   
		int result = STRU_SERVER_PACKAGE_RS<T_SERVER_OFFCIAL_PRIVATE_MESSAGE_RS>::Check();
		if(result < 0)
		{
			return -1;
		}

		if(!t_fb_data->s_msg())
		{
			return -1;
		}

		if(!t_fb_data->s_msg()->msg_content())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
			return -1;
		}

		if(!t_fb_data->s_msg()->msg_content()->c_str())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content str is nil");
			return -1;
		}

		if(!t_fb_data->s_msg()->send_user_name())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name is nil");
			return -1;
		}

		if(!t_fb_data->s_msg()->send_user_name()->c_str())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send_user_name str is nil");
			return -1;
		}

		if(!t_fb_data->offcial_id())
		{
			return -1;
		}

		if(!t_fb_data->message_id())
		{
			return -1;
		}

		return 1;
	}
};




#endif //__OFFCIALRSERVERPACKDEF_H
