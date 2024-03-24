
#include "DBConnetor.h"
namespace db
{
	DBConnetor::DBConnetor(S_DBXML* cfg)
	{
		mysql = nullptr;
		m_mysqlcfg = cfg;
	}

	DBConnetor::~DBConnetor()
	{
	}
	int DBConnetor::Stop()
	{
		return 0;
	}


	int DBConnetor::StartRun(int id,Callback callback, BeginCallback bcallback)
	{
		if (mysql != nullptr) return -1;
		mysql = new MySqlConnetor();
		
		
		m_ThreadID = id;
		m_Callback = callback;
		m_BeginCallback = bcallback;
		

		//1、连接数据库
		int err = mysql->ConnectMySql(m_mysqlcfg->ip,
										m_mysqlcfg->username,
										m_mysqlcfg->userpass,
										m_mysqlcfg->dbname,
										m_mysqlcfg->port) ? 0 : -1;
		if (err != 0) LOG_MSG("mysql start failed...%d:%s-%s\n", id, m_mysqlcfg->ip,m_mysqlcfg->dbname);
		else  LOG_MSG("mysql connect success...%d:%s-%s\n", id, m_mysqlcfg->ip, m_mysqlcfg->dbname);
		//开启线程
		m_BuffsPool.Init(100, 1 * 1024);

		std::thread  th(&DBConnetor::Run, this);
		th.detach();
		return err;
	}


	//把BUFFER推到线程里去
	void DBConnetor::PushToThread(DBBuffer * buff)
	{
		if (buff == nullptr) return;

		std::unique_lock<std::mutex> lock(m_Mutex);
		bool isnotify = m_Queue.empty();
		m_Queue.push(buff);
		lock.unlock();
		
		++m_WorkCount;
		if (isnotify) m_ConditionVar.notify_one();
	}

	DBBuffer * DBConnetor::PopBuffer()
	{
		DBBuffer* buff = m_BuffsPool.Pop();
		buff->Clear();
		buff->SetDB(this);
		return buff;
	}
	//******************************************************************************
	//******************************************************************************
	//******************************************************************************

	//执行线程
	void DBConnetor::Run()
	{
		//Sleep(200);
		//LOG_MSG("mysql thread run..%d.\n",m_ThreadID);
		if (m_BeginCallback != nullptr) m_BeginCallback();

		while (true)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			while (m_Queue.empty())
			{
				m_ConditionVar.wait(lock);
			}
			m_SwapQueue.swap(m_Queue);
			lock.unlock();

			while (!m_SwapQueue.empty())
			{
				DBBuffer* buff = m_SwapQueue.front();//访问第一个元素
				m_SwapQueue.pop();//弹出队列的第一个元素

				m_Callback(buff);
				m_WorkCount--;
				if (buff != nullptr) m_BuffsPool.Push(buff);
			}

		}
	
		LOG_MSG("mysql thread exit...\n");
	}

	

}

