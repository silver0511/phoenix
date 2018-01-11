// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：Exception.h
// 创建人： 史云杰
// 文件说明:异常处理
// $_FILEHEADER_END ******************************

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <stdio.h>
#include <string.h>
#include "base0/platform.h"
// ----------------------------------
class GeneralException
{
public:
    GeneralException(const char *m, int e = 0) 
	{
		strcpy(msg,m);
		err=e;
	}
    char msg[128];
	int err;
};

// -------------------------------------
class StreamException : public GeneralException
{
public:
	StreamException(const char *m) : GeneralException(m) {}
	StreamException(const char *m,int e) : GeneralException(m,e) {}
};

// ----------------------------------
class SockException : public StreamException
{
public:
    SockException(const char *m="Socket") : StreamException(m) {}
    SockException(const char *m, int e) : StreamException(m,e) {}
};
// ----------------------------------
class EOFException : public StreamException
{
public:
    EOFException(const char *m="EOF") : StreamException(m) {}
    EOFException(const char *m, int e) : StreamException(m,e) {}
};

// ----------------------------------
class CryptException : public StreamException
{
public:
    CryptException(const char *m="Crypt") : StreamException(m) {}
    CryptException(const char *m, int e) : StreamException(m,e) {}
};


// ----------------------------------
class TimeoutException : public StreamException
{
public:
    TimeoutException(const char *m="Timeout") : StreamException(m) {}
};
#endif //_EXCEPTION_H