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
			int num = epoll_wait(epoll->epollfd, events, 2048, -1);//����2048���¼�
			if( num == -1)//����
			{
				if(errno == EINTR) continue;//���ֲ��ù�,����û���꣬�������
				LOG_MSG("return run manager...code:%d\n", errno);
				break;
			}

			for(int i = 0; i< num; i++)
			{
				int socketfd = events[i].data.fd;

				//������µ�����
				if( socketfd == epoll->listenfd)
				{
					epoll->m_AcceptCond.notify_one();//֪ͨ����һ���߳�

				}
				//���µ����ݵ���
				else if(events[i].events & EPOLLIN)//�ж��ǲ��ǿɶ��¼�
				{
					{
						std::unique_lock<std::mutex> gurad(epoll->m_RecvMutex);
						epoll->m_Socketfds.push_back(socketfd);
					}
					epoll->m_RecvCond.notify_one();//֪ͨ�����µ�����
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
				epoll->m_AcceptCond.wait(gurad);//����
			}
			//���մ����µ�����

		}
	}

	void EpollServer::run_recv(EpollServer* epoll, int tid)
	{
		int socketfd = -1;

		while (epoll->m_IsRunning)
		{
			{
				std::unique_lock<std::mutex> gurad(epoll->m_RecvMutex);
				while(epoll->m_Socketfds.empty())//ֻ��������Ϊ�յ�����²���������ֹ��ٻ��ѵ��º��浯����
				{
					epoll->m_RecvCond.wait(gurad);//����
				}
				socketfd = epoll->m_Socketfds.front();//��ȡֵ
				epoll->m_Socketfds.pop_front();//���������ݾ�����
			}
			//���մ����µ�����

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