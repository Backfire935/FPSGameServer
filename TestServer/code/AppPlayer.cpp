#include "AppPlayer.h"
#include<list>
#include <time.h>
#include"../../share/ShareFunction.h"

namespace app
{

	IContainer* __AppPlayer = NULL;
	std::map<int, S_PLAYER_BASE*> __Onlines;//��ҵ�½��������
	std::list<S_PLAYER_BASE*> __PlayersPool;//��Ҷ�����ճ�
	u64 temptime = 0;
	u32 __GlobalMemid = 10000000;

	 AppPlayer::AppPlayer()
	{
	}

	 AppPlayer::~AppPlayer()
	 {
	 }

	 void AppPlayer::onInit()
	 {
		
	 }

	 void AppPlayer::onUpdate()
	 {
		 
	 }

	 bool AppPlayer::onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	 {
		 if(ts->isSecure_F_Close(c->ID, func::C_ConnectSecure))
		 {
			LOG_MSG("AppPlayer err... line:%d \n ",__LINE__);
			return  false;
		 }

		 switch (cmd)
		 {
			 case CMD_LOGIN:onLogin(ts, c);break;
			 case CMD_MOVE:onMove(ts, c); break;
			 case CMD_PLAYERDATA:onGetPlayerData(ts, c); break;
		 }
		 return false; 
	 }

	S_PLAYER_BASE* findPlayer(u32 memid, net::S_CLIENT_BASE* c)
	 {
		 auto it = __Onlines.find(memid);//������Ҷ���
		 if (it == __Onlines.end())return nullptr;
		 auto player = it->second;
		if(player->socketfd != c->socketfd)
		{
			LOG_MSG("findplayer err memid:%d c->ID:%d, player->socketfd:%d,c->socketfd:%d... \n", memid, c->ID, player->socketfd,c->socketfd);
			return nullptr;
		}
		return player;
	 }

	//1000 ��½
	 void AppPlayer::onLogin(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	 {

		if(c->state == func::S_Login)
		{
			LOG_MSG("onlogin is login...\n");//�Ѿ���½���˲���Ҫ���µ�½
			return;
		}

		S_PLAYER_BASE* player = nullptr;
		if(__PlayersPool.empty())//�����Ҷ����Ϊ�գ��ʹ���һ���µ���Ҷ���
		{
			player = new S_PLAYER_BASE();
			player->init();
		}
		else//�����Ҷ���ز�Ϊ�գ��ʹ���Ҷ������ȡ��һ����Ҷ���
		{
			player = __PlayersPool.front();
			__PlayersPool.pop_front();
			player->init();//��ʼ����Ҷ���
		}

		srand(time(NULL));

		c->state = func::S_Login;
		player->memid = __GlobalMemid;//��Ҷ����memid
		player->socketfd = c->socketfd;
		player->curhp = 3000;
		player->maxhp = 6000;
		sprintf(player->nick, "nick_%d", player->socketfd);
		__GlobalMemid++;//memid����
		__Onlines.insert(std::make_pair(player->memid, player));//����Ҷ�����뵽�������������)

		ts->begin(c->ID, CMD_LOGIN);
		ts->sss(c->ID, player, sizeof(S_PLAYER_BASE));//����Ҷ����͸��ͻ���
		ts->end(c->ID);

		LOG_MSG("player login successfully... %d-%d\n", player->memid, (int)c->socketfd);
	 }

	 void AppPlayer::onMove(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	 {
		 s32 memid;
		 f32 speed;
		 S_VECTOR pos;
		 S_VECTOR rot;
		 ts->read(c->ID, memid);
		 ts->read(c->ID, speed);
		 ts->read(c->ID, &pos, sizeof(S_VECTOR));
		 ts->read(c->ID, &rot, sizeof(S_VECTOR));

		auto player = findPlayer(memid, c);
		if (player == nullptr) return;

		player->speed = speed;
		player->pos = pos;
		player->rot = rot;

		auto it = __Onlines.begin();
		while(it != __Onlines.end())
		{
			auto other = it->second;
			auto c2 = ts->client((SOCKET)other->socketfd, true);
			if(c2 == nullptr || other->memid == memid)
			{
				++it;
				continue;
			}

			ts->begin(c->ID, CMD_MOVE);
			ts->sss(c->ID, memid);
			ts->sss(c->ID, speed);
			ts->sss(c->ID, &pos, sizeof(S_VECTOR));
			ts->sss(c->ID, &rot, sizeof(S_VECTOR));
			ts->end(c->ID);
			++it;
		}
	 }

	 void AppPlayer::onGetPlayerData(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	 {

	 }
}
