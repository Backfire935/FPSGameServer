#include	"TcpServer.h"

#ifdef ____WIN32_
#include "IOPool.h"
using namespace func;
namespace net
{
	//运行线程
	void TcpServer::runThread(int num)
	{
		m_IsRunning = true;
		m_ThreadNum = num;
		if (m_ThreadNum > 10) m_ThreadNum = 10;

		//初始化线程
		for(int i = 0; i<m_ThreadNum; i++)
		{
			m_workthread[i].reset(new std::thread(TcpServer::run, this ,i) );
		}
		//detach的含义是主线程和子线程相互分离，互不干扰
		for(int i = 0;i<m_ThreadNum;i++)
		{
			m_workthread[i]->detach();
		}

	}

	

	//用完后送回对象回收池
	void pushContext(IOContext* context)
	{
		switch (context->m_Mode)
		{
		case func::SC_WAIT_ACCEPT:
			AcceptContext::push((AcceptContext*)context);
			
			break;
		case func::SC_WAIT_RECV:
			RecvContext::push((RecvContext*)context);
			break;
		case func::SC_WAIT_SEND:
			SendContext::push((SendContext*)context);
			break;
		}
	}

	void TcpServer::run(TcpServer* tcp, int id)
	{
		LOG_MSG("run workthread ...%d\n", id);
		ULONG_PTR key = 1;
		OVERLAPPED* overlapped = nullptr;
		DWORD recvBytes = 0;

		while(tcp->m_IsRunning)
		{
			bool iscomplete = GetQueuedCompletionStatus(tcp->getCompletePort(), &recvBytes , &key, &overlapped , INFINITE);
			//BOOL WINAPI GetQueuedCompletionStatus(
		//	__in          HANDLE CompletionPort,   //完成端口内核对象的句柄
		//	__out         LPDWORD lpNumberOfBytes, //发送或者接收字节数
		//	__out         PULONG_PTR lpCompletionKey, //自定义的数据结构指针
		//	__out         LPOVERLAPPED * lpOverlapped, //重叠数据结构
		//	__in          DWORD dwMilliseconds  //超时时间
			IOContext* context = CONTAINING_RECORD(overlapped, IOContext, m_OverLapped);//通过重叠结构精确查找实例地址
			// addr: 结构体中某个成员变量的地址
			// type: 结构体的原型 
			// field: 结构体的某个成员
			//这个宏的作用是：根据一个结构体实例中的成员的地址，取到整个结构体实例的地址
			if(context == nullptr) continue;
			if(iscomplete == false)//说明掉线了或者有人出错了
			{
				DWORD dwErr = GetLastError();
				if(dwErr == WAIT_TIMEOUT) continue;//超时
				if(overlapped != NULL)
				{
					//掉线，玩家主动关闭，后面在这添加玩家离线的处理
					tcp->shutDown(context->m_Socket, context->m_Mode, NULL, 3001);
					pushContext(context);
					continue;
				}
				
				tcp->shutDown(context->m_Socket, context->m_Mode, NULL, 3002);
				pushContext(context);
				continue;
			}
			else
			{
				if(overlapped == NULL)
				{
					LOG_MSG("overlapped == NULL in line %d \n", __LINE__);
					break;
				}
				if(key != 0)
				{
					LOG_MSG("overlapped == NULL in line %d \n", __LINE__);
					continue;
				}

				if((recvBytes == 0) && (context->m_Mode == func::SC_WAIT_RECV || context->m_Mode == func::SC_WAIT_SEND))
				{
					tcp->shutDown(context->m_Socket, context->m_Mode, NULL ,3003);
					pushContext(context);//扔回对象池
					continue;
				}

				switch (context->m_Mode)
				{
				case func::SC_WAIT_ACCEPT:
					{
						auto acc = (AcceptContext*)context;
						int err = tcp->onAccept(acc);
						if(err != 0)//不是0说明有错误发生
						{
							tcp->closeSocket(acc->m_Socket, NULL , 3004);
							AcceptContext::push(acc);//回收到对象池
							tcp->postAccept();//产生新的投递
						}
					}
					break;
				case func::SC_WAIT_RECV:
					tcp->onRecv(context, (int)recvBytes, id);
					break;
				case func::SC_WAIT_SEND:
					tcp->onSend(context, (int)recvBytes);
					break;
				}

			}
		}
		
		LOG_MSG("exit workthread.. %d\n", id);
	}
}

#endif
