// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <iostream>
#include "base0/platform.h"
#include "common/CriticalSection.h"
#include "common/Debug_log.h"
#include "common/basefun.h"
#include "common/System.h"
#include "common/U9Assert.h"
#include "common/U9Exception.h"

#ifdef WIN32
//#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif //WIN32

// TODO: reference additional headers your program requires here

