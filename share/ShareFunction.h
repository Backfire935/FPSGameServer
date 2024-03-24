#ifndef  ____SHAREFUNCTION_H
#define  ____SHAREFUNCTION_H 

#define CMD_REIGSTER      100
#define CMD_UPDATELOGIN   200
#define CMD_CREATEROLE    300

#define CMD_LOGIN      1000
#define CMD_MOVE       2000
#define CMD_PLAYERDATA 3000
#define CMD_LEAVE      4000

#define USER_MAX_MEMBER 20


#include "IDefine.h"

namespace share
{
	enum E_MEMBER_STATE
	{
		M_FREE = 0x00,	//使用-空闲
		M_REQUESTING,   //正在请求数据中
		M_LOGIN,		//使用-登录中
		M_SAVING,       //保存中
		M_LIMIT 		//禁用		
	};


#pragma pack(push,packing)
#pragma pack(1)
	struct S_VECTOR
	{
		f64 x;
		f64 y;
		f64 z;
	};
	struct  S_PLAYER_BASE
	{
		s32  memid;
		s32  socketfd;
		s32  state;
		s32  curhp;
		s32  maxhp;
		f32  speed;
		S_VECTOR   pos;
		S_VECTOR   rot;
		char  nick[20];
		inline void init()
		{
			memset(this, 0, sizeof(S_PLAYER_BASE));
		}
	};


	//账号信息
	struct S_USER_MEMBER_BASE
	{
		inline bool IsT() const { return ID > 0; }
		inline bool IsT_ID(const __int64 id) { return (ID == id); }
		inline bool IsT_Name(const char* fname) { return (stricmp(name, fname) == 0); };

		E_MEMBER_STATE			state;      //账号状态
		int					    ID;         //账号唯一标记 
		char					name[USER_MAX_MEMBER];   //账号 
		char                    password[USER_MAX_MEMBER];//密码
		int						timeCreate;//账号创建时间
		int						timeLastLogin;//上次登录时间
	};



	struct S_REGISTER_BASE
	{
		int         socketfd;
		int		    ID;
		char name[USER_MAX_MEMBER];
		char password[USER_MAX_MEMBER];
	};

	struct S_LOGIN_BASE
	{
		int         socketfd;
		int64_t		ID;
	};



#pragma pack(pop, packing)

	extern bool InitData();

}


#endif