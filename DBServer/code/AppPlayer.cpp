#include "AppPlayer.h"
#include <list>
#include <time.h>
#include "../../share/ShareFunction.h"
#include "GameData.h"
#include "DBManager.h"
#include "ErrorCode.h"

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
	}

	void playerLeave(u32 memid)
	{
		auto it = __Onlines.begin();
		while (it != __Onlines.end())
		{
			auto player = it->second;
#ifdef  ____WIN32_
			auto c = __TcpServer->client((SOCKET)player->socketfd, true);
#else
			auto c = __TcpServer->client(player->socketfd, true);
#endif
			if (c == nullptr || player->memid == memid)
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
		int value = clock() - temptime;
		if (value < 100) return;
		temptime = clock();

		auto it = __Onlines.begin();
		while (it != __Onlines.end())
		{
			auto player = it->second;
#ifdef  ____WIN32_
			auto c = __TcpServer->client((SOCKET)player->socketfd, true);
#else
			auto c = __TcpServer->client(player->socketfd, true);
#endif

			if (c == nullptr)
			{
				++it;
				continue;
			}
			if (c->state == func::S_NeedSave)
			{
				//写数据库
				auto db = __DBManager->GetDBSource(ETT_USERWRITE);
				auto buff = db->PopBuffer();
				buff->b(CMD_LEAVE);
				buff->s(player, sizeof(S_PLAYER_BASE));
				buff->e();
				db->PushToThread(buff);

				c->state = func::S_Saving;
				playerLeave(player->memid);
			}

			++it;
		}
	}

	bool app::AppPlayer::onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
	{
		if (ts->isSecure_F_Close(c->ID, func::S_ConnectSecure))
		{
			LOG_MSG("AppPlayer err....line:%d \n", __LINE__);
			return false;
		}

		switch (cmd)
		{
		case CMD_REIGSTER:onReigster(ts, c); break;
		case CMD_LOGIN:onLogin(ts, c); break;
		case CMD_MOVE:onMove(ts, c); break;
		case CMD_PLAYERDATA:onGetPlayerData(ts, c); break;
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
				//返回
				ts->begin(c->ID, 9999);
				ts->sss(c->ID, len);
				ts->sss(c->ID, str1, len);
				ts->sss(c->ID, str2, 20);
				ts->end(c->ID);

				delete[] str1;

			}
			break;
		}

		return false;
	}



	S_PLAYER_BASE* findPlayer(u32 memid, net::S_CLIENT_BASE* c)
	{
		auto it = __Onlines.find(memid);
		if (it == __Onlines.end()) return NULL;
		auto player = it->second;
		if (player->socketfd != c->socketfd)
		{
			LOG_MSG("findplayer err...%d-%d-%d-%d \n", memid, c->ID, player->socketfd, c->socketfd);
			return NULL;
		}
		return player;
	}
	S_PLAYER_BASE* findPlayer(int memid)
	{
		auto it = __Onlines.find(memid);
		if (it == __Onlines.end()) return NULL;
		return it->second;
	}
	//100 注册
	void AppPlayer::onReigster(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		S_REGISTER_BASE registerData;
		ts->read(c->ID, &registerData, sizeof(S_REGISTER_BASE));
		registerData.db_socketfd = c->socketfd;
		registerData.db_port = c->port;
		//LOG_MSG("AppPlayer db_fd:%d center_fd:%d gate_fd:%d\n %d", registerData.db_socketfd, registerData.center_socketfd, registerData.gate_socketfd,__LINE__);

		//1、查找账号  账号已经存在
		std::string fname(registerData.name);
		auto mem = app::FindMember(fname);
		if (mem != nullptr)
		{
			ts->begin(c->ID, CMD_REIGSTER);
			ts->sss(c->ID, &registerData, sizeof(S_REGISTER_BASE));
			ts->sss(c->ID, app::ErrorCode_Account::EC_FAILED);
			ts->end(c->ID);
			return;
		}

		//向数据库申请数据 获取玩家游戏数据
		auto db = __DBManager->DBAccount;
		auto buff = db->PopBuffer();
		buff->b(CMD_REIGSTER);
		buff->s(&registerData, sizeof(S_REGISTER_BASE));
		buff->e();
		db->PushToThread(buff);
	}
	//1000 登陆
	void AppPlayer::onLogin(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		if (c->state == func::S_Login)
		{
			LOG_MSG("onlogin is login....\n");
			return;
		}

		S_REGISTER_BASE loginData;
		ts->read(c->ID, &loginData, sizeof(S_REGISTER_BASE));
		loginData.db_socketfd = c->socketfd;
		loginData.db_port = c->port;

		//1、查找账号  账号不存在
		std::string fname(loginData.name);
		auto mem = app::FindMember(fname);
		if (mem == nullptr)
		{
			LOG_MSG("name is %s 账号不存在\n", loginData.name);
			ts->begin(c->ID, CMD_LOGIN);
			ts->sss(c->ID, EC_ACCOUNT_NOT_EXIST);
			ts->sss(c->ID, &loginData, sizeof(S_REGISTER_BASE));
			ts->end(c->ID);
			return;
		}
		else
		{
			if (mem->state >= M_LOGIN)//账号已经登录
			{
				ts->begin(c->ID, CMD_LOGIN);
				ts->sss(c->ID, EC_ACCOUNT_LOGINED);
				ts->sss(c->ID, &loginData, sizeof(S_REGISTER_BASE));
				ts->end(c->ID);
				return;
			}

		}
		//2、密码不正确
		int value = strcmp(loginData.password, mem->password);
		LOG_MSG("name is %s password is %s %s %d\n",loginData.name, loginData.password, mem->password, value);
		if(value != 0)
		{
			ts->begin(c->ID, CMD_LOGIN);
			ts->sss(c->ID, EC_PASSWORD_ERROR);
			ts->sss(c->ID, &loginData, sizeof(S_REGISTER_BASE));
			ts->end(c->ID);
			return;
		}
		//3、查找玩家在线不？防止挤号
		auto player = findPlayer(mem->ID);
		if (player != nullptr)
		{
			ts->begin(c->ID, CMD_LOGIN);
			ts->sss(c->ID, &loginData, sizeof(S_REGISTER_BASE));
			ts->sss(c->ID, EC_ACCOUNT_LOGIN_OTHER);
			ts->end(c->ID);
			return;
		}
		
		//向数据库申请数据 获取玩家游戏数据
		loginData.ID = mem->ID;
		auto db = __DBManager->GetDBSource(ETT_USERREAD);
		auto buff = db->PopBuffer();
		buff->b(CMD_LOGIN);
		buff->s(&loginData, sizeof(S_REGISTER_BASE));
		buff->e();
		db->PushToThread(buff);

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
		ts->read(c->ID, &pos,sizeof(S_VECTOR));
		ts->read(c->ID, &rot, sizeof(S_VECTOR));

		auto player = findPlayer(memid,c);
		if (player == NULL) return;

		player->speed = speed;
		player->pos = pos;
		player->rot = rot;

		auto it = __Onlines.begin();
		while (it != __Onlines.end())
		{
			auto other = it->second;

#ifdef  ____WIN32_
			auto c2 = ts->client((SOCKET)other->socketfd, true);
#else
			auto c2 = ts->client(other->socketfd, true);
#endif

			if (c2 == nullptr || other->memid == memid)
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
	//3000 获取其他玩家数据
	void AppPlayer::onGetPlayerData(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
	{
		s32 memid;
		ts->read(c->ID, memid);
		auto it = __Onlines.find(memid);
		if (it == __Onlines.end()) return;

		auto player = it->second;
#ifdef  ____WIN32_
		auto c2 = ts->client((SOCKET)player->socketfd, true);
#else
		auto c2 = ts->client(player->socketfd, true);
#endif
		if (c2 == NULL) return;


		ts->begin(c->ID, CMD_PLAYERDATA);
		ts->sss(c->ID, player, sizeof(S_PLAYER_BASE));
		ts->end(c->ID);

	}



	//*****************************************************************
	//100 db注册成功
	void OnDB_ReigsterAccount(DBBuffer* buf)
	{
		app::S_REGISTER_BASE data;
		buf->r(&data, sizeof(app::S_REGISTER_BASE));

		auto c = __TcpServer->client((SOCKET)data.db_socketfd, true);
		if (c == nullptr)
		{
			LOG_MSG("reg err...%d \n", data.db_socketfd);
			return;
		}

		app::S_USER_MEMBER_BASE* mem = new app::S_USER_MEMBER_BASE();
		mem->ID = data.ID;
		memcpy(mem->name, data.name, USER_MAX_MEMBER);
		memcpy(mem->password, data.password, USER_MAX_MEMBER);
		mem->timeCreate    = (int)time(NULL);
		mem->timeLastLogin = (int)time(NULL);

		std::string fname(data.name);
		app::__AccountsName.insert(std::make_pair(fname, mem));
		app::__AccountsID.insert(std::make_pair(mem->ID, mem));

		//返回给客户端 注册成功消息
		__TcpServer->begin(c->ID, CMD_REIGSTER);
		__TcpServer->sss(c->ID, &data, sizeof(app::S_REGISTER_BASE));
		__TcpServer->sss(c->ID, app::ErrorCode_Account::EC_ACCOUNT_ACTIVE);
		__TcpServer->end(c->ID);
	}
	//200 db登录返回
	void OnDB_Login(DBBuffer* buf)
	{
		app::S_REGISTER_BASE logindata;
		S_PLAYER_BASE player_data;
		buf->r(&logindata, sizeof(app::S_REGISTER_BASE));
		buf->r(&player_data, sizeof(app::S_PLAYER_BASE));


		auto c = __TcpServer->client((SOCKET)logindata.db_socketfd, true);
		if (c == nullptr)
		{
			LOG_MSG("OnDB_Login err...%d line:%d\n", logindata.db_socketfd,__LINE__);
			return;
		}
		auto mem = app::FindMember(logindata.ID);
		if (mem == nullptr)
		{
			LOG_MSG("OnDB_Login err...%d line:%d\n", logindata.db_socketfd, __LINE__);
			return;
		}
		mem->timeLastLogin = (int)time(NULL);
		mem->state = M_LOGIN;
		//更新登录时间
		auto db = __DBManager->DBAccount;
		auto buff = db->PopBuffer();
		buff->b(CMD_UPDATELOGIN);
		//buff->s(player_data.memid);
		buff->s(&logindata, sizeof(app::S_REGISTER_BASE));
		buff->e();
		db->PushToThread(buff);
		//**************************************************************

		S_PLAYER_BASE* player = NULL;
		if (__PlayersPool.empty())
		{
			player = new S_PLAYER_BASE();
			player->init();
		}
		else
		{
			player = __PlayersPool.front();
			__PlayersPool.pop_front();
			player->init();
		}

		c->state = func::S_Login;
		memcpy(player, &player_data, sizeof(S_PLAYER_BASE));
		__Onlines.insert(std::make_pair(player->memid, player));

		__TcpServer->begin(c->ID, CMD_LOGIN);
		__TcpServer->sss(c->ID, EC_ACCOUNT_LOGINED);
		__TcpServer->sss(c->ID, &logindata, sizeof(S_REGISTER_BASE));
		//__TcpServer->sss(c->ID, &player_data, sizeof(S_PLAYER_BASE));//等客户端单独拉返回玩家数据
		__TcpServer->end(c->ID);

		LOG_MSG("player login successfully...%d-%d\n", player->memid, (int)c->socketfd);
	}

	//4000 玩家离开
	void OnDB_Leave(DBBuffer* buf)
	{
		int err = 0;
		app::S_PLAYER_BASE player;

		buf->r(err);
		buf->r(&player, sizeof(app::S_PLAYER_BASE));


		//错误处理？？？
		if (err != 0)
		{
			LOG_MSG("OnDB_Leave err...%d line:%d\n", player.socketfd, __LINE__);
		}

		//1、清理连接数据
		auto c = __TcpServer->client((SOCKET)player.socketfd, true);
		if (c == nullptr)
		{
			LOG_MSG("OnDB_Leave err...%d line:%d\n", player.socketfd, __LINE__);
			return;
		}
		c->Reset();

		//2、清理账号状态
		auto mem = app::FindMember(player.memid);
		if (mem == nullptr) 
		{
			LOG_MSG("OnDB_Leave err...%d line:%d\n", player.socketfd, __LINE__);
			return;
		}
		mem->state = M_FREE;
	
		//3、清理玩家数据
		auto it = __Onlines.find(player.memid);
		if (it == __Onlines.end())
		{
			LOG_MSG("OnDB_Leave err...%d line:%d\n", player.memid, __LINE__);
			return;
		}

		auto playdata = it->second;
		__Onlines.erase(it);
		playdata->init();
		__PlayersPool.push_back(playdata);
		LOG_MSG("player leave success...%d-%d\n", err,player.memid);
	}
	//DB数据库返回
	bool AppPlayer::OnDBCommand(void* buff)
	{
		DBBuffer* buf = (DBBuffer*)buff;
		if (buf == nullptr) return false;
		u16 cmd;
		buf->init_r();
		buf->r(cmd);

		switch (cmd)
		{
		case CMD_REIGSTER:OnDB_ReigsterAccount(buf); break;
		case CMD_LOGIN:OnDB_Login(buf); break;
		case CMD_LEAVE:OnDB_Leave(buf); break;
		}
		return true;
	}
}
