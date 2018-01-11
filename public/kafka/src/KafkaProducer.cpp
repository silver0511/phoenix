// #include "librdkafka/rdkafka.h"
// #include "zookeeper/zookeeper.h"
// #include "zookeeper/zookeeper.jute.h"

#include "kafka/KafkaProducer.h"
#include "const/KafkaDef.h"

CKafkaProducer::CKafkaProducer(std::string strBrokerList, std::string strTopic, INT partition)
{
	mpProducer = NULL;
	mPTopic = NULL;

	mStrTopic = strTopic;
	mwPartition = partition;
	mStrBrokerList = strBrokerList;

	mMaxMessageCount = 100000;
}

CKafkaProducer::~CKafkaProducer()
{
	SAFE_DELETE(mpProducer);
	SAFE_DELETE(mPTopic);
}

bool CKafkaProducer::CreateProducer()
{
	if(NULL == mpProducer)
	{
		std::string errstr;
		RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
		RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

		conf->set("metadata.broker.list", mStrBrokerList, errstr);

		// if(conf->set("debug", "all", errstr) != RdKafka::Conf::CONF_OK) 
		// {
		// 	LOG_TRACE(LOG_ERR, false, __FUNCTION__, " debug failed");
    	// }

		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " mStrBrokermStrBroker = " << mStrBrokerList 
								<< " strTopic = " << mStrTopic);
		conf->set("event_cb", &ex_event_cb, errstr);

	    /* Set delivery report callback */
    	conf->set("dr_cb", &ex_dr_cb, errstr);

        conf->set("log_level", "5", errstr);
        conf->set("batch.num.messages", "10000", errstr);
		conf->set("queue.buffering.max.messages", to_string(mMaxMessageCount), errstr);
		conf->set("queue.buffering.max.ms", "500", errstr);
		conf->set("socket.blocking.max.ms", "50", errstr);
		conf->set("socket.keepalive.enable", "true", errstr);
		conf->set("request.required.acks", "0", errstr);
//        conf->set("compression.codec", "gzip", errstr);


        string log_level;
        string acks;
        string batch_num;
        string queue_max_msg;
        string queue_max_ms;
        string block_max_ms;
        string socket_keeylive_enable;
        string compression_codec;
        conf->get("log_level", log_level);
		conf->get("request.required.acks", acks);
        conf->get("batch.num.messages", batch_num);
        conf->get("queue.buffering.max.messages", queue_max_msg);
        conf->get("queue.buffering.max.ms", queue_max_ms);
        conf->get("socket.blocking.max.ms", block_max_ms);
        conf->get("socket.keepalive.enable", socket_keeylive_enable);
        conf->get("compression.codec", compression_codec);

        LOG_TRACE(LOG_ALERT, false, __FUNCTION__, " conf seted log_level = " << log_level.c_str()
                 << " request.required.acks = " << acks.c_str()
                 << " batch.num.messages = " << batch_num.c_str()
                 << " queue.buffering.max.messages = " << queue_max_msg.c_str()
                 << " queue.buffering.max.ms = " << queue_max_ms.c_str()
                 << " socket.blocking.max.ms = " << block_max_ms.c_str()
                 << " socket.keepalive.enable = " << socket_keeylive_enable.c_str()
                 << " compression.codec = " << compression_codec
                 << " partition = " << mwPartition);

		mpProducer = RdKafka::Producer::create(conf, errstr);
		if (!mpProducer) 
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "init producer failed");
			return false;
		}

		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "Created producer name is = " << mpProducer->name());

		/*
		* Create topic handle.
		*/
		mPTopic = RdKafka::Topic::create(mpProducer, mStrTopic, tconf, errstr);
		if (!mPTopic) 
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "init producer topic failed");
			return false;
		}

	}

	return true;
}

bool CKafkaProducer::ProducerMessage(USERID user_id, void* strContent, WORD mwLen)
{
	if(!mpProducer)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "create producer is failed");
		return false;
	}

	std::string pass_key = to_string(user_id);
	RdKafka::ErrorCode resp =
	mpProducer->produce(mPTopic, user_id % mwPartition,
			  RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
			  strContent, 
			  mwLen,
			  &pass_key,
			  NULL);

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "init producer topic partition = " << mwPartition
								<< " topic = " << mStrTopic << " strContent = " << strContent
								<< " user_id = " << (USERID)user_id << " mwLen = " << mwLen);

    if (resp != RdKafka::ERR_NO_ERROR)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "producer failed please repeated"
                << " error = " << RdKafka::err2str(resp)
                << " out_len = " << mpProducer->outq_len());
		return false;
	}

    mpProducer->poll(0);
	return true;
}

bool CKafkaProducer::ProducerMessage(UINT64 seperate_number, void* strContent, WORD mwLen, string strKey)
{
	if(!mpProducer)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "create producer is failed");
		return false;
	}

	if(mpProducer->outq_len() > mMaxMessageCount - 5000)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "len = " << mpProducer->outq_len()
							<< " seperate_number = " << seperate_number
							<< " mStrTopic = " << mStrTopic)
		return false;
	}

	RdKafka::ErrorCode resp =
	mpProducer->produce(mPTopic, seperate_number % mwPartition,
			  RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
			  strContent, 
			  mwLen,
			  &strKey,
			  NULL);

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "init producer topic partition = " << mwPartition
								<< " topic = " << mStrTopic << " strContent = " << strContent
								<< " seperate_number = " << (USERID)seperate_number << " mwLen = " << mwLen);

    if (resp != RdKafka::ERR_NO_ERROR)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "producer failed please repeated"
                << " error = " << RdKafka::err2str(resp)
                << " out_len = " << mpProducer->outq_len());
		return false;
	}

    mpProducer->poll(0);
	return true;
}

