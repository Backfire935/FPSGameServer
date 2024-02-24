#include "AppPlayer.h"
#include<list>
#include <time.h>
#include"../../share/ShareFunction.h"

namespace app
{

	IContainer* __AppPlayer = NULL;
	std::map<int, S_PLAYER_BASE*> __Onlines;//玩家登陆在线数据
	std::list<S_PLAYER_BASE*> __PlayersPool;//玩家对象回收池
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
		while(it != __Onlines.end())//遍历在线玩家数据
		{
			auto player = it->second;
			auto c = __TcpServer->client((SOCKET)player->socketfd, true);
			if(c == nullptr)
			{
				++it;
				continue;
			}
			if(c->state == func::S_NeedSave)//如果玩家需要保存数据
			{
				playerLeave(player->memid);
				it = __Onlines.erase(it);//从在线玩家数据中删除玩家对象
				player->init();
				__PlayersPool.push_back(player);//将玩家对象放入到玩家对象池中

				c->Reset();
				continue;
			}
			++it;
		}
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
				//返回
				ts->begin(c->ID, 9999);
				ts->sss(c->ID, len);
		 		ts->sss(c->ID, strl, len);
				ts->sss(c->ID, str2, 20);
				ts->end(c->ID);

				delete[] strl;//释放内存
			 }
		 }
		 return false; 
	 }

	S_PLAYER_BASE* findPlayer(u32 memid, net::S_CLIENT_BASE* c)
	 {
		 auto it = __Onlines.find(memid);//查找玩家对象
		 if (it == __Onlines.end())return nullptr;
		 auto player = it->second;
		if(player->socketfd != c->socketfd)
		{
			LOG_MSG("findplayer err memid:%d c->ID:%d, player->socketfd:%d,c->socketfd:%d... \n", memid, c->ID, player->socketfd,c->socketfd);
			return nullptr;
		}
		return player;
	 }

	//1000 登陆
	 void AppPlayer::onLogin(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	 {

		if(c->state == func::S_Login)
		{
			LOG_MSG("onlogin is login...\n");//已经登陆过了不需要重新登陆
			return;
		}

		S_PLAYER_BASE* player = nullptr;
		if(__PlayersPool.empty())//如果玩家对象池为空，就创建一个新的玩家对象
		{
			player = new S_PLAYER_BASE();
			player->init();
		}
		else//如果玩家对象池不为空，就从玩家对象池中取出一个玩家对象
		{
			player = __PlayersPool.front();
			__PlayersPool.pop_front();
			player->init();//初始化玩家对象
		}

		srand(time(NULL));

		c->state = func::S_Login;
		player->memid = __GlobalMemid;//玩家对象的memid
		player->socketfd = c->socketfd;
		player->curhp = 3000;
		player->maxhp = 6000;
		sprintf(player->nick, "nick_%d", player->socketfd);
		__GlobalMemid++;//memid自增
		__Onlines.insert(std::make_pair(player->memid, player));//将玩家对象插入到在线玩家数据中)

		ts->begin(c->ID, CMD_LOGIN);
		ts->sss(c->ID, player, sizeof(S_PLAYER_BASE));//将玩家对象发送给客户端
		ts->end(c->ID);

		LOG_MSG("player login successfully... %d-%d\n", player->memid, (int)c->socketfd);
	 }

	//2000 玩家移动
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

	//获取其他玩家数据
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
