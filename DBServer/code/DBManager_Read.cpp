
#include "DBManager.h"
#include "GameData.h"
#include "ShareFunction.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace db
{
	//一、线程开始 读取全部数据
	//1、全部账号  启动服务器读取 可以直接保存在内存缓冲中 不用推送 其他读取需要推送到主线程 线程安全
	void DBManager::Thread_BeginAccount()
	{
		auto mysql = DBAccount->GetMysqlConnector();

		int64_t ftime = clock();
		stringstream sql;
		sql << "select * from user_account;";
		int error = mysql->ExecQuery(sql.str());
		if (error != 0)
		{
			LOG_MSG("[thread:%d] beginAccount error:%d-%s line:%d\n", DBAccount->GetThreadID(), mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
			return;
		}
		int row = mysql->GetQueryRowNum();
		for (int i = 0; i < row; i++)
		{
			int8_t state = 0;
			app::S_USER_MEMBER_BASE* mem = new app::S_USER_MEMBER_BASE();
			mysql->r("id", mem->ID);
			mysql->r("state", state);
			mysql->r("username", mem->name);
			mysql->r("password", mem->password);
			mysql->r("createtime", mem->timeCreate);
			mysql->r("logintime", mem->timeLastLogin);
			mem->state = (app::E_MEMBER_STATE)state;
			LOG_MSG("[thread:%d-%d] name:%s  timecreate:%d  timelogin:%d  \n", DBAccount->GetThreadID(), (int)mem->ID, mem->name, mem->timeCreate, mem->timeLastLogin);


			std::string name(mem->name);
			app::__AccountsName.insert(std::make_pair(name, mem));
			app::__AccountsID.insert(std::make_pair(mem->ID, mem));
			mysql->QueryNext();
			
		}
		int64_t value = clock() - ftime;
		LOG_MSG("[thread:%d] allmember success...[count:%d]  time：%d毫秒 line:%d\n", DBAccount->GetThreadID(), app::__AccountsName.size(),  value, __LINE__);

	}


	//********************************************************************
	//********************************************************************
	//********************************************************************
	//100 玩家登录获取数据
	void   DBManager::Read_UserLogin(DBBuffer* buff, DBConnetor* db)
	{

		app::S_REGISTER_BASE data;
		buff->r(&data, sizeof(app::S_REGISTER_BASE));

		auto mysql = db->GetMysqlConnector();
		stringstream sql;
		sql << "SELECT * from user_data where user_data.memid = " << data.ID << ";";

		clock_t ftime = clock();
		int error = mysql->ExecQuery(sql.str());
		if (error != 0)
		{
			LOG_MSG("[thread:%d] cmd:100 uselogin error:%d %d-%s\n line:%d", db->GetThreadID(), error, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
	
			return;
		}
		int ftimevalue = clock() - ftime;
		int rownum = mysql->GetQueryRowNum();
		app::S_PLAYER_BASE player;
		memset(&player, 0, sizeof(app::S_REGISTER_BASE));
		player.memid = data.ID;
		player.socketfd = data.db_socketfd;
		
		//没有数据 就创建一条玩家的数据
		if (rownum == 0)
		{
			LOG_MSG("[thread:%d] cmd:100 uselogin row = 0 line:%d \n", db->GetThreadID(), __LINE__);
			sprintf(player.nick, "user_%d", player.memid);
			player.curhp = 3000;
			player.maxhp = 3000;

			stringstream sql2;
			sql2 << "insert user_data(memid,nick,curhp,maxhp) values("
				 << player.memid << ",'" << player.nick << "'," << player.curhp << "," << player.maxhp << ");";

			error = mysql->ExecQuery(sql2.str());
			if (error != 0)
			{
				LOG_MSG("[thread:%d] cmd:100 uselogin error:%d %d-%s\n line:%d \n", db->GetThreadID(), error, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);
				return;
			}
		
		}
		else
		{
			LOG_MSG("[thread:%d] cmd:100 uselogin success：%ld  row:%d time:%d毫秒\n", db->GetThreadID(), data.ID, rownum, ftimevalue, __LINE__);
			
			int value = 0;
			mysql->r("curhp", player.curhp);
			mysql->r("maxhp", player.maxhp);
			mysql->r("nick", player.nick);
			mysql->r("speed", value); player.speed = value / 100;
			mysql->r("pos_x", value); player.pos.x = value / 100;
			mysql->r("pos_y", value); player.pos.y = value / 100;
			mysql->r("pos_z", value); player.pos.z = value / 100;
			mysql->r("rot_x", value); player.rot.x = value / 100;
			mysql->r("rot_y", value); player.rot.y = value / 100;
			mysql->r("rot_z", value); player.rot.z = value / 100;
		}


		//读取角色数据
		DBBuffer* main_buff = PopPool();
		main_buff->b(CMD_LOGIN);
		main_buff->s(&data, sizeof(app::S_REGISTER_BASE));
		main_buff->s(&player, sizeof(app::S_PLAYER_BASE));
		main_buff->e();
		PushToMainThread(main_buff);
		mysql->CloseQuery();
		return;
	}




}