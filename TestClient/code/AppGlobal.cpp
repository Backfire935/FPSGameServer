#include "AppGlobal.h"

namespace app
{
	net::ITcpServer*  __TcpServer = nullptr;
	std::vector<net::ITcpClient*> __TcpGame;

	//�����߳��е��õ�
	void onClientAccept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)return;
		LOG_MSG("new connect...%d [%s:%d] %d\n", (int)c->socketfd, c->ip, c->port, c->threadID);

	}
	//���߳� Ҳ����ҵ���߳��е���
	void onClientSecureConnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)return;
		int aa = 0;
		int bb = 0;
		tcp->getSecurityCount( aa, bb );
		func::SetConsoleColor(10);
		LOG_MSG("security connect...%d [%s:%d] [connect:%d-%d]\n", (int)c->socketfd, c->ip, c->port, aa, bb);
		func::SetConsoleColor(7);
	}

	//���߳�
	void onClientDisconnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)return;
		int aa = 0;
		int bb = 0;
		tcp->getSecurityCount( aa, bb );
		const char* str1 = func::getShutDownError(c->shutdown_kind);
		const char* str2 = func::getCloseSocketError(code);

		func::SetConsoleColor(8);
		LOG_MSG("disconnect...%d [%s:%d] [connect:%d-%d]\n", (int)c->socketfd, str1, str2, aa, bb);
		func::SetConsoleColor(7);

		//�����ǰ�������ǰ�ȫ��
		if(c->state == func::S_Connect || c->state == func::S_ConnectSecure)
		{
			//��ʼ��һ��
			c->Reset();
		}
	}

	//ֻ���ڹ����̴߳�����ʱ
	void onClientTimeout(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)return;
	}

	void onClientExcept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)return;
	}

	//�ͻ��������¼�
	void onConnect(net::ITcpClient* tcp, const s32 code)
	{
		LOG_MSG("----------client  connect... ����:%d ID:%d \n", code, tcp->getData()->ID);
	}

	//��ȫ���ӵ�ʱ��
	void onSecureConnect(net::ITcpClient* tcp, const s32 code)
	{
		LOG_MSG("----------client security connect... ����:%d ID:%d \n",code, tcp->getData()->ID );
	}

	void onDisconnect(net::ITcpClient* tcp, const s32 code)
	{
		LOG_MSG("----------client onDisconnect... ����:%d \n", code);
	}

	void onExcept(net::ITcpClient* tcp, const s32 code)
	{
		LOG_MSG("----------client onExcept... %d \n", code);
	}
}


