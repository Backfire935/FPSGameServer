#include "AppPlayer.h"
#include <list>
#include <time.h>
#include "../../share/ShareFunction.h"
#include "GameData.h"
#include<string>

namespace app
{
	IContainer* __AppPlayer = NULL;
	std::map<int, S_PLAYER_BASE*>  __Onlines;//��ҵ�½��������
	std::list<S_PLAYER_BASE*>  __PlayersPool;//������ճ�
	u64  temptime = 0;

	
	AppPlayer::AppPlayer()
	{
	}

    AppPlayer::~AppPlayer()
	{
	}

	void AppPlayer::onInit()
	{
		IContainer::onInit();
	}


	void AppPlayer::onUpdate()
	{
		IContainer::onUpdate();
	}

	bool app::AppPlayer::onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_ConnectSecure))
		{
			LOG_MSG("AppPlayer err....line:%d \n", __LINE__);
			return false;
		}

		switch (cmd)//��DBServer����Ϣ
		{
		case CMD_MOVE:onSendMove(ts, c); break;
		case CMD_PLAYERDATA:onSendGetPlayerData(ts, c); break;
		case 9999:
			{
			int len = 0;
			char* str1 = NULL;
			char str2[20];
			memset(str2, 0, 20);

			ts->read(c->ID, len);
			str1 = new char[len];
			ts->read(c->ID, str1, len);

			ts->read(c->ID, str2, 20);
			if (__TcpDB->getData()->state < func::C_Connect)
			{
				LOG_MSG("DB server not connect...\n");
				return false;
			}
			//����
			__TcpDB->begin( 9999);
			__TcpDB->sss( len);
			__TcpDB->sss( str1, len);
			__TcpDB->sss(str2, 20);
			__TcpDB->end();

			delete[] str1;
			}
			break;
		}

		return false;
	}


	//2000 ����ƶ�
	void AppPlayer::onSendMove(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 memid;
		f32 speed;
		S_VECTOR pos;
		S_VECTOR rot;
		ts->read(c->ID, memid);
		ts->read(c->ID, speed);
		ts->read(c->ID, &pos, sizeof(S_VECTOR));
		ts->read(c->ID, &rot, sizeof(S_VECTOR));
		if (__TcpDB->getData()->state < func::C_Connect)
		{
			LOG_MSG("DB server not connect...\n");
			return;
		}
		//�����յ������ݷ���DBServer
		__TcpDB->begin(CMD_MOVE);
		__TcpDB->sss(memid);
		__TcpDB->sss(speed);
		__TcpDB->sss(&pos, sizeof(S_VECTOR));
		__TcpDB->sss(&rot, sizeof(S_VECTOR));
		__TcpDB->end();
	}
	//3000 ��ȡ�����������
	void AppPlayer::onSendGetPlayerData(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 memid;
		ts->read(c->ID, memid);
		if (__TcpDB->getData()->state < func::C_Connect)
		{
			LOG_MSG("DB server not connect...\n");
			return;
		}
		//�����յ������ݷ���DBServer
		__TcpDB->begin(CMD_PLAYERDATA);
		__TcpDB->sss(memid);
		__TcpDB->end();
	}

	bool AppPlayer::onClientCommand(net::ITcpClient* tc, const u16 cmd)
	{
		
		switch (cmd)
		{
		case CMD_PLAYERDATA:OnRecvGetPlayerData(tc); break;
		}
		return true;
	}



	void AppPlayer::OnRecvGetPlayerData(net::ITcpClient* tc)
	{
		
	}



}
