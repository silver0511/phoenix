// $_FILEHEADER_BEGIN ****************************
// 文件名称：CircleIndexDef.h
// 创建人：史云杰
// 文件说明：环形索引定义
// $_FILEHEADER_END ******************************
#ifndef __CIRCLEINDEXDEF_H
#define __CIRCLEINDEXDEF_H
//-----------------------------------------------------------------------------
#include "base0/platform.h"
//-----------------------------------------------------------------------------
inline INT INC_CIRCLE_INDEX(INT aiIndex,INT aiCount,INT aiSize)
{
	return (aiIndex+aiCount)%aiSize;
}
inline INT DEC_CIRCLE_INDEX(INT aiIndex,INT aiCount,INT aiSize)
{
	UINT liTemp=(aiIndex+aiSize);
	return (INT)(liTemp-aiCount)%aiSize;
}
inline INT g_GetCircleIndexCount(INT aiStartIndex,INT aiEndIndex,INT aiSize)
{
	int liLen =0;
	if(aiEndIndex>=aiStartIndex)//环形索引定义。重新计算偏移
		liLen=aiEndIndex-aiStartIndex+1;
	else
		liLen=aiSize-aiStartIndex+aiEndIndex+1;
	return liLen;
}
inline INT g_GetCircleIndexDistance(INT aiStartIndex,INT aiEndIndex,INT aiSize)
{
	int liLen =0;
	if(aiEndIndex>=aiStartIndex)//环形索引定义。重新计算偏移
		liLen=aiEndIndex-aiStartIndex;
	else
		liLen=aiSize-aiStartIndex+aiEndIndex;
	return liLen;
}

inline BOOL g_IsCircleIndexRange(INT aiIndex,INT aiStartIndex,INT aiEndIndex,INT aiSize)
{
	return (BOOL )(g_GetCircleIndexCount(aiStartIndex,aiIndex,aiSize)
		<= g_GetCircleIndexCount(aiStartIndex,aiEndIndex,aiSize));
}

inline BOOL g_IsCircleIndexRangebySize(INT aiIndex,INT aiStartIndex,INT aiLen,INT aiSize)
{
	return (BOOL )(g_GetCircleIndexCount(aiStartIndex,aiIndex,aiSize)<= aiLen);
}

#endif //__CIRCLEINDEXDEF_H