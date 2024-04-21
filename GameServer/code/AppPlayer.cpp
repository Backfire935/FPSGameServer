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

		switch (cmd)//收到网关发来的消息
		{
		case CMD_MOVE:onSendMove(ts, c); break;
		case CMD_PLAYERDATA:onSendGetPlayerData(ts, c); break;
		case CMD_LEAVE:onSendLeave(ts, c); break;
		default:
			break;
		}

		return false;
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

	}

	//4000 玩家下线
	void AppPlayer::onSendLeave(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 memid;
		ts->read(c->ID, memid);
		LOG_MSG("玩家:%d下线\n",memid);

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
