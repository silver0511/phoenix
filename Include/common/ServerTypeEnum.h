#ifndef __SERVER_TYPE_ENUM_H
#define __SERVER_TYPE_ENUM_H

enum EnumServerLogicType 
{
    TCP_COMET = 1,
    WS_COMET = 2,
	HTTP_COMET = 3,

    USER_SERVER = 10,
    GC_SERVER,
    SC_SERVER,
    FD_SERVER,
    DB_SERVER,
    EC_SERVER,
	OFFCIAL_SERVER,
    GROUP_SERVER,
	REG_SERVER,

    SC_ROUTE = 100,
    GC_ROUTE,
    PC_ROUTE,

    ALL_SERVER
};

#endif