#ifndef __FRIENDLISTTASK_H__
#define __FRIENDLISTTASK_H__
#include "Task.h"

#define FULLGET 0 
#define INCGET  1

class CFriendListTask : public CTask
{
public:
	CFriendListTask():CTask(){};
	CFriendListTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform){};	
	virtual ~CFriendListTask(){};
	virtual void run();
	void DoForwardPackageFail();	
};
#endif

