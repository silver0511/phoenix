#ifndef __BASEPACKETDEF_H_
#define __BASEPACKETDEF_H_

#include "common/MemorySerialize.h"
#include "const/constdef.h"
#include "common/Debug_log.h"
#include "common/U9Assert.h"
//////////////////////////////////////////////////////////////////////////
//base包
struct STRU_BASE_PACKAGE
{
public:
	STRU_BASE_PACKAGE()
	{
	}

	virtual ~STRU_BASE_PACKAGE(){};

	//打包函数 从数据包中将数据放入到缓冲区
	INT Pack(BYTE* apBuffer, LONG alBuffLen)
	{
		try{

			CMemorySerialize loSerialize(apBuffer,alBuffLen, ISerialize::STORE);
			INT result = Serialize(loSerialize);
			if(result < 0)
			{
				LOG_TRACE(LOG_CRIT,true, __FUNCTION__, "pack serialize failed")
				return -1;
			}
			return loSerialize.getDataLen();
		}
		catch(...){
			return -1;
		}	
	}

	//解包函数 从缓冲区中将数据放入数据包
	INT UnPack(BYTE* apBuffer, LONG alBuffLen)
	{
		try{
			CMemorySerialize loSerialize(apBuffer,alBuffLen, ISerialize::LOAD);
			INT result = Serialize(loSerialize);
			if(result < 0)
			{
				LOG_TRACE(LOG_CRIT,true, __FUNCTION__, "unpack serialize failed resule:" << result)
				return -1;
			}
			return result;
		}
		catch(...){
			LOG_TRACE(LOG_CRIT,true, __FUNCTION__, "STRU_BASE_PACKAGE CATCH")
			return -1;
		}
	}	
public:
	//序列化函数
	virtual INT Serialize(ISerialize & aoSerialize)=0;
public:
protected:
};

//////////////////////////////////////////////////////head start////////////////////////////////////////////////////////
//通用包头模型
struct STRU_GENERAL_HEAD_PACKAGE: public STRU_BASE_PACKAGE
{
public:
	WORD mwPackType;
public:
	STRU_GENERAL_HEAD_PACKAGE()
	{
		mwPackType = 0;
	}

	virtual INT Serialize(ISerialize & aoSerialize)
	{
		return aoSerialize.Serialize(mwPackType);
	}

	WORD GetHeadLen()
	{
		return DEF_TCP_PACKHEAD_LEN;
	}
};

//连接服务器<->后台服务器包头
struct STRU_COMET_2_BACK_HEAD : public STRU_GENERAL_HEAD_PACKAGE
{
public:
	USERID	 	miUserID;
	SESSIONID  miCometID;		//服务器会话ID
public:
    STRU_COMET_2_BACK_HEAD():STRU_GENERAL_HEAD_PACKAGE()
	{
		miUserID = 0;
		miCometID = 0;
	}

	virtual INT Serialize(ISerialize & aoSerialize)
	{
		INT len = 0;
		len += STRU_GENERAL_HEAD_PACKAGE::Serialize(aoSerialize);
		len += aoSerialize.Serialize(miUserID);
		len += aoSerialize.Serialize(miCometID);
		return len;
	}

	WORD GetHeadLen()
	{
		return DEF_COMET_2_BACK_HEAD_LEN;
	}
};

//逻辑服务器<->数据库服务器包头
struct STRU_LOGIC_2_DB_HEAD : public STRU_COMET_2_BACK_HEAD
{
public:
	SESSIONID  miLogicID;		//服务器会话ID
public:
	STRU_LOGIC_2_DB_HEAD():STRU_COMET_2_BACK_HEAD()
	{
		miLogicID = 0;
	}

	virtual INT Serialize(ISerialize & aoSerialize)
	{
		INT len = 0;
		len += STRU_COMET_2_BACK_HEAD::Serialize(aoSerialize);
		len += aoSerialize.Serialize(miLogicID);
		return len;
	}

	WORD GetHeadLen()
	{
		return DEF_LOGIC_2_DB_HEAD_LEN;
	}
};

//逻辑服务器->消费服务器服务器包头
struct STRU_LOGIC_2_CONSUME_HEAD : public STRU_GENERAL_HEAD_PACKAGE
{
public:
	enum
	{
		LIST_COUNT = 500,
	};

	WORD miUserCount;
	USERID moUserList[LIST_COUNT];
public:
	STRU_LOGIC_2_CONSUME_HEAD():STRU_GENERAL_HEAD_PACKAGE()
	{
		miUserCount = 0;
		ZeroMemory(moUserList, LIST_COUNT * sizeof(USERID));
	}

	virtual INT Serialize(ISerialize & aoSerialize)
	{
		INT len = STRU_GENERAL_HEAD_PACKAGE::Serialize(aoSerialize);
		len += aoSerialize.Serialize(miUserCount);
		if(miUserCount > LIST_COUNT)
		{
			throw(1);
		}

		for(int i = 0; i < miUserCount; ++i)
		{
			len += aoSerialize.Serialize(moUserList[i]);
		}

		return len;
	}

	WORD GetHeadLen()
	{
		return DEF_BACK_2_COMET_HEAD_MAX_LEN;
	}
};

//逻辑服务器-> 单聊消费服务器服务器包头
struct STRU_LOGIC_2_SC_CONSUMER_HEAD : public STRU_GENERAL_HEAD_PACKAGE
{
public:
	USERID user_id;
public:
	STRU_LOGIC_2_SC_CONSUMER_HEAD():STRU_GENERAL_HEAD_PACKAGE()
	{
		
	}

	virtual INT Serialize(ISerialize & aoSerialize)
	{
		INT len = STRU_GENERAL_HEAD_PACKAGE::Serialize(aoSerialize);
		len += aoSerialize.Serialize(user_id);
		return len;
	}

	WORD GetHeadLen()
	{
		return DEF_BACK_2_COMET_HEAD_MAX_LEN;
	}
};

//后台服务器->连接服务器服务器包头
struct STRU_BACK_2_COMET_HEAD_PACKAGE : public STRU_GENERAL_HEAD_PACKAGE
{
public:
	enum
	{
		LIST_COUNT = 500,
	};

	WORD miUserCount;
	USERID moUserList[LIST_COUNT];
public:
	STRU_BACK_2_COMET_HEAD_PACKAGE():STRU_GENERAL_HEAD_PACKAGE()
	{
		miUserCount = 0;
		ZeroMemory(moUserList, LIST_COUNT * sizeof(USERID));
	}

	virtual INT Serialize(ISerialize & aoSerialize)
	{
		INT len = STRU_GENERAL_HEAD_PACKAGE::Serialize(aoSerialize);
		len += aoSerialize.Serialize(miUserCount);
		if(miUserCount > LIST_COUNT)
		{
			throw(1);
		}

		for(int i = 0; i < miUserCount; ++i)
		{
			len += aoSerialize.Serialize(moUserList[i]);
		}

		return len;
	}

	WORD GetHeadLen()
	{
		return DEF_BACK_2_COMET_HEAD_MAX_LEN;
	}
};
//////////////////////////////////////////////////////head end//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////body start////////////////////////////////////////////////////////
//通用包体模型
template<typename PARA>
struct STRU_GENERAL_BODY_PACKAGE : public STRU_BASE_PACKAGE
{
	PARA	moUserLoad;		//用户负载
	STRU_GENERAL_BODY_PACKAGE():STRU_BASE_PACKAGE(){}
	virtual INT Serialize(ISerialize & aoSerialize)
	{
		return moUserLoad.Serialize(aoSerialize);
	}

	STRU_GENERAL_BODY_PACKAGE&  operator =(const STRU_GENERAL_BODY_PACKAGE &aoObj)
	{
		U9_ASSERT(&aoObj!=this);
		if(&aoObj==this)
			return *this;
		moUserLoad=aoObj.moUserLoad;
		return *this;
	}
};

//通用包体模型
template<typename PARA>
struct STRU_GENERAL_BODY_PACKAGE_P : public STRU_BASE_PACKAGE
{
	PARA	*mpoUserLoad;		//用户负载
	STRU_GENERAL_BODY_PACKAGE_P():STRU_BASE_PACKAGE(){}
	virtual INT Serialize(ISerialize & aoSerialize)
	{
		return mpoUserLoad->Serialize(aoSerialize);
	}
};

//传输服务器通用包体
struct STRU_TRANS_BODY_PACKAGE
{
public:
	BYTE* 	mpBuffer;
	WORD 	mwLen;
public:

	STRU_TRANS_BODY_PACKAGE()
	{
	}

	virtual INT Serialize(ISerialize & aoSerialize)
	{
		return aoSerialize.Serialize(mpBuffer, mwLen, DEF_MAX_UDPPACK_LEN);
	}
};

//////////////////////////////////////////////////////body start////////////////////////////////////////////////////////
#endif //__BASEPACKETDEF_H_
