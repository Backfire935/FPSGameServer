#include "AppGlobal.h"

namespace app
{
	net::ITcpServer*  __TcpServer = nullptr;
	
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
		LOG_MSG("disconnect...%d [%s:%s] [connect:%d-%d]\n", (int)c->socketfd, str1, str2, aa, bb);
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
		LOG_MSG("onClientTimeout:%d\r\n", code);

	}

	void onClientExcept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)return;
		LOG_MSG("onClientExcept:%d\r\n", code);

	}
}


