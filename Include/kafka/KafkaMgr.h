#ifndef __KAFKAMGR_H
#define __KAFKAMGR_H

#include <map>
#include "common/Debug_log.h"
#include "const/KafkaDef.h"
#include "kafka/KafkaProducer.h"
#include "common/BaseConfig.h"

#define	 DEF_KAFKA_INI					"kafka.ini"

class CKafkaConfig:public CBaseConfig
{
public:
	CKafkaConfig();
	~CKafkaConfig();

	virtual BOOL Read();
	virtual BOOL DynamicRead();
public:
    std::string mStrBrokerList;
    std::map<std::string, std::string> mapKafkaName;
    std::map<std::string, INT> mapKafkaPartitionNumber;
};


class CKafkaMgr
{
public:
	CKafkaMgr();
	~CKafkaMgr();

public:
    void InitTopic(std::string strTopic);
    
    void InitGcRouteTopic();

    void InitScRouteTopic();

	void InitOffcialRouteTopic();

    void InitUserInfoTopic();

	void InitScApnsTopic();
	void InitGcApnsTopic();

    bool ProducerByUserId(std::string strTopic, const USERID& user_id, void* strContent, WORD mwLen);
    bool ProducerByUserId(std::string strTopic, const USERID& user_id, void* strContent, WORD mwLen, string strKey);

    void ReleaseProducer();

    WORD GetProducerTopicPartitions(std::string strTopic);
public:
    std::map<string, CKafkaProducer*> mpMapTopicToProducer;

private:
    CKafkaConfig moKafkaConfig;
};
#endif