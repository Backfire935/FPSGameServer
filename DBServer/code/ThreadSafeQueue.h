#pragma once
#include <concurrent_queue.h>
#include "DBBuffer.h"
#include <atomic>

using namespace std;
using namespace db;
//线程安全池模板类
class ThreadSafePool 
{
private:
	Concurrency::concurrent_queue<DBBuffer*> buffs;//安全的并发队列
	int		m_BufferSize;//
	int   m_MaxCount;
	atomic_int  m_Count;//总的大小
public:
	ThreadSafePool() {}
	~ThreadSafePool()
	{
		while (!buffs.empty())
		{
			DBBuffer* buff = NULL;
			buffs.try_pop(buff);
			if (NULL != buff)
			{
				delete buff;
			}
		}
	}

	void Init(int num,int size)
	{
		m_Count = 0;
		m_MaxCount = num;
		m_BufferSize = size;
	}
	int	GetCount() 
	{
		return m_Count;
	}
	DBBuffer* Pop()
	{
		DBBuffer* buff = nullptr;
		if (buffs.empty() == true)
		{
			buff = new DBBuffer(m_BufferSize);
			buff->Clear();
		}
		else
		{
			buffs.try_pop(buff);
			if (buff == nullptr)
			{
				buff = new DBBuffer(m_BufferSize);
				buff->Clear();
			}
			else
			{
				m_Count--;
			}
		}

		return buff;
	}

	void Push(DBBuffer* buff)
	{
		if (buff == nullptr) return;
		if (m_Count > m_MaxCount)
		{
			delete buff;
			return;
		}
		buff->Clear();
		buffs.push(buff);
		m_Count++;
	}

	void Clear() 
	{
	}
};