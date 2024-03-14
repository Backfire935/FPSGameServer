#include "AppManager.h"

#ifndef  ____WIN32_
#include <ctime>
#include <unistd.h>
#endif

#include "AppGlobal.h"
#include "../../share/ShareFunction.h"
#include "AppTest.h"
namespace app
{
	AppManager* __AppManager = nullptr;
	int temp_time = 0;
	char printfstr[1000];
	
	AppManager::AppManager()
	{
	}

	AppManager::~AppManager()
	{
		
	}



	void onUpdate()
	{
		if (__AppTest == nullptr) return;
		for (u32 i = 0; i < TESTCONNECT; i++)
			__TcpGame[i]->parseCommand();

		__AppTest->onUpdate();
	}
	
	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		if (func::__ServerListInfo.size() < 1)return;


		__AppTest = new AppTest();
		auto xml = func::__ServerListInfo[0];
		__TcpGame.reserve(TESTCONNECT);
		for (u32 i = 0; i< TESTCONNECT; i++)
		{
			auto client = net::NewTcpClient();
			client->setOnConnect(onConnect);
			client->setOnSecure(onSecureConnect);
			client->setOnDisConnect(onDisconnect);	
			client->setOnExcept(onExcept);

			__TcpGame.emplace_back(client);
			client->runClient(xml->ID, xml->IP,xml->Port);
			client->getData()->ID = i;

			//¿Í»§¶Ë×¢²áÖ¸Áî
			client->registerCommand(1000, __AppTest);
			//client->registerCommand(2000, __AppTest);
		}

		
		while(true)
		{
			onUpdate();
#ifdef ____WIN32_
			Sleep(5);
#else
			usleep(2);
#endif
		}
	}

	int run()
	{
		if(__AppManager == nullptr)
		{
			__AppManager = new AppManager();
			__AppManager->init();
		}
		return 0;
	}
}
