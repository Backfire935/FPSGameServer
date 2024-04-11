#include "AppPlayer.h"
#include <list>
#include <time.h>
#include "../../share/ShareFunction.h"
#include "GameData.h"
#include<string>


namespace app
{
	IContainer* __AppPlayer = NULL;
	std::map<int, S_PLAYER_BASE*>  __Onlines;//玩家登陆在线数据
	std::list<S_PLAYER_BASE*>  __PlayersPool;//对象回收池
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

		switch (cmd)//向DBServer发消息
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
			if (__TcpDB->getData()->state < func::C_Connect)
			{
				LOG_MSG("DB server not connect...\n");
				return false;
			}
			//返回
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

	//100 注册
	void AppPlayer::onSendReigster(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		//读取接受到的数据
		S_REGISTER_BASE registerData;
		ts->read(c->ID, &registerData, sizeof(S_REGISTER_BASE));
		//ts->read(c->ID, password, USER_MAX_MEMBER);
		registerData.center_socketfd = c->socketfd;
		registerData.center_port = c->port;

		if (__TcpDB->getData()->state < func::C_Connect)
		{
			LOG_MSG("DB server not connect...\n");
			return;
		}
		//将接收到的数据发往DBServer
		__TcpDB->begin(CMD_REIGSTER);
		__TcpDB->sss(&registerData, sizeof(S_REGISTER_BASE));
		__TcpDB->end();
	}
	//1000 登陆
	void AppPlayer::onSendLogin(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		S_REGISTER_BASE loginData;
		ts->read(c->ID, &loginData, sizeof(S_REGISTER_BASE));
		//ts->read(c->ID, password, USER_MAX_MEMBER);
		loginData.center_socketfd = c->socketfd;
		loginData.center_port = c->port;
		if (__TcpDB->getData()->state < func::C_Connect)
		{
			LOG_MSG("DB server not connect...\n");
			return;
		}
		//将接收到的数据发往DBServer
		__TcpDB->begin(CMD_LOGIN);
		__TcpDB->sss(&loginData, sizeof(S_REGISTER_BASE));
		__TcpDB->end();

	}
	//2000 玩家移动
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
		//将接收到的数据发往DBServer
		__TcpDB->begin(CMD_MOVE);
		__TcpDB->sss(memid);
		__TcpDB->sss(speed);
		__TcpDB->sss(&pos, sizeof(S_VECTOR));
		__TcpDB->sss(&rot, sizeof(S_VECTOR));
		__TcpDB->end();
	}
	//3000 获取其他玩家数据
	void AppPlayer::onSendGetPlayerData(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 memid;
		ts->read(c->ID, memid);
		if (__TcpDB->getData()->state < func::C_Connect)
		{
			LOG_MSG("DB server not connect...\n");
			return;
		}
		//将接收到的数据发往DBServer
		__TcpDB->begin(CMD_PLAYERDATA);
		__TcpDB->sss(memid);
		__TcpDB->end();
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
		s32 registerCode;//注册情况
		S_REGISTER_BASE registerData;
		tc->read(&registerData, sizeof(S_REGISTER_BASE));
		tc->read(registerCode);

		auto c = __TcpServer->client(registerData.center_socketfd, true);
		if (c == nullptr)
		{
			LOG_MSG("AppPlayer err... line:%d __TcpServer == nullptr fd:%d \n ", __LINE__, registerData.center_socketfd);
			return;
		}

		//返回给客户端 注册情况
		__TcpServer->begin(c->ID, CMD_REIGSTER);
		__TcpServer->sss(c->ID, &registerData, sizeof(app::S_REGISTER_BASE));
		__TcpServer->sss(c->ID, registerCode);
		__TcpServer->end(c->ID);
	}
	void AppPlayer::OnRecvLogin(net::ITcpClient* tc)
	{
		s32 loginCode;//注册情况
		S_REGISTER_BASE loginData;
		tc->read(loginCode);
		tc->read(&loginData, sizeof(S_REGISTER_BASE));

		auto c = __TcpServer->client(loginData.center_socketfd, true);
		if (c == nullptr)
		{
			LOG_MSG("AppPlayer err... line:%d S_CLIENT_BASE c == nullptr \n ", __LINE__);
			return;
		}
		//返回给客户端 注册成功消息
		__TcpServer->begin(c->ID, CMD_LOGIN);
		__TcpServer->sss(c->ID, loginCode);
		__TcpServer->sss(c->ID, &loginData, sizeof(S_REGISTER_BASE));
		__TcpServer->end(c->ID);
	}

	void AppPlayer::OnRecvGetPlayerData(net::ITcpClient* tc)
	{
		
	}



}
