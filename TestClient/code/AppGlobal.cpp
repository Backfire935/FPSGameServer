#include "AppGlobal.h"

namespace app
{
	net::ITcpServer*  __TcpServer = nullptr;
	std::vector<net::ITcpClient*> __TcpGame;

	//工作线程中调用的
	void onClientAccept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)return;
		LOG_MSG("new connect...%d [%s:%d] %d\n", (int)c->socketfd, c->ip, c->port, c->threadID);

	}
	//主线程 也就是业务线程中调用
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

	//主线程
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

		//如果当前的连接是安全的
		if(c->state == func::S_Connect || c->state == func::S_ConnectSecure)
		{
			//初始化一下
			c->Reset();
		}
	}

	//只会在工作线程触发超时
	void onClientTimeout(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)return;
	}

	void onClientExcept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)return;
	}

	//客户端连接事件
	void onConnect(net::ITcpClient* tcp, const s32 code)
	{
		LOG_MSG("----------client  connect... 代码:%d ID:%d \n", code, tcp->getData()->ID);
	}

	//安全连接的时候
	void onSecureConnect(net::ITcpClient* tcp, const s32 code)
	{
		LOG_MSG("----------client security connect... 代码:%d ID:%d \n",code, tcp->getData()->ID );
	}

	void onDisconnect(net::ITcpClient* tcp, const s32 code)
	{
		LOG_MSG("----------client onDisconnect... 代码:%d \n", code);
	}

	void onExcept(net::ITcpClient* tcp, const s32 code)
	{
		LOG_MSG("----------client onExcept... %d \n", code);
	}
}


