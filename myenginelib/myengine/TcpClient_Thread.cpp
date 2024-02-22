#include "TcpClient.h"

#ifndef ____WIN32_
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace net
{
	void TcpClient::runThread()
	{
		m_workthread.reset(new std::thread(TcpClient::run, this));
		m_workthread->detach();
	}

	void TcpClient::run(TcpClient* tcp)
	{
		auto c = tcp->getData();
		auto socketfd = tcp->getSocket();

		int sleep_time = 10*1000;

#ifdef ____WIN32_
		sleep_time = 20;
#endif
		while (true)
		{
			if(c->state == func::C_Free)//空闲状态
			{
				tcp->onAutoConnect();
				socketfd = tcp->getSocket();
			}
			else if(c->state == func::C_ConnectTry)
			{
				tcp->connectServer();
				socketfd = tcp->getSocket();
			}

			if(c->state < func::C_Connect)
			{
#ifdef ____WIN32_
	Sleep(sleep_time);//以毫秒为单位
#else
				usleep(sleep_time);//以微秒为单位
#endif
				continue;
			}
			//连接成功
			struct timeval tv;//超时结构体
			tv.tv_sec = 5;
			tv.tv_usec = 100;

			fd_set f_read;
			FD_ZERO(&f_read);
			FD_SET(socketfd, &f_read);//把socketfd存在f_read里面
			//
			int errcode = select(socketfd + (u32)1, &f_read, NULL,NULL,&tv);//在Linux下，文件描述符从0开始
			if(errcode > 0)//说明有数据来了
			{
				if(FD_ISSET(socketfd, &f_read))//判断socketfd是不是在f_read里面
				{
					tcp->onRecv();//接收数据成功后保存到缓冲接收区里
				}
			}
			else if(errcode == 0)//超时了 正常情况 什么都不处理
			{
				
			}
			else
			{
#ifdef ____WIN32_
				int err = WSAGetLastError();
				switch (err)
				{
				case WSAEINTR://无效的参数
					break;
				default:
					tcp->disconnectServer(1001, "select -1");
					break;
				}
#else
				switch (errno)
				{
				case EINTR:
					break;
				default:
					tcp->disconnectServer(1001, "select -1");
					break;
				}
#endif
			}
		}
		
		
	}


}
