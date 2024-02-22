#include "IOPool.h"

#ifdef ____WIN32_
#include<concurrent_queue.h>//安全队列
Concurrency::concurrent_queue<AcceptContext*> __accepts;
Concurrency::concurrent_queue<RecvContext*> __recvs;
Concurrency::concurrent_queue<SendContext*> __sends;
//第二章第二节
IOContext::IOContext()
{
	m_Socket = INVALID_SOCKET;
	m_Mode	 = 0;
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

IOContext::~IOContext()
{
	//因为是对象回收池，没有什么传值，就不需要写什么了
}

//******************************
//新的连接
AcceptContext::AcceptContext(int mode, SOCKET lisfd, SOCKET sfd)
{
	m_Mode		= mode;
	listenfd	= lisfd;
	m_Socket	= sfd;

	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

AcceptContext::~AcceptContext(void)
{
	listenfd	= NULL;
	m_Socket	= NULL;	
	m_Mode		= 0;
	
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

void AcceptContext::clear()
{
	listenfd	= NULL;
	m_Socket	= NULL;	
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

void AcceptContext::setSocket(SOCKET lisfd, SOCKET sfd)
{
	listenfd	= lisfd;
	m_Socket	= sfd;
}

AcceptContext* AcceptContext::pop()//获取一个对象池的对象
{
	AcceptContext* buff = nullptr;
	if(__accepts.empty() == true)//对象池没有空闲对象
	{
		//没有对象就new一个对象
		buff = new AcceptContext(func::SC_WAIT_ACCEPT, NULL, NULL);
	}
	else
	{
		__accepts.try_pop(buff);//如果有对象的话就尝试弹出一个对象到buff里
		if( buff == NULL)
		{
			buff = new AcceptContext(func::SC_WAIT_ACCEPT, NULL, NULL);
		}
	}
	return buff;
}

void AcceptContext::push(AcceptContext* acc)//把对象放回对象池
{
	if(acc == NULL) return;
	if(__accepts.unsafe_size() > POOL_SIZE) //容器太多的话
	{
		delete acc;
		return;
	}
	//小于这个数量的话，先清理数据,再放入池子
	acc->clear();
	__accepts.push(acc);
}

int AcceptContext::getCount()
{
	return 	__accepts.unsafe_size();
}

//*******************************************
RecvContext::RecvContext(int mode)
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= mode;
	m_Buffs = new char[func::__ServerInfo->ReceOne];//开辟内存空间
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));//清理重叠结构
	ZeroMemory(m_Buffs, func::__ServerInfo->ReceOne);//清理重叠结构
	m_wsaBuf.buf = m_Buffs;//关联，有数据就会填充到这里来
	m_wsaBuf.len = func::__ServerInfo->ReceOne;//设置长度
	
}

RecvContext::~RecvContext()
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= -1;
	
}

void RecvContext::clear()
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= -1;

	ZeroMemory(m_Buffs, func::__ServerInfo->ReceOne);//清理重叠结构
	m_wsaBuf.buf = m_Buffs;//关联，有数据就会填充到这里来
	m_wsaBuf.len = func::__ServerInfo->ReceOne;//设置长度
}

RecvContext* RecvContext::pop()
{
	RecvContext* buff = nullptr;
	if(__recvs.empty() == true)//对象池没有空闲对象
		{
			//没有对象就new一个对象
			buff = new RecvContext(func::SC_WAIT_RECV);
		}
	else
	{
		__recvs.try_pop(buff);//如果有对象的话就尝试弹出一个对象到buff里
		if( buff == NULL)
		{
			buff = new RecvContext(func::SC_WAIT_RECV);
		}
	}
	return buff;
}

void RecvContext::push(RecvContext* acc)
{
	if(acc == NULL) return;
	if(__recvs.unsafe_size() > POOL_SIZE) //容器太多的话
		{
			delete acc;
			return;
		}
	//小于这个数量的话，先清理数据,再放入池子
	acc->clear();
	__recvs.push(acc);
}

int RecvContext::getCount()
{
	return __recvs.unsafe_size(); 
}

//********************************************
SendContext::SendContext(int mode)
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= mode;
	
	m_Buffs = new char[func::__ServerInfo->SendOne];//开辟内存空间
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));//清理重叠结构
	ZeroMemory(m_Buffs, func::__ServerInfo->SendOne);
	m_wsaBuf.buf = m_Buffs;//关联，有数据就会填充到这里来
	m_wsaBuf.len = func::__ServerInfo->SendOne;//设置长度
}

SendContext::~SendContext()
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= -1;
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));//清理重叠结构
}

void SendContext::clear()
{
	m_Socket	= INVALID_SOCKET;
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));//清理重叠结构
	ZeroMemory(m_Buffs, func::__ServerInfo->SendOne);
	m_wsaBuf.buf = m_Buffs;//关联，有数据就会填充到这里来
	m_wsaBuf.len = func::__ServerInfo->SendOne;//设置长度
}



SendContext* SendContext::pop()
{
	SendContext* buff = nullptr;
	if(__sends.empty() == true)//对象池没有空闲对象
		{
		//没有对象就new一个对象
		buff = new SendContext(func::SC_WAIT_SEND);
		}
	else
	{
		__sends.try_pop(buff);//如果有对象的话就尝试弹出一个对象到buff里
		if( buff == NULL)
		{
			buff = new SendContext(func::SC_WAIT_SEND);
		}
	}
	return buff;
}

void SendContext::push(SendContext* acc)
{
	if(acc == NULL) return;
	if(__sends.unsafe_size() > POOL_SIZE) //容器太多的话
		{
		delete acc;
		return;
		}
	//小于这个数量的话，先清理数据,再放入池子
	acc->clear();
	__sends.push(acc);
}

int SendContext::getCount()
{
	return __sends.unsafe_size();
}

//发送信息的函数
int SendContext::setSend(SOCKET fd, char* data, int sendBytes)
{
	m_Socket = fd;
	if(&m_wsaBuf)
	{
		if(sendBytes != 0 && data != NULL)
		{
			memcpy(m_Buffs, data, sendBytes);//这个时候就把内容复制过去了
			m_wsaBuf.buf = m_Buffs;//至于什么时候发送就看操作系统了
			m_wsaBuf.len = sendBytes;
		}
	}

	return sendBytes;
}

#endif
