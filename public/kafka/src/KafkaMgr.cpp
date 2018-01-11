#include "kafka/KafkaMgr.h"

CKafkaMgr::CKafkaMgr()
{
    if(-1 == moKafkaConfig.Load(DEF_KAFKA_INI))
    {
        LOG_TRACE(LOG_CRIT,false,__FUNCTION__," kafka.Load Fail");
    }
}

CKafkaMgr::~CKafkaMgr()
{
}

void CKafkaMgr::InitGcRouteTopic()
{
    InitTopic(IM_GC_TOPIC);
}

void CKafkaMgr::InitScRouteTopic()
{
    InitTopic(IM_SC_TOPIC);
}

void CKafkaMgr::InitOffcialRouteTopic()
{
	InitTopic(IM_OFFCIALR_TOPIC);
}

void CKafkaMgr::InitUserInfoTopic()
{
    InitTopic(USER_TOPIC);
}

void CKafkaMgr::InitScApnsTopic()
{
    InitTopic(IM_APNS_TOPIC);
}

void CKafkaMgr::InitGcApnsTopic()
{
    InitTopic(IM_APNS_TOPIC);
}

void CKafkaMgr::InitTopic(std::string strTopic)
{
    // 主要用来数据测试
    CKafkaProducer* mpKafaInfo = new CKafkaProducer(moKafkaConfig.mStrBrokerList,
                                        moKafkaConfig.mapKafkaName[strTopic], 
                                        moKafkaConfig.mapKafkaPartitionNumber[strTopic]);
	bool bSuccess = mpKafaInfo->CreateProducer();
    if(!bSuccess)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "create producer failed");
	}
    else
    {
        mpMapTopicToProducer[strTopic] = mpKafaInfo;
    }
}

WORD CKafkaMgr::GetProducerTopicPartitions(std::string strTopic)
{
    CKafkaProducer* mpKafaInfo = mpMapTopicToProducer[strTopic];
    if(!mpKafaInfo)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "topic = " << strTopic << " not created");
        return 0;
    }

    return mpKafaInfo->mwPartition;
}

bool CKafkaMgr::ProducerByUserId(std::string strTopic, const USERID& user_id, void* strContent, WORD mwLen)
{
    CKafkaProducer* mpKafaInfo = mpMapTopicToProducer[strTopic];
    if(!mpKafaInfo)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "topic = " << strTopic << " not created");
        return false;
    }

    return mpKafaInfo->ProducerMessage(user_id, strContent, mwLen);
}

bool CKafkaMgr::ProducerByUserId(std::string strTopic, const UINT64& seperate_number, 
                                    void* strContent, WORD mwLen, string strKey)
{
    CKafkaProducer* mpKafaInfo = mpMapTopicToProducer[strTopic];
    if(!mpKafaInfo)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "topic = " << strTopic << " not created");
        return false;
    }

    return mpKafaInfo->ProducerMessage(seperate_number, strContent, mwLen, strKey);
}

void CKafkaMgr::ReleaseProducer()
{
    for (auto it = mpMapTopicToProducer.begin(); it != mpMapTopicToProducer.end(); ++it)
    {
        LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " partition = " << it->second->mwPartition);
        
        SAFE_DELETE(it->second);   
    }

    mpMapTopicToProducer.clear();   
}

CKafkaConfig::CKafkaConfig()
{
    
}

CKafkaConfig::~CKafkaConfig()
{

}

BOOL CKafkaConfig::Read()
{
	//PS顺序必须一致
    std::string strBorkerList = "";
	moIniFile.GetValue("broker_list", "COMMON", mStrBrokerList, "192.168.131.41:9092");
    moIniFile.GetValue("im_gc_topic", "TOPIC", mapKafkaName["im_gc_topic"], "im_gc_topic");
    moIniFile.GetValue("im_sc_topic", "TOPIC", mapKafkaName["im_sc_topic"], "im_sc_topic");
    moIniFile.GetValue("im_offcial_topic", "TOPIC", mapKafkaName["im_offcial_topic"], "im_offcial_topic");
    moIniFile.GetValue("im_apns_topic", "TOPIC", mapKafkaName["im_apns_topic"], "im_apns_topic");
    
    moIniFile.GetValue("im_gc_topic", "PARTITION", mapKafkaPartitionNumber["im_gc_topic"], 3);
     moIniFile.GetValue("im_offcial_topic", "PARTITION", mapKafkaPartitionNumber["im_offcial_topic"], 3);
    moIniFile.GetValue("im_sc_topic", "PARTITION", mapKafkaPartitionNumber["im_sc_topic"], 3);
    moIniFile.GetValue("im_apns_topic", "PARTITION", mapKafkaPartitionNumber["im_apns_topic"], 3);
   

    for(auto begin = mapKafkaName.begin(); begin != mapKafkaName.end(); begin++)
    {
        LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "key = " << begin->first << " name = " << begin->second);
    }

    for(auto begin = mapKafkaPartitionNumber.begin(); begin != mapKafkaPartitionNumber.end(); begin++)
    {
        LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "key = " << begin->first << " name = " << begin->second);
    }

    LOG_TRACE(LOG_ALERT, false, __FUNCTION__, " mStrBrokerList = " << mStrBrokerList);
	return true;
}

BOOL CKafkaConfig::DynamicRead()
{
	return TRUE;
}