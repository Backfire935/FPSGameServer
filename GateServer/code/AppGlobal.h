#ifndef ____APPGLOBAL_H
#define ____APPGLOBAL_H

#include "INetBase.h"
#include "IContainer.h"

//ҵ���
namespace app
{
	extern net::ITcpServer* __TcpServer;
	extern net::ITcpClient* __TcpCenter;
	extern  std::vector<net::ITcpClient*> __TcpGame;

	//����һ���µ�����
	extern void onClientAccept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	//һ����ȫ����
	extern void onClientSecureConnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	//ʧȥ����
	extern void onClientDisconnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	//��ʱ����
	extern void onClientTimeout(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	//�쳣����
	extern void onClientExcept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);

	//�ͷ���˽�������
	extern void onConnect(net::ITcpClient* tcp, const s32 code);
	//�ͷ���˰�ȫ����
	extern void onSecureConnect(net::ITcpClient* tcp, const s32 code);
	//�ͷ����ʧȥ����
	extern void onDisconnect(net::ITcpClient* tcp, const s32 code);
	//�쳣����
	extern void onExcept(net::ITcpClient* tcp, const s32 code);
}

#endif
