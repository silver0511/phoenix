// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：RefObj.h
// 创建人： 史云杰
// 文件说明：引用计数类
// $_FILEHEADER_END ******************************
#ifndef _REFOBJ_H
#define _REFOBJ_H
#include <stdlib.h>
#include "CircleIndexDef.h"
#include "CriticalSection.h"

U9_BEGIN_NAMESPACE
//引用计数类
class CRefObject
{
public:
	CRefObject()
	{
		mlRefCount=0;
		mbNeedDeleteSelf=FALSE;
	}

	virtual ~CRefObject(){};
	//引用计数加1
	inline LONG IncreaseRef()
	{
		LONG llValue=0;
		CRITICAL_SECTION_HELPER(moRefCriticalSection);
		mlRefCount++;
		llValue=mlRefCount;
		return llValue;
	}
	//引用计数减1
	inline LONG DecreaseRef()
	{
		LONG llValue=0;
		BOOL lbNeedDelete = FALSE;
		{
			CRITICAL_SECTION_HELPER(moRefCriticalSection);
			if(mlRefCount>0)
				mlRefCount--;
			llValue=mlRefCount;
			if(0==mlRefCount && mbNeedDeleteSelf)//需要删除
				lbNeedDelete = TRUE;
		}
		if (lbNeedDelete)
			delete this;
		return llValue;
	}

	//需要删除
	inline void  NeedRelease()
	{
		mbNeedDeleteSelf=TRUE;
	}
private:
	CCriticalSection moRefCriticalSection;
	BOOL mbNeedDeleteSelf;
	LONG mlRefCount;
};

U9_END_NAMESPACE
#endif //_REFOBJ_H

