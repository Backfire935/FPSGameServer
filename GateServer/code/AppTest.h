#ifndef ____APPTEST_H
#define ____APPTEST_H

#include "AppGlobal.h"

namespace app
{
	//ֻ�м̳���Container�������ɷ���Ϣ
	class AppTest : public IContainer
	{
	public:
		AppTest();
		~AppTest();
		virtual void onInit();
		virtual void onUpdate();
		virtual bool onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);

	};

	extern IContainer* __AppTest;//ȫ�ֱ���


}

#endif