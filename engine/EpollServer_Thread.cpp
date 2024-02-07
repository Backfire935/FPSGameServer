#include"EpollServer.h"

#ifndef ____WIN32_
#include <sys/socket.h>
#include <sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>


namespace net
{
	void EpollServer::run_manager(EpollServer* epoll)
	{
		struct epoll_event events[2048];

		for(; ; )
		{
			int num = epoll_wait(epoll->epollfd, events, 2048, -1);//创建2048个事件
			if( num == -1)//出错
			{
				if(errno == EINTR) continue;//这种不用管,数据没收完，正常情况
				LOG_MSG("return run manager...code:%d\n", errno);
				break;
			}

			for(int i = 0; i< num; i++)
			{
				int socketfd = events[i].data.fd;

				//如果有新的连接
				if( socketfd == epoll->listenfd)
				{
					epoll->m_AcceptCond.notify_one();//通知唤醒一个线程

				}
				//有新的数据到了
				else if(events[i].events & EPOLLIN)//判断是不是可读事件
				{
					{
						std::unique_lock<std::mutex> gurad(epoll->m_RecvMutex);
						epoll->m_Socketfds.push_back(socketfd);
					}
					epoll->m_RecvCond.notify_one();//通知接收新的数据
				}

			}

		}
	}

	void EpollServer::run_accept(EpollServer* epoll, int tid)
	{
		while(epoll->m_IsRunning)
		{
			{
				std::unique_lock<std::mutex> gurad(epoll->m_AcceptMutex);
				epoll->m_AcceptCond.wait(gurad);//阻塞
			}
			//接收处理新的连接

		}
	}

	void EpollServer::run_recv(EpollServer* epoll, int tid)
	{
		int socketfd = -1;

		while (epoll->m_IsRunning)
		{
			{
				std::unique_lock<std::mutex> gurad(epoll->m_RecvMutex);
				while(epoll->m_Socketfds.empty())//只有在数据为空的情况下才阻塞，防止虚假唤醒导致后面弹出空
				{
					epoll->m_RecvCond.wait(gurad);//阻塞
				}
				socketfd = epoll->m_Socketfds.front();//获取值
				epoll->m_Socketfds.pop_front();//消费完数据就销毁
			}
			//接收处理新的数据

		}
	}

	void EpollServer::runThread(int num)
	{
		m_IsRunning = true;
		m_ManagerThread.reset(new std::thread(EpollServer::run_manager, this ));
		m_AcceptThread.reset(new std::thread(EpollServer::run_accept, this ));
		m_RecvThread.reset(new std::thread(EpollServer::run_recv, this ));

		m_ManagerThread->detach();
		m_AcceptThread->detach();
		m_RecvThread->detach();
	}
}


#endif