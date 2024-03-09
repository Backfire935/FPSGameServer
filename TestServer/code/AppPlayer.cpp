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


	void playerLeave(u32 memid)
	 {
		auto it = __Onlines.begin();
		while(it != __Onlines.end())
		{
			auto player = it->second;
			auto c = __TcpServer->client((SOCKET)player->socketfd, true);
			if(c == nullptr || player->memid == memid)
			{
				++it;
				continue;
			}
			__TcpServer->begin(c->ID, CMD_LEAVE);
			__TcpServer->sss(c->ID, memid);
			__TcpServer->end(c->ID);
			++it;
		}
	 }

	 void AppPlayer::onUpdate()
	 {
		 int  value = clock() - temptime;
		 if (value < 100) return;
		temptime = clock();

		auto it = __Onlines.begin();
		while(it != __Onlines.end())//���������������
		{
			auto player = it->second;
			auto c = __TcpServer->client((SOCKET)player->socketfd, true);
			if(c == nullptr)
			{
				++it;
				continue;
			}
			if(c->state == func::S_NeedSave)//��������Ҫ��������
			{
				playerLeave(player->memid);
				it = __Onlines.erase(it);//���������������ɾ����Ҷ���
				player->init();
				__PlayersPool.push_back(player);//����Ҷ�����뵽��Ҷ������

				c->Reset();
				continue;
			}
			++it;
		}
	 }

	 bool AppPlayer::onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	 {
	 	//��ҿͻ��˸����ӷ���˵�ʱ����Ϊ��û�кͷ���˽�����ȫ���ӣ������ڷ���˼�һ���жϣ��Ƿ��ǵ�½�Ȳ�������Ȼ�ͻ���Ϊ���Ӳ���ȫ��ֱ�ӹر�����
	 	if(cmd == CMD_LOGIN || cmd == CMD_MOVE || cmd == CMD_PLAYERDATA)
	 	{
	 		
	 	}
		 else
			if(ts->isSecure_F_Close(c->ID, func::C_ConnectSecure) )
		 {
			LOG_MSG("AppPlayer err... line:%d \n ",__LINE__);
			return  false;
		 }

		 switch (cmd)
		 {
			 case CMD_LOGIN:onLogin(ts, c);break;
			 case CMD_MOVE:onMove(ts, c); break;
			 case CMD_PLAYERDATA:onGetPlayerData(ts, c); break;

		 case 9999:
			 {
			 int len = 0;
			 char* strl = NULL;
			 char str2[20];
			 memset(str2, 0, 20);

			   ts->read(c->ID, len);
		 		strl = new char[len];
				ts->read(c->ID, strl, len);

				ts->read(c->ID, str2, 20);
				//����
				ts->begin(c->ID, 9999);
				ts->sss(c->ID, len);
		 		ts->sss(c->ID, strl, len);
				ts->sss(c->ID, str2, 20);
				ts->end(c->ID);

				delete[] strl;//�ͷ��ڴ�
			 }
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

	//2000 ����ƶ�
	 void AppPlayer::onMove(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	 {
		 s32 memid;
		 f32 speed;
		 f32 temp1,temp2 = 0;

		 S_VECTOR pos;
		 S_VECTOR rot;
		 ts->read(c->ID, memid);
		 ts->read(c->ID, speed);
		 ts->read(c->ID, &pos, sizeof(S_VECTOR));
		 ts->read(c->ID, &rot, sizeof(S_VECTOR));

		 //temp1 = rot.x;
		 //temp2 = rot.y;
		 //rot.x = rot.z;
		 //rot.y= temp1;
	 	// rot.z = temp2;

		 printf("rot %.2f %.2f %.2f\n", rot.x, rot.y, rot.z);
		S_PLAYER_BASE* player = findPlayer(memid, c);
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

			ts->begin(c2->ID, CMD_MOVE);
			ts->sss(c2->ID, memid);
			ts->sss(c2->ID, speed);
			ts->sss(c2->ID, &pos, sizeof(S_VECTOR));
			ts->sss(c2->ID, &rot, sizeof(S_VECTOR));
			ts->end(c2->ID);
			++it;
		}
	 }

	//��ȡ�����������
	 void AppPlayer::onGetPlayerData(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	 {
		 s32 memid;
		 ts->read(c->ID, memid);

		 auto it = __Onlines.find(memid);
		 if (it == __Onlines.end()) return;

		 auto player = it->second;
		 auto c2 = ts->client((SOCKET)player->socketfd, true);
		 if (c2 == nullptr) return;

		 ts->begin( c->ID, CMD_PLAYERDATA );
		 ts->sss(c->ID, player ,sizeof(S_PLAYER_BASE));
		 ts->end(c->ID);

	 }
}
