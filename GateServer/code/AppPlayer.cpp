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

		switch (cmd)//�����淢��Ϣ
		{
		case CMD_REIGSTER:onSendReigster(ts, c); break;
		case CMD_LOGIN:onSendLogin(ts, c); break;
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
			
			if (__TcpCenter->getData()->state < func::C_Connect)
			{
				LOG_MSG("Center server not connect...\n");
				return false;
			}
			//����
			__TcpCenter->begin( 9999);
			__TcpCenter->sss( len);
			__TcpCenter->sss( str1, len);
			__TcpCenter->sss(str2, 20);
			__TcpCenter->end();

			delete[] str1;
			}
			break;
		}

		return false;
	}

	//100 ע��
	void AppPlayer::onSendReigster(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		char name[USER_MAX_MEMBER];
		char password[USER_MAX_MEMBER];
		memset(name, 0, USER_MAX_MEMBER);
		memset(password, 0, USER_MAX_MEMBER);
		//��ȡ���ܵ�������
		ts->read(c->ID, name, USER_MAX_MEMBER);
		ts->read(c->ID, password, USER_MAX_MEMBER);
		if (__TcpCenter->getData()->state < func::C_Connect)
		{
			LOG_MSG("Center server not connect...\n");
			return;
		}
		//�����յ������ݷ���DBServer
		__TcpCenter->begin(CMD_REIGSTER);
		__TcpCenter->sss(name, USER_MAX_MEMBER);
		__TcpCenter->sss(password, USER_MAX_MEMBER);
		__TcpCenter->end();
	}
	//1000 ��½
	void AppPlayer::onSendLogin(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		char name[USER_MAX_MEMBER];
		char password[USER_MAX_MEMBER];
		memset(name, 0, USER_MAX_MEMBER);
		memset(password, 0, USER_MAX_MEMBER);
		//��ȡ���ܵ�������
		ts->read(c->ID, name, USER_MAX_MEMBER);
		ts->read(c->ID, password, USER_MAX_MEMBER);
		if (__TcpCenter->getData()->state < func::C_Connect)
		{
			LOG_MSG("Center server not connect...\n");
			return;
		}
		//�����յ������ݷ���DBServer
		__TcpCenter->begin(CMD_LOGIN);
		__TcpCenter->sss(name, USER_MAX_MEMBER);
		__TcpCenter->sss( password, USER_MAX_MEMBER);
		__TcpCenter->end();

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
		//Center��������Ļ������������������䷢�ͻ��ߵ�ping���ȷ���
		if (__TcpGame[1] == nullptr || __TcpGame[1]->getData()->state < func::C_Connect)
		{
			LOG_MSG("Game server not connect...\n");
			return;
		}
		//�����յ������ݷ���DBServer
		__TcpGame[1]->begin(CMD_MOVE);
		__TcpGame[1]->sss(memid);
		__TcpGame[1]->sss(speed);
		__TcpGame[1]->sss(&pos, sizeof(S_VECTOR));
		__TcpGame[1]->sss(&rot, sizeof(S_VECTOR));
		__TcpGame[1]->end();
	}
	//3000 ��ȡ�����������
	void AppPlayer::onSendGetPlayerData(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 memid;
		ts->read(c->ID, memid);
		if (__TcpCenter->getData()->state < func::C_Connect)
		{
			LOG_MSG("Center server not connect...\n");
			return;
		}
		//�����յ������ݷ���Center
		__TcpCenter->begin(CMD_PLAYERDATA);
		__TcpCenter->sss(memid);
		__TcpCenter->end();
	}

	bool AppPlayer::onClientCommand(net::ITcpClient* tc, const u16 cmd)
	{
		
		switch (cmd)
		{
		case CMD_REIGSTER:OnRecvReigster(tc); break;
		case CMD_LOGIN:OnRecvLogin(tc); break;
		case CMD_PLAYERDATA:OnRecvGetPlayerData(tc); break;
		}
		return true;
	}

	void AppPlayer::OnRecvReigster(net::ITcpClient* tc)
	{
		s32 registerCode;//ע�����
		tc->read(registerCode);

		auto c = __TcpServer->client(tc->getSocket(), true);
		if (c == nullptr)
		{
			LOG_MSG("AppPlayer err... line:%d __TcpServer == nullptr \n ", __LINE__);
			return;
		}
		//���ظ��ͻ��� ע��ɹ���Ϣ
		__TcpServer->begin(c->ID, CMD_REIGSTER);
		__TcpServer->sss(c->ID, registerCode);
		__TcpServer->end(c->ID);
	}
	void AppPlayer::OnRecvLogin(net::ITcpClient* tc)
	{
		S_PLAYER_BASE* playerdata;//ע�����
		tc->read(&playerdata, sizeof(app::S_PLAYER_BASE));

		auto c = __TcpServer->client(tc->getSocket(), true);
		if (c == nullptr)
		{
			LOG_MSG("AppPlayer err... line:%d S_CLIENT_BASE c == nullptr \n ", __LINE__);
			return;
		}
		//���ظ��ͻ��� ��½�ɹ���Ϣ
		__TcpServer->begin(c->ID, CMD_LOGIN);
		__TcpServer->sss(c->ID, 0);
		__TcpServer->sss(c->ID, playerdata, sizeof(S_PLAYER_BASE));
		__TcpServer->end(c->ID);
	}

	void AppPlayer::OnRecvGetPlayerData(net::ITcpClient* tc)
	{
		
	}



}
