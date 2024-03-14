#ifndef ____APPTEST_H
#define ____APPTEST_H

#include "AppGlobal.h"

namespace app
{
	//只有继承自Container，才能派发消息
	class AppTest : public IContainer
	{
	public:
		AppTest();
		~AppTest();
		virtual void onInit();
		virtual void onUpdate();
		virtual bool onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool onClientCommand(net::ITcpClient* tc,  const u16 cmd);
	};

	extern IContainer* __AppTest;//全局变量


}

#endif
