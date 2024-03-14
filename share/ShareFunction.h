#ifndef __SHAREFUNCTION_H
#define  __SHAREFUNCTION_H

#define CMD_LOGIN           1000
#define CMD_MOVE            2000
#define CMD_PLAYERDATA 3000
#define CMD_LEAVE            4000
#include "IDefine.h"


namespace share
{
    extern bool InitData();
#pragma pack(push,packing)
#pragma  pack(1)
	struct S_LOGIN_1000
	{
		u32	memid;
		u8	rolenum;
		u32	gate_socketfd;
		u16	gate_port;
		u32	center_socketfd;
		u16	center_port;
	};
#pragma pack(pop,packing )
}

#endif
