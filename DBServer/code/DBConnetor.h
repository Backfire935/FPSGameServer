#ifndef  ____MYSQLMANAGER_H
#define  ____MYSQLMANAGER_H
#include <string>
#include <functional>
#include <atomic>
#include <queue>
#include <mutex> //互斥锁
#include <thread> 

#include "DBBuffer.h"
#include "ThreadSafeQueue.h"
#include "MySqlConnetor.h"



namespace db
{
	typedef std::function<void(DBBuffer*)> Callback;
	typedef std::function<void()> BeginCallback;

	struct S_DBXML
	{
		int	    port;//端口号
		char	ip[20];//IP
		char	username[20];//数据库账号
		char	userpass[20];//数据库密码
		char	dbname[20];//数据库名字

		S_DBXML() { memset(this, 0, sizeof(*this)); }
	};
	//开启线程类型
	enum E_THREAD_TYPE
	{
		ETT_USERREAD  = 0x01,
		ETT_USERWRITE  = 0x02,
		ETT_ACCOUNT   = 0x03
	};

	class DBConnetor
	{
	public:
		DBConnetor(S_DBXML* cfg);
		~DBConnetor();
	private:
		MySqlConnetor *mysql;//mysql连接器
		S_DBXML*	   m_mysqlcfg;//数据库配置表

		//线程相关数据
		int       m_ThreadID;//线程ID
		Callback  m_Callback;//回调函数
		BeginCallback  m_BeginCallback;//回调函数

		std::mutex	m_Mutex;
		std::queue<DBBuffer*> m_Queue;//共享数据队列
		std::queue<DBBuffer*> m_SwapQueue;//交换数据队列
		std::condition_variable m_ConditionVar;//条件变量
		ThreadSafePool  m_BuffsPool;//BUFF池
		atomic<int>  m_WorkCount;

	public:
		int					Stop();
		int					StartRun(int id,  Callback callback, BeginCallback bcallback = nullptr);
		DBBuffer*        PopBuffer();
		void PushToThread(DBBuffer* buffer);
		inline MySqlConnetor*	GetMysqlConnector() { return mysql; }
		inline int GetThreadID() { return m_ThreadID; }
		inline int	GetWorkCount()const { return m_WorkCount; }
		void Run();
	};

	
}


#endif