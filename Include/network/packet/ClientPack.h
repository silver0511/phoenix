#ifndef QBIM_CLIENTPACK_H
#define QBIM_CLIENTPACK_H

#include "base0/platform.h"
#include "common/CriticalSection.h"
#include "network/ITransProtocolMgr.h"
#include "network/packet/BasePackDef.h"

using namespace U9;

//function map
#ifdef WIN32
#define TCP_COMET_PACKET_FUN(classname) typedef INT (##classname:: *TCPNewRecvFunc)(WORD awPackageType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
#else
#define	TCP_COMET_PACKET_FUN(classname) typedef INT (classname:: *TCPNewRecvFunc)(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
#endif

#define TCP_COMET_MAP_INIT(classname) TCP_COMET_PACKET_FUN(classname) \
	TCPNewRecvFunc moNewRecvDealFunList[65536];\

#define TCP_PACKET_MAP_BEGIN() \
private:\
	void InitNetFunctions()\
{\
	ZeroMemory(&moNewRecvDealFunList,65536 * sizeof(TCPNewRecvFunc));\


//带关联指针的调用
#define TCP_PACKET_MAP(C,P,F) moNewRecvDealFunList[P::ENUM_TYPE]        = &C::F;

#define TCP_PACKET_MAP_END()	};\

#define TCP_PACKET_DISTRIBUTE(package_type, user_id, comet_id) \
	if (NULL == moNewRecvDealFunList[package_type])\
		return -1;\
	return (this->*moNewRecvDealFunList[package_type])(package_type, user_id, comet_id, apRelationPtr, apData, awLen);\



#define TCP_PACKET_INIT()	InitNetFunctions();


#define CHECK_INVALID() \
	if(NULL == apRelationPtr)\
	{\
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error apRelationPtr is NULL"<<" RecvSocket="<<ahSocket<<"PakcetType"<<*((WORD*)apData));\
		return 0;\
	}\
	CUser* lpUser = (CUser*)apRelationPtr;\
	if(lpUser->mhSocket != ahSocket || lpUser->miIPAddr !=  aiIP || lpUser->miIPPort != awPort)\
	{\
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error invalid user poniter."\
			<<" RelationPtr="<<apRelationPtr\
			<<" UserSocket="<<lpUser->mhSocket\
			<<" RecvSocket="<<ahSocket\
			<<" IP="<<GetIPString(lpUser->miIPAddr)\
			<<":"<<ntohs(lpUser->miIPPort)\
			<<" RcvIP="<<GetIPString(aiIP)\
			<<":"<<ntohs(awPort)\
			);\
		return 0;\
	}\

#define CLIENT_CHECK_LOGIN() \
	CUser* lpUser = (CUser*)apRelationPtr;\
	if(lpUser->miUserID <= 0)\
	{\
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error invalid login user."\
			<<" RelationPtr="<<apRelationPtr\
			<<" UserSocket="<<lpUser->mhSocket\
			<<" IP="<<GetIPString(lpUser->miIPAddr)\
			<<":"<<ntohs(lpUser->miIPPort)\
			<<"PakcetType"<<awPackType\
			);\
		ClientPack::GetInstance().DisConnectTo(ahSocket); \
		return 0;\
	}\

//通用包头
#define GENERAL_CLIENT_UNPACK_HEAD(packagetype, disconnect) \
    packagetype loHeadPackage; \
    INT liUnPackLen = loHeadPackage.UnPack(apData, loHeadPackage.GetHeadLen()); \
    if(liUnPackLen < 0) \
    { \
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error unpack fault1." \
        <<" IP="<<GetIPString(aiIP) \
        <<":"<<ntohs(awPort) \
        <<"PakcetType"<< loHeadPackage.mwPackType \
    	); \
        if(disconnect)\
        {\
    	    ClientPack::GetInstance().DisConnectTo(ahSocket); \
        }\
    	return 0; \
    } \
    apData += liUnPackLen; \
    awLen -= liUnPackLen; \

//客户端包头
#define CLIENT_UNPACK_HEAD() GENERAL_CLIENT_UNPACK_HEAD(STRU_GENERAL_HEAD_PACKAGE, TRUE)
//连接服务器->逻辑服务器包头
#define COMET_2_BACK_UNPACK_HEAD() GENERAL_CLIENT_UNPACK_HEAD(STRU_COMET_2_BACK_HEAD, FALSE)

#define GENERAL_UNPACK_BODY(packtype, disconnect) STRU_GENERAL_BODY_PACKAGE<packtype> loRecvPack;\
	CUser* lpUser = (CUser*)apRelationPtr;\
	if(0 > loRecvPack.UnPack(apData, awLen)) \
	{\
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error unpack fault2."\
			<<" RelationPtr="<<apRelationPtr\
			<<" UserSocket="<<lpUser->mhSocket\
			<<" IP="<<GetIPString(lpUser->miIPAddr)\
			<<":"<<ntohs(lpUser->miIPPort)\
			<<"PakcetType"<<awPackType\
			);\
        if(disconnect)\
        {\
            ClientPack::GetInstance().DisConnectTo(lpUser->mhSocket);\
        }\
		return 0;\
	}\
	const packtype &loUserLoad = loRecvPack.moUserLoad;\


#define TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(packtype, aiUserID, RS_PACK_TYPE) STRU_GENERAL_BODY_PACKAGE<packtype> loRecvPack;\
	CUser* lpUser = (CUser*)apRelationPtr;\
	const packtype &loUserLoad = loRecvPack.moUserLoad;\
	INT nResult = loRecvPack.UnPack(apData, awLen);\
	if(-2 == nResult) \
	{\
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error unpack fault3."\
			<<" RelationPtr="<<apRelationPtr\
			<<" IP="<<GetIPString(lpUser->miIPAddr)\
			<<":"<<ntohs(lpUser->miIPPort)\
			<<"PakcetType"<<awPackType\
			);\
		TEMPLATE_GENERAL_FAIL_RESULT(RS_PACK_TYPE, MAKE_ERROR_RESULT(RET_UNPACK_FAILED_RESULT));\
		return 0;\
	}\
	if(0 >= nResult || aiUserID != loUserLoad.GetUserID())\
	{\
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error user_id is invalid user_id = "\
			<< aiUserID << " new user_id = " << loUserLoad.GetUserID());\
		TEMPLATE_GENERAL_FAIL_RESULT(RS_PACK_TYPE, MAKE_ERROR_RESULT(RET_UNPACK_FAILED_RESULT));\
		return 0;\	
	}\

#define TEMPLATE_GENERAL_FAIL_RESULT(RS_PACK_TYPE, result) STRU_BASE_RS loSendFailedPack;\
	commonpack::S_RS_HEAD s_rs_head_result(aiUserID, loUserLoad.GetPackSessionID(), result, 0);\
	loSendFailedPack.fbbuf = commonpack::CreateT_BASE_RS(loSendFailedPack.fbbuilder, &s_rs_head_result);\
	Send2Comet(RS_PACK_TYPE, loSendFailedPack, aiUserID, lpUser);\

#define TEMPLATE_GENERAL_DB_FAIL_RESULT(RS_PACK_TYPE, result, pack_session_id, platform) STRU_BASE_RS loSendFailedPack;\
	commonpack::S_RS_HEAD s_rs_head_result(aiUserID, pack_session_id, result, platform);\
	loSendFailedPack.fbbuf = commonpack::CreateT_BASE_RS(loSendFailedPack.fbbuilder, &s_rs_head_result);\
	Send2Comet(RS_PACK_TYPE, loSendFailedPack, aiUserID, lpUser);\

#define GENERAL_PACK_SAME_BEGIN() int liResult = MAKE_SUCCESS_RESULT();\
	auto t_fb_data = loUserLoad.t_fb_data;\
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "t_fb_data info rq "\
			<<" package_session_id="<< t_fb_data->s_rq_head()->pack_session_id()\
			<<" user_id=" << t_fb_data->s_rq_head()->user_id()\
			<< " platform = " << t_fb_data->s_rq_head()->platform());\
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),\
									t_fb_data->s_rq_head()->pack_session_id(),\
									liResult, \
									t_fb_data->s_rq_head()->platform());\


#define GENERATE_DBSERVER_TO_LOGIC_CMD(pack_type) string str_val;\
	CUser* lpUser = mpServerMgr->moUserConMgr.getUser(aiCometID, aiUserID);\
	if(!lpUser)\
	{\
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << aiUserID);\
		return 1;\
	}\
	RJsonValue &jsondata = JsonParse::get(jsoncmd, CDBToLogicCommon::GetData());\
	int liResult = jsoncmd["responseCode"].GetInt();\
	if(liResult != MAKE_SUCCESS_RESULT())\
	{\
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "liResult = " << liResult);\
		TEMPLATE_GENERAL_DB_FAIL_RESULT(pack_type,\
				MAKE_ERROR_RESULT(liResult), aiPackSessionID, aiPlatform);\
		return 0;\
	}\
	commonpack::S_RS_HEAD s_rs_head(aiUserID, aiPackSessionID, liResult, aiPlatform);\

#define GENERATE_LOGIC_TO_DBSERVER_CMD() RJ_CREATE_O_DOCUMENT(l_document, l_allocator);\
	JsonParse::add(l_document, CLogicToDBCommon::GetUserID(), std::to_string(aiUserID), l_allocator);\

#define GENERATE_LOGIC_TO_DBSERVER_END(TaskClass) string szmsg;\
	JsonParse::to_string(l_document, szmsg);\
	TaskClass *task_data = new TaskClass(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());\
    if (NULL == task_data)\
    {\
        LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "new task_data failed!");\
        return -1;\
    }\
    std::string szdata = szmsg;\
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "send to db msg:" << szdata);\
    task_data->SetData(szdata);\
    task_data->SetClientSide(this);\
    m_taskthreadpoll.AddTask(task_data);\

class ClientPack
{
private:
    ClientPack();
    ClientPack(const ClientPack &);
    ClientPack & operator = (const ClientPack &);
public:
    ~ClientPack();
    static ClientPack & GetInstance()
    {
        static ClientPack instance;
        return instance;
    }
public:
	bool Open(ITransMgrCallBack *interface, STRU_OPTION_TCP &option_tcp, ENUM_CONNECT_MODE connect_mode);
    bool AddRelationPtr(void*apSocketInfo,void* apRelationPtr);
    bool DelRelationPtr(void*apSocketInfo,void* apRelationPtr);
	INT DisplayStatisticsInfo();
	void Close();

	INT PackHead(WORD package_type, BYTE *buffer);

	INT PackBack2CometHead(WORD package_type, const WORD &user_count, const USERID *user_id_list, BYTE *buffer);

	template <class T>
	INT PackBody(T &send_struct, BYTE *buffer)
	{
		STRU_GENERAL_BODY_PACKAGE_P<T> body_package;
		body_package.mpoUserLoad = &send_struct;
		return body_package.Pack(buffer, DEF_MAX_TCP_PACKET_LEN);
	}

	template <class T>
	INT PackHeadBody(WORD package_type, T &send_struct, BYTE *buffer)
	{
		INT head_len = PackHead(package_type, buffer);
		buffer += head_len;
		INT body_len = PackBody(send_struct, buffer);

		return head_len + body_len;
	}

	template <class T>
	INT PackBack2Comet(WORD package_type, const WORD &user_count, const USERID *user_id_list, T &send_struct, BYTE *buffer)
	{
		INT head_len = PackBack2CometHead(package_type, user_count, user_id_list, buffer);
		buffer += head_len;
		INT body_len = PackBody(send_struct, buffer);

		return head_len + body_len;
	}

    template<class T>
    INT Send2Client(WORD package_type, T &send_struct, void* p_socket_info,
				 CCriticalSection &ref_critical_section, BOOL dis_connect = FALSE)
	{
		if(NULL == p_tcp_mgr ||
		   NULL == p_socket_info)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " Error SocketInfoPtr is NULL pack type:"<< package_type);
			return -1;
		}

		BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
		INT pack_len  = PackHeadBody(package_type, send_struct, lpBuffer);
		LOG_TRACE(LOG_DEBUG,TRUE, __FUNCTION__, "pack_len: " << pack_len);

		if(pack_len <= 0)
		{
			LOG_TRACE(LOG_ERR, TRUE, __FUNCTION__, "invalid pack_len: " << pack_len
																  << "pack_type " << package_type);
			return -1;
		}

		STRU_SENDDATA_TCP send_data;
		send_data.mhSocket = 0;
		send_data.mpData = lpBuffer;
		send_data.mwLen = pack_len;
		send_data.mbNeedClose = dis_connect;

		CRITICAL_SECTION_HELPER(ref_critical_section);
		return p_tcp_mgr->SendData(p_socket_info, &send_data);
	}

    template<class T>
    INT Send2Comet(WORD package_type, const WORD &user_count, const USERID *user_id_list, const SOCKET &ahSocket,
				   T &send_struct,  BOOL dis_connect = FALSE)
    {
        if(NULL == p_tcp_mgr)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " Error p_tcp_mgr is NULL pack type:"<< package_type);
            return -1;
        }

        BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
        INT pack_len  = PackBack2Comet(package_type, user_count, user_id_list, send_struct, lpBuffer);
        LOG_TRACE(LOG_DEBUG,TRUE, __FUNCTION__, "pack_len: " << pack_len);

        if(pack_len <= 0)
        {
            LOG_TRACE(LOG_ERR, TRUE, __FUNCTION__, "invalid pack_len: " << pack_len
                                                                  << "pack_type " << package_type);
            return -1;
        }

        STRU_SENDDATA_TCP send_data;
        send_data.mhSocket = ahSocket;
        send_data.mpData = lpBuffer;
        send_data.mwLen = pack_len;
        send_data.mbNeedClose = dis_connect;

        return p_tcp_mgr->SendData(&send_data);
    }

	INT Send2Comet(const SOCKET &ahSocket, BYTE* lpBuffer, INT pack_len, BOOL dis_connect = FALSE)
	{
		STRU_SENDDATA_TCP send_data;
        send_data.mhSocket = ahSocket;
        send_data.mpData = lpBuffer;
        send_data.mwLen = pack_len;
        send_data.mbNeedClose = dis_connect;

		return p_tcp_mgr->SendData(&send_data);
	}

    INT DisConnectTo(const SOCKET &socket);
private:
	ITransProtocolMgr *p_tcp_mgr;
};


#endif //QBIM_CLIENTPACK_H
