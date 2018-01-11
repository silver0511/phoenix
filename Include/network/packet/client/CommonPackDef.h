#ifndef __COMMON_PACK_DEF_H
#define __COMMON_PACK_DEF_H

#include "network/packet/PackTypeDef.h"
#include "network/packet/BasePackDef.h"
#include "Package/hfbs/common/fb_base_rs_generated.h"

using namespace commonpack;

struct STRU_BASE_RS
{
public:
	flatbuffers::Offset<T_BASE_RS> fbbuf;
	flatbuffers::FlatBufferBuilder fbbuilder;
public:
	MAPPING_PACKTYPE(NEW_DEF_SERVER_BASE_ERROR);

	STRU_BASE_RS()
	{
	};

	INT Serialize(ISerialize & aoSerialize)
	{
		fbbuilder.Finish(fbbuf);
		BYTE*	lpData = fbbuilder.GetBufferPointer();
		WORD	lwLen = (WORD)fbbuilder.GetSize();
		WORD    liBufferLen = (WORD)fbbuilder.GetSize();
		return aoSerialize.Serialize(lpData, lwLen, liBufferLen);
	}
};

template<class T>
struct STRU_PACKAGE_RQ
{
public:
	T *t_fb_data;

	STRU_PACKAGE_RQ()
	{
		t_fb_data = NULL;
	};
	
	INT Check()
	{
		if(!t_fb_data)
		{
			return -1;
		}

		if(!t_fb_data->s_rq_head())
		{
			return -1;
		}
		if(t_fb_data->s_rq_head()->user_id() <= 0)
		{
			return -1;
		}

		return 1;
	}

	USERID GetUserID() const
	{
		if(!t_fb_data)
		{
			return -1;
		}

		if(!t_fb_data->s_rq_head())
		{
			return -2;
		}

		return t_fb_data->s_rq_head()->user_id();
	}

	USERID GetPackSessionID() const
	{
		if(!t_fb_data)
		{
			return -1;
		}

		if(!t_fb_data->s_rq_head())
		{
			return -2;
		}

		return t_fb_data->s_rq_head()->pack_session_id();
	}
};

template<typename T>
struct STRU_PACKAGE_RS
{
public:
	flatbuffers::Offset<T> fbbuf;
	flatbuffers::FlatBufferBuilder fbbuilder;

public:
	STRU_PACKAGE_RS(WORD packType)
	{
		m_wPackType = packType;
	} 

	STRU_PACKAGE_RS()
	{
		
	}

	INT Serialize(ISerialize & aoSerialize)
	{
		fbbuilder.Finish(fbbuf);
		BYTE*	lpData = fbbuilder.GetBufferPointer();
		WORD	lwLen = (WORD)fbbuilder.GetSize();
		WORD    liBufferLen = (WORD)fbbuilder.GetSize();
		return aoSerialize.Serialize(lpData, lwLen, liBufferLen);
	}

	inline WORD GetPackType()
	{
		return m_wPackType;
	}
private:
	WORD m_wPackType;
};

template<typename T>
using STRU_SERVER_PACKAGE_RQ = STRU_PACKAGE_RS<T>;

template<class T>
struct STRU_SERVER_PACKAGE_RS
{
public:
	T *t_fb_data;

	STRU_SERVER_PACKAGE_RS()
	{
		t_fb_data = NULL;
	};
	
	INT Check()
	{
		if(!t_fb_data)
		{
			return -1;
		}

		if(!t_fb_data->s_rs_head())
		{
			return -1;
		}
		if(t_fb_data->s_rs_head()->user_id() <= 0)
		{
			return -1;
		}

		return 1;
	}
};

#endif //__COMMON_PACK_DEF_H
