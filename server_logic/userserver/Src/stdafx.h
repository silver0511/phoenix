#pragma once
#ifdef WIN32
	#ifdef API_EXPORTS
		#define _LINK_API	__declspec(dllexport)
	#else
		#define _LINK_API	__declspec(dllimport)
	#endif
#else
	#define _LINK_API
#endif

#include <iostream>
#include "base0/platform.h"
#include "common/CriticalSection.h"
#include "common/Debug_log.h"
#include "common/basefun.h"
#include "common/System.h"

#ifdef WIN32
//#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif //WIN32



#include "GlobalResource.h"
// TODO: reference additional headers your program requires here




