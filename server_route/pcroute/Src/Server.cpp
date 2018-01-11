#include <signal.h>
#include "stdafx.h"

#include "ServerMgr.h"

volatile bool gbRunning = true;

void Task(int port, char * pstrapptype, int log_type)
{
	CSystem::InitRandom();

    std::string szapptype = pstrapptype;
    if (szapptype != "web")
    {
        szapptype = "app";
    }

#ifdef TRACE_LOG
	std::string log_file_name = g_server_name + "_" + std::to_string(port);
	CSystem::CreateDebugInfo(log_file_name.c_str(), log_type, LOG_DEBUG);
#endif

	CServerMgr*  lpServer = new CServerMgr;
	if (1!=lpServer->Open(port, szapptype))
	{
		delete lpServer;
		LOG_TRACE(LOG_ERR,false, __FUNCTION__, g_server_name + " Start failt Server .....");
		return ;
	}
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, g_server_name + "  Server Open Success");
	
	while(gbRunning)
	{
		CSystem::Sleep(10);
	}

	LOG_TRACE(LOG_NOTICE,true, __FUNCTION__,"Exit Server .");
	lpServer->Close();
	delete lpServer;
	LOG_CLOSE();
}

static void sig_1(int)
{	
}
static void sig_2(int code)
{//退出信号
	LOG_TRACE(LOG_NOTICE,true, __FUNCTION__,"SESSION OUT:" << code);
}
static void sig_quit(int)
{
	gbRunning=false;

}
int main(int argc, char* argv[])
{
	signal(SIGPIPE,sig_1);
	signal(SIGTERM,sig_quit);
	signal(SIGHUP, sig_2);
	if(argc < 4)
	{
		printf("argv is invalid missing port");
		return 1;
	}


	Task(atoi(argv[1]), argv[2], atoi(argv[3]));
	return 1;
}

