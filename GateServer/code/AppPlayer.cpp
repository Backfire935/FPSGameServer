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

		switch (cmd)//向上面发消息
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
			//返回
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

	//100 注册
	void AppPlayer::onSendReigster(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		//读取接受到的数据
		s32 len = 0;
		S_REGISTER_BASE registerData;

		ts->read(c->ID, len);;
		ts->read(c->ID, registerData.name, len);
		ts->read(c->ID, len);;
		ts->read(c->ID, registerData.password, len);
		registerData.gate_socketfd = c->socketfd;
		registerData.gate_port = c->port;

		if (__TcpCenter->getData()->state < func::C_Connect)
		{
			LOG_MSG("Center server not connect...\n");
			return;
		}
		//将接收到的数据发往CenterServer
		__TcpCenter->begin(CMD_REIGSTER);
		__TcpCenter->sss(&registerData, sizeof(registerData));
		__TcpCenter->end();
	}
	//1000 登陆
	void AppPlayer::onSendLogin(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		//读取接受到的数据
		s32 len = 0;
		S_REGISTER_BASE registerData;

		ts->read(c->ID, len);;
		ts->read(c->ID, registerData.name, len);
		ts->read(c->ID, len);;
		ts->read(c->ID, registerData.password, len);
		registerData.gate_socketfd = c->socketfd;
		registerData.gate_port = c->port;

		if (__TcpCenter->getData()->state < func::C_Connect)
		{
			LOG_MSG("Center server not connect...\n");
			return;
		}
		//将接收到的数据发往DBServer
		__TcpCenter->begin(CMD_LOGIN);
		__TcpCenter->sss(&registerData, sizeof(registerData));
		__TcpCenter->end();

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
		//Center服务器多的话，这里可以用随机分配发送或者低ping优先发送
		if (__TcpGame[1] == nullptr || __TcpGame[1]->getData()->state < func::C_Connect)
		{
			LOG_MSG("Game server not connect...\n");
			return;
		}
		//将接收到的数据发往DBServer
		__TcpGame[1]->begin(CMD_MOVE);
		__TcpGame[1]->sss(memid);
		__TcpGame[1]->sss(speed);
		__TcpGame[1]->sss(&pos, sizeof(S_VECTOR));
		__TcpGame[1]->sss(&rot, sizeof(S_VECTOR));
		__TcpGame[1]->end();
	}
	//3000 获取其他玩家数据
	void AppPlayer::onSendGetPlayerData(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 memid;
		ts->read(c->ID, memid);
		if (__TcpCenter->getData()->state < func::C_Connect)
		{
			LOG_MSG("Center server not connect...\n");
			return;
		}
		//将接收到的数据发往Center
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
		s32 registerCode;//注册情况
		S_REGISTER_BASE registerData;
		tc->read(&registerData, sizeof(S_REGISTER_BASE));
		tc->read(registerCode);

		auto c = __TcpServer->client(registerData.gate_socketfd  , true);
		if (c == nullptr)
		{
			LOG_MSG("AppPlayer err... line:%d __TcpServer == nullptr \n ", __LINE__);
			return;
		}
		//返回给客户端 注册成功消息
		__TcpServer->begin(c->ID, CMD_REIGSTER);
		//__TcpServer->sss(c->ID, &registerData, sizeof(app::S_REGISTER_BASE));
		__TcpServer->sss(c->ID, registerCode);
		__TcpServer->end(c->ID);
	}
	void AppPlayer::OnRecvLogin(net::ITcpClient* tc)
	{
		s32 LoginCode;//注册情况
		S_REGISTER_BASE LoginData;
		tc->read(&LoginData, sizeof(S_REGISTER_BASE));
		tc->read(LoginCode);

		auto c = __TcpServer->client(LoginData.gate_socketfd, true);
		if (c == nullptr)
		{
			LOG_MSG("AppPlayer err... line:%d S_CLIENT_BASE c == nullptr \n ", __LINE__);
			return;
		}
		//返回给客户端 登陆成功消息
		__TcpServer->begin(c->ID, CMD_LOGIN);
		__TcpServer->sss(c->ID, &LoginData, sizeof(S_REGISTER_BASE));
		__TcpServer->sss(c->ID, LoginCode);
		__TcpServer->end(c->ID);
	}

	void AppPlayer::OnRecvGetPlayerData(net::ITcpClient* tc)
	{
		
	}



}
