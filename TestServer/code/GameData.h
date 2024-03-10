#ifndef ____GAMEDATE_H
#define  ____GAMEDATA_H

#include"AppGlobal.h"

namespace app
{
#pragma pack(push,packing)
#pragma pack(1)

	struct S_VECTOR
	{
		f64 x;
		f64 y;
		f64 z;
	};
	;
	
	struct S_PLAYER_BASE
	{
		s32  memid;
		s32 socketfd;
		s32 state;
		s32 curhp;
		s32 maxhp;
		f32 speed;

		S_VECTOR pos;
		S_VECTOR rot;
		char nick[20];
		inline void init()
		{
			memset(this, 0 , sizeof(S_PLAYER_BASE));
		}
	};

	#pragma pack(pop,packing)
}

#endif

