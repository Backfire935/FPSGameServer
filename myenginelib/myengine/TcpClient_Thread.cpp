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
			if(c->state == func::C_Free)//����״̬
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
	Sleep(sleep_time);//�Ժ���Ϊ��λ
#else
				usleep(sleep_time);//��΢��Ϊ��λ
#endif
				continue;
			}
			//���ӳɹ�
			struct timeval tv;//��ʱ�ṹ��
			tv.tv_sec = 5;
			tv.tv_usec = 100;

			fd_set f_read;
			FD_ZERO(&f_read);
			FD_SET(socketfd, &f_read);//��socketfd����f_read����
			//
			int errcode = select(socketfd + (u32)1, &f_read, NULL,NULL,&tv);//��Linux�£��ļ���������0��ʼ
			if(errcode > 0)//˵������������
			{
				if(FD_ISSET(socketfd, &f_read))//�ж�socketfd�ǲ�����f_read����
				{
					tcp->onRecv();//�������ݳɹ��󱣴浽�����������
				}
			}
			else if(errcode == 0)//��ʱ�� ������� ʲô��������
			{
				
			}
			else
			{
#ifdef ____WIN32_
				int err = WSAGetLastError();
				switch (err)
				{
				case WSAEINTR://��Ч�Ĳ���
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
