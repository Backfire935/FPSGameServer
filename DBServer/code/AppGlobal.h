#ifndef ____APPGLOBAL_H
#define ____APPGLOBAL_H

#include "INetBase.h"
#include "IContainer.h"

//业务层
namespace app
{
	extern net::ITcpServer* __TcpServer;

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

}

#endif
