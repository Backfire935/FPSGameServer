
#include "DBManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "GameData.h"
#include "ShareFunction.h"
#include"ErrorCode.h"
namespace db
{

	//**********************************************************************
	//账号数据库
	//200 更新用户登录时间
	void  DBManager::Write_UserLoginTime(DBBuffer* buff, DBConnetor* db)
	{
		app::S_REGISTER_BASE data;
		buff->r(&data, sizeof(app::S_REGISTER_BASE));
		LOG_MSG("db_fd login:%d center_fd:%d gate_fd:%d\n line:%d", data.db_socketfd, data.center_socketfd, data.gate_socketfd, __LINE__);

		s64 memid = data.ID;
		time_t logintime = time(NULL);
		stringstream sql;
		sql << "update user_account set logintime = " << logintime << " where id = " << memid << ";";
		

		auto mysql = db->GetMysqlConnector();
		u64 ftime = clock();
		int ret = mysql->ExecQuery(sql.str());
		if (ret != 0)
		{
			LOG_MSG("DBManager_Write failed:%d %d line:%d\n", mysql->GetErrorCode(), ret, __LINE__);
			return;
		}

		s32 ftimevalue = clock() - ftime;
		LOG_MSG("更新登录时间：[%d毫秒] \n", ftimevalue);
	}
	//100 账号注册
	void  DBManager::Write_UserRegister(DBBuffer* _buff, DBConnetor* db)
	{
		int64_t createtime = time(NULL);
		app::S_REGISTER_BASE data;
		_buff->r(&data, sizeof(app::S_REGISTER_BASE));
		//LOG_MSG("db_fd:%d center_fd:%d gate_fd:%d\n line:%d",data.db_socketfd, data.center_socketfd, data.gate_socketfd,__LINE__);
		stringstream sql;
		sql << "insert user_account(username,password,createtime,logintime) values("
			<< "'" << data.name << "','" << data.password << "'," << createtime << "," << createtime << ");" <<endl;

		auto mysql = db->GetMysqlConnector();
		int64_t ftime = clock();
		int ret = mysql->ExecQuery(sql.str());
		if (ret != 0)
		{
			LOG_MSG("[thread:%d] cmd:200 register error:-%d-%d-%s line:%d\n", db->GetThreadID(), ret, mysql->GetErrorCode(),mysql->GetErrorStr(),  __LINE__);
			DBBuffer* buff = PopPool();
			buff->b(CMD_REIGSTER);
			buff->s(&data, sizeof(app::S_REGISTER_BASE));
			buff->s(app::ErrorCode_Account::EC_FAILED);
			buff->e();
			PushToMainThread(buff);
			return;
		}
		int ftimevalue = clock() - ftime;
		data.ID = mysql->mysql->insert_id;
		
		LOG_MSG("[thread:%d] cmd:200 register success...%ld-%s  time:%d毫秒 \n",db->GetThreadID(), data.ID, data.name, ftimevalue);

		DBBuffer* buff = PopPool();
		buff->b(CMD_REIGSTER);
		buff->s(&data, sizeof(app::S_REGISTER_BASE));
		buff->s(app::ErrorCode_Account::EC_SUCCESS);
		buff->e();
		PushToMainThread(buff);
	}
	//玩家离开 保存数据
	void DBManager::Write_UserSave(DBBuffer* _buff, DBConnetor* db)
	{
		s32 memid;
		_buff->r(memid);
		auto mysql = db->GetMysqlConnector();
		int px =  101;
		int py =  102;
		int pz =  103;
		int rx =  104;
		int ry =  105;
		int rz =  106;
		int curhp = 10000;
		stringstream sql;
		sql << "update user_data set "
			<< "curhp = " << curhp
			<< ",pos_x = " << px
			<< ",pos_y = " << py
			<< ",pos_z = " << pz
			<< ",rot_x = " << rx
			<< ",rot_y = " << ry
			<< ",rot_z = " << rz 
			<< " where memid = " << memid
			<< ";";

		int err = mysql->ExecQuery(sql.str());
		if (err != 0)
		{
			LOG_MSG("[thread:%d]  usersave error:%d-%d-%s line:%d\n", db->GetThreadID(), err, mysql->GetErrorCode(), mysql->GetErrorStr(), __LINE__);

			DBBuffer* buff = PopPool();
			buff->b(CMD_LEAVE);
			buff->s(app::ErrorCode_Account::EC_FAILED);
			buff->s(memid);
			buff->e();
			PushToMainThread(buff);
			return;
		}

		LOG_MSG("[thread:%d]  usersave successfully... line:%d\n", db->GetThreadID(), __LINE__);


		DBBuffer* buff = PopPool();
		buff->b(CMD_LEAVE);
		buff->s(app::ErrorCode_Account::EC_SUCCESS);
		buff->s(memid);
		buff->e();
		PushToMainThread(buff);
	}
	//*******************************************************************
	//*******************************************************************

}