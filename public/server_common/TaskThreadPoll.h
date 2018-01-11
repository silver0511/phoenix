#ifndef __TASK_THREAD_POLL_H
#define __TASK_THREAD_POLL_H

#include "common/BaseThread.h"
#include "common/Queue.h"
#include "ThreadNotify.h"
#include "httpclient.h"
#include "Task.h"

class CTaskThread:public CBaseThread
{
public:
	CTaskThread();
	~CTaskThread();

	bool Init();
	void Stop();
	virtual void Run();	
	virtual void Release();
	
	void AddTask(CTask* ptask);
	long GetQueueSize() {return m_task_num;}	
	
private:
	bool m_bWantToStop;
	long m_task_num;
	CQueue<CTask> m_task_queue;
	CThreadNotify m_empty_notify;
	CHttpClient m_httpclient;
};

class CTaskThreadPoll
{
public:
	CTaskThreadPoll();
	~CTaskThreadPoll();

	bool Init(unsigned int uiWorkerSize);
	void Destory();
	void AddTask(CTask* ptask);
	unsigned long GetThreadQueueSize(unsigned int ithreadid);
	
private:
	unsigned int m_worker_cnt;
	unsigned int m_curworker_id;
	CTaskThread * m_worker_list;
};
#endif

