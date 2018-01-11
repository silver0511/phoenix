#ifndef __KAFKA_BASE_H
#define __KAFKA_BASE_H
#include "librdkafka/rdkafka.h"
#include "zookeeper/zookeeper.h"
#include "zookeeper/zookeeper.jute.h"
#include "librdkafka/rdkafkacpp.h"
#include "common/Debug_log.h"
#include "base0/platform.h"

static bool run = true;
static bool exit_eof = false;

static void sigterm (int sig) {
  run = false;
}


// --------------------------------------------------------------
class MyHashPartitionerCb : public RdKafka::PartitionerCb {
 public:
  int32_t partitioner_cb (const RdKafka::Topic *topic, const std::string *key,
                          int32_t partition_cnt, void *msg_opaque) {
    return djb_hash(key->c_str(), key->size()) % partition_cnt;
  }
 private:

  static inline unsigned int djb_hash (const char *str, size_t len) {
    unsigned int hash = 5381;
    for (size_t i = 0 ; i < len ; i++)
      hash = ((hash << 5) + hash) + str[i];
    return hash;
  }
};

class ExampleEventCb : public RdKafka::EventCb {
 public:
  void event_cb (RdKafka::Event &event) {
    switch (event.type())
    {
      case RdKafka::Event::EVENT_ERROR:
        LOG_TRACE(LOG_INFO, false, __FUNCTION__, "INFO (" << RdKafka::err2str(event.err()) << "): "
                  << event.str());
        // std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
        //     event.str() << std::endl;
        if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
          run = false;
        break;

      case RdKafka::Event::EVENT_STATS:
        LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "\"STATS\": " << event.str());
        // std::cerr << "\"STATS\": " << event.str() << std::endl;
        break;

      case RdKafka::Event::EVENT_LOG:
        LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "LOG: severity = " << event.severity()
                      << " fac = " << event.fac().c_str() << " str = " << event.str().c_str());
        // fprintf(stderr, "LOG-%i-%s: %s\n",
        //         event.severity(), event.fac().c_str(), event.str().c_str());
        break;

      default:
        LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "\"EVENT\": " << event.type() <<
            " (" << RdKafka::err2str(event.err()) << "): " << event.str());

        // std::cerr << "EVENT " << event.type() <<
        //     " (" << RdKafka::err2str(event.err()) << "): " <<
        //     event.str() << std::endl;
        break;
    }
  }
};

class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
 public:
  void dr_cb (RdKafka::Message &message) {
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "Message delivery for (" << message.len() << " bytes): " <<
                      message.errstr());
    // std::cout << "Message delivery for (" << message.len() << " bytes): " <<
    //     message.errstr() << std::endl;
    if (message.key())
      LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "Key: " << *(message.key()) << ";");
      // std::cout << "Key: " << *(message.key()) << ";" << std::endl;
  }
};

class CKafkaProducer
{
public:
    CKafkaProducer(std::string strBrokerList, std::string strTopic, INT partition);
	~CKafkaProducer(void);

public:
    bool CreateProducer();
    bool ProducerMessage(USERID user_id, void* strContent, WORD mwLen);
    bool ProducerMessage(UINT64 seperate_number, void* strContent, WORD mwLen, string strKey);

private:
    RdKafka::Producer* mpProducer;
    ExampleEventCb ex_event_cb;
    ExampleDeliveryReportCb ex_dr_cb;
    RdKafka::Topic* mPTopic;
public: 
    WORD mwPartition;
    std::string mStrTopic;
    std::string mStrBrokerList;
    int mMaxMessageCount;
};


#endif //