#include "stdafx.h"
#include <sstream>
#include "common/Debug_log.h"
#include "ServerMgr.h"
#include "const/KafkaDef.h"
#include "CKafkaConsumeThread.h"

void ExampleRebalanceCb::part_list_print (RdKafka::ErrorCode err, const std::vector<RdKafka::TopicPartition*>&partitions)
{
	std::stringstream topickinfo;
	for (unsigned int i = 0 ; i < partitions.size() ; i++)
	{
		topickinfo <<  partitions[i]->topic() << " [" <<  partitions[i]->partition() << "], ";		
	}

	LOG_TRACE(LOG_CRIT, true, __FUNCTION__," RebalanceCb: " << RdKafka::err2str(err) << topickinfo.str());
}

void ExampleRebalanceCb::rebalance_cb (RdKafka::KafkaConsumer *consumer,
							     			  RdKafka::ErrorCode err,
					                          std::vector<RdKafka::TopicPartition*> &partitions) 
{
    part_list_print(err, partitions);
    if (err == RdKafka::ERR__ASSIGN_PARTITIONS) 
	{
      consumer->assign(partitions);
    } 
	else 
	{
      consumer->unassign();
    }
}

void GCEventCb::event_cb(RdKafka::Event &event)
{
	switch (event.type())
	{
		case RdKafka::Event::EVENT_ERROR:
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " ERROR (" << RdKafka::err2str(event.err()) << "): " << event.str());
			if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, " all brokers down " ); 
			}
		}	
		break;

	case RdKafka::Event::EVENT_STATS:
		LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, " \"STATS\": " << event.str());
		break;

	case RdKafka::Event::EVENT_LOG:
		//LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " LOG-" << event.severity()
		//								<< "-" << event.fac()
		//								<< "-" << event.str());
		break;

	case RdKafka::Event::EVENT_THROTTLE:
		LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, " THROTTLED: " << event.throttle_time() 
  										 << "ms by " << event.broker_name()
  										 << " id "   << (int)event.broker_id());
		break;

	default:
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "EVENT " << event.type() 
										 << " (" << RdKafka::err2str(event.err()) 
										 << "): "<< event.str());
		break;
	}

}

CKafkaConsumeThread::CKafkaConsumeThread()
{
	m_pconsumer = NULL;
	m_ulReceiveSucCnt = 0;
	m_ulReceiveFailCnt = 0;
}

CKafkaConsumeThread::CKafkaConsumeThread(CServerMgr* pServerMgr, uint32 threadid):m_pservermgr(pServerMgr), m_threadid(threadid)
{
	m_pconsumer = NULL;
	m_ulReceiveSucCnt = 0;
	m_ulReceiveFailCnt = 0;
}


CKafkaConsumeThread::~CKafkaConsumeThread()
{

}

bool CKafkaConsumeThread::Init()
{		
	if(-1 == moKafkaConfig.Load(DEF_KAFKA_INI))
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," kafka.Load Fail");
        return false;
	}

	std::string errstr;
	RdKafka::Conf *tconf;
	RdKafka::Conf* conf;
	std::vector<std::string> topics;
	topics.push_back(moKafkaConfig.mapKafkaName[IM_GC_TOPIC]);

	conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

	//conf->set("debug", "all", errstr);
	//set debug info
	//if (conf->set("debug", "all", errstr) != RdKafka::Conf::CONF_OK)
	//{
	//	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " failed to set debug "<< errstr);
	//	return false;
	//}	



	if (conf->set("rebalance_cb", &m_ex_rebalance_cb, errstr) != RdKafka::Conf::CONF_OK) 
	{    
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " failed to set rebalancecb "<< errstr);
        return false;
    }

	if (conf->set("group.id",  m_pservermgr->moConfig.m_sgroupid, errstr) != RdKafka::Conf::CONF_OK) 
	{    
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " failed to set groupid "<< errstr);
        return false;
    }

	if (conf->set("metadata.broker.list",  moKafkaConfig.mStrBrokerList, errstr) != RdKafka::Conf::CONF_OK)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " failed to set brokers "<< errstr);
        return false;
	}

	conf->set("log_level", "5", errstr);
	conf->set("event_cb", &m_ex_event_cb, errstr);
	conf->set("default_topic_conf", tconf, errstr);
	conf->set("socket.keepalive.enable", "true", errstr);
	conf->set("socket.blocking.max.ms", "50", errstr);
	conf->set("fetch.wait.max.ms", "200", errstr);

	string log_level;
	string fetch_wait_max_ms;
	string block_max_ms;
	string socket_keeylive_enable;
	string fetch_message_max_bytes;
	string max_partition_fetch_bytes;
	conf->get("log_level", log_level);
	conf->get("fetch.wait.max.ms", fetch_wait_max_ms);
	conf->get("socket.blocking.max.ms", block_max_ms);
	conf->get("socket.keepalive.enable", socket_keeylive_enable);
	conf->get("fetch.message.max.bytes", fetch_message_max_bytes);
	conf->get("max.partition.fetch.bytes", max_partition_fetch_bytes);

	LOG_TRACE(LOG_ALERT, false, __FUNCTION__, " conf seted log_level = " << log_level.c_str()
																		 << " fetch_wait_max_ms = " << fetch_wait_max_ms.c_str()
																		 << " socket.blocking.max.ms = " << block_max_ms.c_str()
																		 << " socket.keepalive.enable = " << socket_keeylive_enable.c_str()
																		 << " fetch_message_max_bytes = " << fetch_message_max_bytes.c_str()
																		 << " max_partition_fetch_bytes = " << max_partition_fetch_bytes.c_str());

	delete tconf;

	m_pconsumer = RdKafka::KafkaConsumer::create(conf, errstr);
	if (!m_pconsumer) 
	{		
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " failed to create consumer"<< errstr);
		return false;
	}
	delete conf;

	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, " created consumer "<< m_pconsumer->name());

	RdKafka::ErrorCode err = m_pconsumer->subscribe(topics);
	if (err) 
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " failed to subscribe to "<< topics.size() 
										<< " topics: "<< RdKafka::err2str(err));		
		return false;
	}

	CBaseThread::BeginThread(0);
	return true;	
}

void CKafkaConsumeThread::Run()
{
	while (getRunState())
	{
		RdKafka::Message *msg = m_pconsumer->consume(1000);
		msg_consume(msg, NULL);
		delete msg;
	}
	return;
}

void CKafkaConsumeThread::Release()
{
	if (NULL == m_pconsumer)
	{
		return;
	}
	m_pconsumer->close();
	delete m_pconsumer;
	
	RdKafka::wait_destroyed(5000);
}

void CKafkaConsumeThread::Stop()
{
	mbIsRun = false;
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__," stop ");
	return;
}


void CKafkaConsumeThread::msg_consume(RdKafka::Message* message, void* opaque)
{
	if (NULL == message)
	{
		return;
	}

	switch (message->err()) 
	{
	case RdKafka::ERR_NO_ERROR:
		{			
			m_ulReceiveSucCnt++;
			LOG_TRACE(LOG_DEBUG, false, __FUNCTION__," topic:" << message->topic_name() 
										    << " partition: " << message->partition()
                                            << " offset: " << message->offset());
			
			StrConsumeMsg* msg =  new StrConsumeMsg();
			if (NULL == msg)
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__," new conume msg error ");
				return;
			}

  			msg->uldatalen = message->len();			
			msg->data = new BYTE[msg->uldatalen];
            if (NULL != message->key())
            {
                msg->szkey = *(message->key());
            }
            else
            {
                delete msg;
                LOG_TRACE(LOG_ERR, false, __FUNCTION__," msg->key is null");
                return;
            } 

			if (NULL == msg->data )
			{
				delete msg;
				LOG_TRACE(LOG_ERR, false, __FUNCTION__," new conume data error ");
				return;
			}
			memcpy(msg->data, (BYTE *)message->payload(), msg->uldatalen);

			if (NULL != m_pservermgr)
			{
				m_pservermgr->AddTailMsg(msg);				
			}
			else
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__," m_pservermgr is abnomal");
				return;
			}			
		}	  
	  	break;
	case RdKafka::ERR__UNKNOWN_TOPIC:
	case RdKafka::ERR__UNKNOWN_PARTITION:
		{
			m_ulReceiveFailCnt++;
			LOG_TRACE(LOG_ERR, false, __FUNCTION__," Consume failed: " << message->errstr());
		}	  	
		break;

	case RdKafka::ERR__TIMED_OUT:
		//m_ulReceiveFailCnt++;
        //LOG_TRACE(LOG_ERR, false, __FUNCTION__," Consume failed: " << message->errstr());
		break;
	case RdKafka::ERR__PARTITION_EOF:
		break;

	default:
		m_ulReceiveFailCnt++;
		LOG_TRACE(LOG_ERR, false, __FUNCTION__," Consume failed: " << message->errstr());
		break;
	}
	return;
}

