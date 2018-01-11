#include "TaskThreadPoll.h"
CTaskThread::CTaskThread()
{
	m_bWantToStop = false;
	m_task_num = 0;
}

CTaskThread::~CTaskThread()
{	
}

bool CTaskThread::Init()
{
	CBaseThread::BeginThread(0);
	return true;
}

void CTaskThread::Release()
{	
	return;
}

void CTaskThread::Stop()
{
	mbIsRun = false;
	return;
}

void CTaskThread::AddTask(CTask* ptask)
{
    m_empty_notify.Lock();
    m_task_queue.AddTail(ptask);
	m_task_num++;
	m_empty_notify.Signal();
    m_empty_notify.Unlock();

	return;
}
 
void CTaskThread::Run(void)
{
	while (getRunState())
	{	
		m_empty_notify.Lock();
     	if (m_task_num <= 0)
		{
			m_empty_notify.Wait();
		}

		CTask* ptask = m_task_queue.GetandDelHead();
		m_task_num--;
		m_empty_notify.Unlock();
		
		ptask->SetParamter((void *) (&m_httpclient));
		ptask->run();

        delete ptask;
        ptask = NULL;
	} 

	return;
}

CTaskThreadPoll::CTaskThreadPoll()
{
	m_worker_cnt = 0;
	m_curworker_id = 0;
	m_worker_list = NULL;
}

CTaskThreadPoll::~CTaskThreadPoll()
{

}

bool CTaskThreadPoll::Init(unsigned int uiWorkerSize)
{
	m_worker_cnt = (uiWorkerSize > 1000) ? 1000:uiWorkerSize;	
	m_worker_list = new CTaskThread[m_worker_cnt];

	if (NULL == m_worker_list) 
    {
		return false;
	}
	
	for (unsigned int i = 0; i < m_worker_cnt; i++) 
	{
		if (!m_worker_list[i].Init())
		{			
			return false;
		}		
	}

	return true;
}

unsigned long CTaskThreadPoll::GetThreadQueueSize(unsigned int ithreadid)
{
	if (NULL == m_worker_list)
	{
		return 0;
	}

	return m_worker_list[ithreadid].GetQueueSize();
}

void CTaskThreadPoll::AddTask(CTask* ptask)
{
	if (m_curworker_id >= m_worker_cnt)
	{
		m_curworker_id = 0;
	}
	
	m_worker_list[m_curworker_id].AddTask(ptask);
	m_curworker_id++;
	return;
}

void CTaskThreadPoll::Destory()
{
	if (m_worker_list == NULL)
	{
		return;
	}

	for (unsigned int i = 0; i < m_worker_cnt; i++) 
	{
		m_worker_list[i].Stop();
	}

    if(m_worker_list)
	{
		delete [] m_worker_list;
	}
	return;        
}

