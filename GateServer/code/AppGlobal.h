#ifndef ____APPGLOBAL_H
#define ____APPGLOBAL_H

#include "INetBase.h"
#include "IContainer.h"

//业务层
namespace app
{
	extern net::ITcpServer* __TcpServer;
	extern net::ITcpClient* __TcpCenter;
	extern  std::vector<net::ITcpClient*> __TcpGame;

	//接收一个新的连接
	extern void onClientAccept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	//一个安全连接
	extern void onClientSecureConnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	//失去连接
	extern void onClientDisconnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	//超时连接
	extern void onClientTimeout(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	//异常连接
	extern void onClientExcept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);

	//和服务端建立连接
	extern void onConnect(net::ITcpClient* tcp, const s32 code);
	//和服务端安全连接
	extern void onSecureConnect(net::ITcpClient* tcp, const s32 code);
	//和服务端失去连接
	extern void onDisconnect(net::ITcpClient* tcp, const s32 code);
	//异常处理
	extern void onExcept(net::ITcpClient* tcp, const s32 code);
}

#endif
