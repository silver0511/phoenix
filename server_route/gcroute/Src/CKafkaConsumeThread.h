#ifndef __KAFKA_CONSUME_THREAD_H__
#define __KAFKA_CONSUME_THREAD_H__
#include <vector>
#include "librdkafka/rdkafkacpp.h"
#include "librdkafka/rdkafka.h"
#include "common/BaseThread.h"
#include "kafka/KafkaMgr.h"

class CServerMgr;

class ExampleRebalanceCb : public RdKafka::RebalanceCb 
{
public:
	virtual void rebalance_cb (RdKafka::KafkaConsumer *consumer,
	  							RdKafka::ErrorCode err, 
	  							std::vector<RdKafka::TopicPartition*> &partitions);
private:
	static void part_list_print (RdKafka::ErrorCode err, const std::vector<RdKafka::TopicPartition*>&partitions);
};

class GCEventCb : public RdKafka::EventCb 
{
public:
	virtual void event_cb (RdKafka::Event &event);
};

class CKafkaConsumeThread :public CBaseThread
{
public:
	CKafkaConsumeThread();
	CKafkaConsumeThread(CServerMgr* pServerMgr, uint32 threadid);
	virtual ~CKafkaConsumeThread();

	bool Init();
	virtual void Run();
	virtual void Release();
	void Stop();
	void msg_consume(RdKafka::Message* message, void* opaque);
	uint64 GetKafkaSucMsgCnt(){return m_ulReceiveSucCnt;}
	uint64 GetKafkaFailMsgCnt(){return m_ulReceiveFailCnt;}
	
private:
	CServerMgr* m_pservermgr;
	RdKafka::KafkaConsumer* m_pconsumer;
	ExampleRebalanceCb m_ex_rebalance_cb;
	GCEventCb m_ex_event_cb;
	uint64 m_ulReceiveSucCnt;
	uint64 m_ulReceiveFailCnt;
    CKafkaConfig moKafkaConfig;
	uint32 m_threadid;
};

#endif

