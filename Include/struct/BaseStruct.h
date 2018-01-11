// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: BastStruct.h
// 创 建 人: 史云杰
// 文件说明: 基本结构
// $_FILEHEADER_END *****************************

#ifndef __BASESTRUCT_H
#define __BASESTRUCT_H
#include "base0/platform.h"

struct STRU_EVENT_DATA
{
    enum _T_EVENT
    {
        ENUM_ERROR=0,
        ENUM_CONNECT_SUCCESS,	//链接成功
        ENUM_CONNECT_FAILURE,	//链接失败
        ENUM_CLOSE,				//链接关闭
        ENUM_RECV,				//接受到数据
        ENUM_CLOSE_SOCKET_AT_SEND_COMPLETE,// 在发送完毕后关闭SOCKET
    };
    _T_EVENT	mTypeEvent;
    USERID		miID;
    SOCKET		mhSocket;
    IPTYPE		miIP;
    WORD		mwPort;

    union
    {
        void*		mpParam1;//指针参数
        int64		miParam;//数值参数
    };
    void*		mpRelationPtr;					//上层处理对象的关联指针
};

struct STRU_EVENT_HTTP_DATA : public STRU_EVENT_DATA
{
    string mStrBody;
};

struct STRU_CONNECT_EVENT
{
    USERID		miID;
    IPTYPE		miDestIP;
    WORD		mwDestPort;
    SOCKET		mhSocket;
    int32		miCreateTime;
};

struct STRU_TRHEAD_CONTEXT
{
    void *mpThis;
    DWORD dwIndex;
};
#endif //__BASESTRUCT_H