#ifndef  ____DBMANAGER_H
#define  ____DBMANAGER_H

#include <concurrent_queue.h>

#include "DBConnetor.h"
#include "DBBuffer.h"
namespace db
{
	//数据库管理器
	class DBManager
	{
	public:
		DBManager();
		~DBManager();
	public:

		S_DBXML*  __GameDBXML;
		S_DBXML*  __AccountXML;

		 
		Concurrency::concurrent_queue<DBBuffer*>  __logicBuffs;//安全的并发队列
		ThreadSafePool  __poolBuffs; //buff回收缓存池


		//mysql操作
		std::vector<DBConnetor* > DBRead;//玩家读取数据库操作
		std::vector<DBConnetor* > DBWrite;//玩家写数据库操作
		DBConnetor *DBAccount;//玩家账号全局数据库操作


		DBConnetor* GetDBSource(int type);
		DBBuffer* PopPool();
		int     LoadDBXML(const char* filename);
		void    InitDB();
		void    update();
		void    PushToMainThread(DBBuffer * buffer);
		
	
		void	Thread_BeginAccount();
		void	Thread_UserRead(DBBuffer* buff);
		void	Thread_UserWrite(DBBuffer* buff);
		void	Thread_UserAccount(DBBuffer* buff);
		

		void    Read_UserLogin(DBBuffer* buff,DBConnetor* db);

		void    Write_UserLoginTime(DBBuffer* buff, DBConnetor* db);
		void    Write_UserRegister(DBBuffer* _buff, DBConnetor* db);
		void    Write_UserSave(DBBuffer* _buff, DBConnetor* db);

	};
	extern DBManager* __DBManager;
	
}
#endif