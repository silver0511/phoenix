#include "ServerToKafka.h"

CServerToKafka::CServerToKafka()
{
    mpKafkaMgr = NULL;
}

CServerToKafka::~CServerToKafka()
{
}

void CServerToKafka::InitGcRouteTopic()
{
    mpKafkaMgr = new CKafkaMgr();
	if (NULL == mpKafkaMgr)
		throw;

    mpKafkaMgr->InitGcRouteTopic();
}

void CServerToKafka::InitScRouteTopic()
{
    mpKafkaMgr = new CKafkaMgr();
	if (NULL == mpKafkaMgr)
		throw;

    mpKafkaMgr->InitScRouteTopic();
}

void CServerToKafka::InitOffcialRouteTopic()
{
	mpKafkaMgr = new CKafkaMgr();
	if (NULL == mpKafkaMgr)
		throw;

	mpKafkaMgr->InitOffcialRouteTopic();
}

void CServerToKafka::InitScApnsTopic()
{
    mpKafkaMgr = new CKafkaMgr();
	if (NULL == mpKafkaMgr)
		throw;

    mpKafkaMgr->InitScApnsTopic();
}

void CServerToKafka::InitGcApnsTopic()
{
     mpKafkaMgr = new CKafkaMgr();
	if (NULL == mpKafkaMgr)
		throw;

    mpKafkaMgr->InitGcApnsTopic();
}

void CServerToKafka::releaseKafka()
{
    if(mpKafkaMgr)
	{
		mpKafkaMgr->ReleaseProducer();
		SAFE_DELETE(mpKafkaMgr);
	}
}