#ifndef __SVRBASESTRUCT_H
#define __SVRBASESTRUCT_H

#include "const/SvrConstDef.h"
#include "common/ISerialize.h"
#include "common/BaseConfig.h"
#include "BaseStruct.h"
#include "mysqldriver/DBI.h"
#include "common/basefun.h"

//////////////////////////////////////////////////////////////////////////
//服务器信息
struct STRU_PURE_SERVER_INFO_BASE
{
	SERVERID	mlServerID;		//服务器ID
	IPTYPE		mlServerIP;		//服务器IP
	WORD		mwServerPort;	//服务器端口
	WORD		mwServerType;	//Server Type
};
inline void SetPureServerInfo(STRU_PURE_SERVER_INFO_BASE &aoDest,
							  const STRU_PURE_SERVER_INFO_BASE & aoSrc)
{
	aoDest.mwServerType = aoSrc.mwServerType;
	aoDest.mlServerID = aoSrc.mlServerID;
	aoDest.mlServerIP = aoSrc.mlServerIP;
	aoDest.mwServerPort = aoSrc.mwServerPort;
}
inline LONG SerializePureServerInfo(STRU_PURE_SERVER_INFO_BASE &aoInfo,ISerialize& aoSerialize)
{
	LONG llOffset=aoSerialize.Serialize(aoInfo.mwServerType);
	llOffset+=aoSerialize.Serialize(aoInfo.mlServerID);
	llOffset+=aoSerialize.Serialize(aoInfo.mlServerIP);
	llOffset+=aoSerialize.Serialize(aoInfo.mwServerPort);
	return llOffset;
}
inline BOOL IsEqualPureServerInfo(const STRU_PURE_SERVER_INFO_BASE & aoObj1,
								  const STRU_PURE_SERVER_INFO_BASE & aoObj2)
{
	if( aoObj1.mwServerType == aoObj2.mwServerType &&
		aoObj1.mlServerID == aoObj2.mlServerID &&
		aoObj1.mlServerIP == aoObj2.mlServerIP &&
		aoObj1.mwServerPort == aoObj2.mwServerPort)
		return TRUE;
	return FALSE;
}
struct STRU_SERVER_INFO_BASE:public STRU_PURE_SERVER_INFO_BASE
{
	STRU_SERVER_INFO_BASE()
	{
		init();
	}
	inline void init()
	{
		mlServerID=0;
		mlServerIP=0;
		mwServerPort=0;
		mwServerType=0;
	}
	virtual LONG Serialize(ISerialize & aoSerialize)
	{
		return SerializePureServerInfo(*this,aoSerialize);
	}

	inline STRU_SERVER_INFO_BASE&  operator =(const STRU_PURE_SERVER_INFO_BASE &aoObj)
	{
		SetPureServerInfo(*this,aoObj);
		return *this;
	}

	inline STRU_SERVER_INFO_BASE&  operator =(const STRU_SERVER_INFO_BASE &aoObj)
	{
		if(&aoObj==this)
			return *this;
		return operator = ((const STRU_PURE_SERVER_INFO_BASE&) aoObj);
	}

	inline BOOL  operator == (const STRU_PURE_SERVER_INFO_BASE &aoObj)
	{
		return IsEqualPureServerInfo(*this,aoObj);
	}
	inline BOOL  operator ==(const STRU_SERVER_INFO_BASE &aoObj)
	{
		return operator ==((const STRU_PURE_SERVER_INFO_BASE &)aoObj);
	}

    inline void GenServerID()
	{
		mlServerID = mwServerPort;
		DWORD ip = mlServerIP;
		if(mlServerIP == inet_addr("0.0.0.0"))
		{
			ip = inet_addr(GetIntranetIP().c_str());
		}
		mlServerID  = (mlServerID  << 33) | (ip);
	}

//	inline SESSIONID GenSessionID(int8 platform)
//	{
//		SESSIONID session_id = platform;
//		session_id <<= 8;
//		session_id += mlServerID;
//		return session_id;
//	}
//
//    inline int8 SplitSessionID(SESSIONID session_id)
//    {
//        mlServerID = session_id & 0xFFFFFF00;
//        mlServerIP = session_id >> 32;
//        mwServerPort = (session_id & 0x0000FF00) >> 16;
//		int8 liPlatform = session_id & 0xFF;
//		return liPlatform;
//    }
};

struct STRU_PACK_DATA
{
	BYTE mpData[DEF_MAX_GENERAL_PACK_LEN];
	WORD mwDataLen;
	STRU_PACK_DATA()
	{
		 init();
	}
	inline void init()
	{
		mwDataLen = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
#endif //__BASESERVERSTRUCT_H_
