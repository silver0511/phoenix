#ifndef __SERVER_TO_KAFKA_H
#define __SERVER_TO_KAFKA_H
#include "common/Debug_log.h"
#include "network/packet/ServerPack.h"
#include "kafka/KafkaMgr.h"

class CServerToKafka
{

private:
    CServerToKafka();
    CServerToKafka(const CServerToKafka &);
    CServerToKafka & operator = (const CServerToKafka &);
    
public:
    ~CServerToKafka();
public:
    static CServerToKafka & GetInstance()
    {
        static CServerToKafka instance;
        return instance;
    }
public:
    void InitGcRouteTopic();
    void InitScRouteTopic();
	void InitOffcialRouteTopic();
	void InitScApnsTopic();
	void InitGcApnsTopic();
    void releaseKafka();

public:
    template <class T>
    bool DistributeMsgToScRoute(const WORD &package_type, const USERID& user_id, T &send_struct, std::string strTopic)
    {
        BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
        WORD len_kafka = ServerPack::GetInstance().GetPackScKafkaByUserId(package_type, user_id, send_struct, lpBuffer);
        if(len_kafka <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " kafka pack failed");
            return false;
        }

        mpKafkaMgr->ProducerByUserId(strTopic, user_id, lpBuffer, len_kafka);
        return true;
    }

	template <class T>
    bool DistributeMsgToScRoute(const WORD &package_type, const USERID& user_id, T &send_struct, 
					std::string strTopic, string& strKey)
    {
        BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
        WORD len_kafka = ServerPack::GetInstance().GetPackScKafkaByUserId(package_type, user_id, send_struct, lpBuffer);
        if(len_kafka <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " kafka pack failed");
            return false;
        }

		mpKafkaMgr->ProducerByUserId(strTopic, user_id, lpBuffer, len_kafka, strKey);
        return true;
    }
	
	bool DistributeGcMsgToApns(const uint16& uimsgtype, const USERID* pUserList,const uint16& uiusercnt,BYTE* pmsg, WORD len, std::string strTopic)
	{
		BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
		BYTE* pos = lpBuffer;
		INT  idstlen = 0;

		//generate usercnt
		memcpy(pos,(BYTE*)(&uiusercnt),sizeof(uiusercnt));
		pos += sizeof(uiusercnt);
		idstlen += sizeof(uiusercnt);

		//generate usercid
		memcpy(pos, pUserList, uiusercnt * sizeof(USERID));
		pos += sizeof(USERID)*uiusercnt;
		idstlen += sizeof(USERID)*uiusercnt;

		//generate type
		memcpy(pos,(BYTE*)(&uimsgtype),sizeof(uimsgtype));
		pos += sizeof(uimsgtype);
		idstlen += sizeof(uimsgtype);
		 
		//generate orgmsg
		memcpy(pos,pmsg,len);
		idstlen += len;
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " length = " << len);
        mpKafkaMgr->ProducerByUserId(strTopic, pUserList[0], lpBuffer, idstlen);

		return true;
	}

	bool DistributeScMsgToApns(const uint16& uimsgtype, const USERID& user_id, BYTE* pmsg, WORD len, std::string strTopic)
	{
		if (NULL == pmsg)
		{
			return false;
		}
		
		BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
		BYTE* pos = lpBuffer;
		INT  idstlen = 0;	

		//generate usercnt
		uint16 uiusercnt = 1;
		memcpy(pos,(BYTE*)(&uiusercnt),sizeof(uiusercnt));
		pos += sizeof(uiusercnt);
		idstlen += sizeof(uiusercnt);

		//generate userid
		memcpy(pos,(BYTE*)(&user_id),sizeof(user_id));
		pos += sizeof(user_id);
		idstlen += sizeof(user_id);

		//generate type
		memcpy(pos,(BYTE*)(&uimsgtype),sizeof(uimsgtype));
		pos += sizeof(uimsgtype);
		idstlen += sizeof(uimsgtype);

		//generate orgmsg
		memcpy(pos,pmsg,len);
		idstlen += len;

		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " length = " << len);
		mpKafkaMgr->ProducerByUserId(strTopic, user_id, lpBuffer, idstlen);
		return true;
	}

    template <class T>
    bool DistributeMsgToGcRoute(const WORD &user_count, const USERID *user_id_list, 
							T &send_struct, std::string strTopic, UINT64 seperate_number, string strKey)
    {
        BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
        WORD len_kafka = ServerPack::GetInstance().GetPackKafka(send_struct.GetPackType(), user_count, user_id_list, send_struct, lpBuffer);
        if(len_kafka <= 0)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " kafka pack failed");
            return false;
        }

        mpKafkaMgr->ProducerByUserId(strTopic, seperate_number, lpBuffer, len_kafka, strKey);

        return true;
    }

	template <class T>
	bool DistributeMsgToPcRoute(const WORD &user_count, const USERID *user_id_list, T &send_struct, std::string strTopic, TEXTMSGID group_id)
	{
		BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
		WORD len_kafka = ServerPack::GetInstance().GetPackKafka(send_struct.GetPackType(), user_count, user_id_list, send_struct, lpBuffer);
		if(len_kafka <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " kafka pack failed");
			return false;
		}

		mpKafkaMgr->ProducerByUserId(strTopic, group_id, lpBuffer, len_kafka);

		return true;
	}

    template <class T>
    bool DistributeMessage(const std::vector<PHONEID>& vector_user_id, T &send_struct, std::string strTopic)
    {
        return true;
    }

public:
    CKafkaMgr* mpKafkaMgr;

};
#endif
