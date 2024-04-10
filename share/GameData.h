#ifndef  ____GAMEDATA_H
#define  ____GAMEDATA_H

#include "IDefine.h"
#include <string>
#include <cstring>
#include <map>

#define USER_MAX_MEMBER 20

namespace app
{
	enum E_MEMBER_STATE
	{
		M_FREE = 0x00,	//ʹ��-����
		M_REQUESTING,   //��������������
		M_LOGIN,		//ʹ��-��¼��
		M_SAVING,       //������
		M_LIMIT 		//����		
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

	//�˺���Ϣ
	struct S_USER_MEMBER_BASE
	{
		inline bool IsT() const { return ID > 0; }
		inline bool IsT_ID(const __int64 id) { return (ID == id); }
		inline bool IsT_Name(const char* fname) { return (stricmp(name, fname) == 0); };

		E_MEMBER_STATE			state;      //�˺�״̬
		int					    ID;         //�˺�Ψһ��� 
		char					name[USER_MAX_MEMBER];   //�˺� 
		char                    password[USER_MAX_MEMBER];//����
		int						timeCreate;//�˺Ŵ���ʱ��
		int						timeLastLogin;//�ϴε�¼ʱ��
	};



	struct S_REGISTER_BASE
	{
		u32         gate_socketfd;
		u16		   gate_port;
		u32			center_socketfd;
		u16			center_port;
		u32			db_socketfd;
		u16			db_port;
		u32			ID;//���ݿ��˺�Ψһ���
		char name[USER_MAX_MEMBER];
		char password[USER_MAX_MEMBER];
	};

	struct S_LOGIN_BASE
	{
		int         socketfd;
		int64_t		ID;
	};

#pragma pack(pop, packing)

	extern std::map<std::string, S_USER_MEMBER_BASE*>   __AccountsName;
	extern std::map<int, S_USER_MEMBER_BASE*>           __AccountsID;
	extern S_USER_MEMBER_BASE* FindMember(std::string name);
	extern S_USER_MEMBER_BASE* FindMember(int memid);

}
#endif