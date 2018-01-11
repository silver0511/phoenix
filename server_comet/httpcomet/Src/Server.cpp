#include "stdafx.h"
#include "ServerMgr.h"

volatile bool gbRunning = true;

void Task(int port, int log_type);

#include <signal.h>
//bool gbRunning = true;
static void sig_1(int code)
{//退出信号
	LOG_TRACE(LOG_CRIT,true, __FUNCTION__,"SESSION OUT:" << code);
}

static void sig_2(int code)
{//退出信号
	LOG_TRACE(LOG_CRIT,true, __FUNCTION__,"SESSION OUT:" << code);
}

static void sig_quit(int)
{//退出信号
	gbRunning=false;

}
int main(int argc, char* argv[])
{
	signal(SIGPIPE,sig_1);
	//	signal(SIGFPE,sig_quit);
	signal(SIGTERM,sig_quit);
	signal(SIGHUP, sig_2);
	//	signal(SIGINT,sig_quit);
	//	signal(SIGCHLD,sig_quit);
	//	signal(SIGALRM,sig_quit);	
	//	signal(SIGQUIT,sig_quit);


	//pid_t pid; 
	//pid = fork();
	//if (pid>0) 
	//{
	//	exit(0);
	//	return -1;
	//} 
	//else if (pid<0) 
	//{
	//	printf("Failure!!!!\n");
	//	exit(-1);
	//	return -1;
	//} 

	//端口参数
	if(argc < 3)
	{
		printf("argv is invalid missing port");
		return 1;
	}


	Task(atoi(argv[1]), atoi(argv[2]));
	return 1;
}

void Task(int port, int log_type)
{
	CSystem::InitRandom();
#ifdef TRACE_LOG
	std::string log_file_name = g_server_name + "_" + std::to_string(port);
	CSystem::CreateDebugInfo(log_file_name.c_str(), log_type, LOG_DEBUG);
#endif //TRACE_LOG
	CServerMgr*  lpServer = new CServerMgr;
	if (1!=lpServer->Open(port))
	{
		delete lpServer;
		LOG_TRACE(LOG_CRIT,true, __FUNCTION__, g_server_name + " Start failt Server .....");
		return ;
	}
	LOG_TRACE(LOG_ALERT , true, __FUNCTION__, g_server_name + " Server Open Success Port:" << port);


	while(gbRunning)
	{
		CSystem::Sleep(10);
	}

	LOG_TRACE(LOG_ALERT,true, __FUNCTION__,"Exit Server .");
	lpServer->Close();
	delete lpServer;
	LOG_CLOSE();
}
