#include "AppManager.h"

#include <ctime>

#include "AppGlobal.h"
#include "AppPlayer.h"
#include "../../share/ShareFunction.h"
//#include "AppTest.h"

#ifndef ____WIN32_
#include <unistd.h>
#endif

#define TESTCONNECT 1000

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

	void printInfo()
	{
#ifdef ____WIN32_
		int temptime = (int)time(NULL) - temp_time;
		if(temptime < 1) return;
		if (func::__ServerInfo == nullptr) return;
		int concount = 0;
		int securitycount = 0;
		__TcpServer->getSecurityCount(concount, securitycount);//
		sprintf_s(printfstr, "GateServer [%d-%d] connect:%d, security:%d", func::__ServerInfo->ID, func::__ServerInfo->Port, concount, securitycount);
		SetWindowTextA(GetConsoleWindow(),printfstr);
		
		temp_time = (int)time(NULL);
#endif
	}

	void onUpdate()
	{
		if(__TcpServer == nullptr) return;
		__TcpServer->parseCommand();
		__TcpCenter->parseCommand();

		int len = __TcpGame.size();
		for (u32 i = 0; i < len; i++)
			__TcpGame[i]->parseCommand();

		printInfo();
	}
	
	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		if (func::__ServerListInfo.size() < 1) return;

		__TcpServer = net::NewTcpServer();

		__TcpServer->setOnClientAccept(onClientAccept);
		__TcpServer->setOnClientSecureConnect(onClientSecureConnect);
		__TcpServer->setOnClientDisConnect(onClientDisconnect);
		__TcpServer->setOnClientTimeout(onClientTimeout);
		__TcpServer->setOnClientExcept(onClientExcept);
		__TcpServer->runServer(1);//

		
		
		 int len = func::__ServerListInfo.size();
		 __TcpGame.reserve(len);
		 for (u32 i = 0; i < len; i++)
		 {
			 auto xml = func::__ServerListInfo[0];
			 auto client = net::NewTcpClient();
			 client->setOnConnect(onConnect);
			 client->setOnSecure(onSecureConnect);
			 client->setOnDisConnect(onDisconnect);
			 client->setOnExcept(onExcept);

			 __TcpGame.emplace_back(client);
			 client->runClient(xml->ID, xml->IP, xml->Port);
			 client->getData()->ID = i;

			
			 if (i == 0)
			 {
				 __TcpCenter = client;//CenterServer在这里绑定
			 }
		 }

		//__AppTest = new AppTest();
		__AppPlayer = new AppPlayer();
		//服务端注册指令
		__TcpServer->registerCommand(CMD_REIGSTER, __AppPlayer);
		__TcpServer->registerCommand(CMD_LOGIN, __AppPlayer);
		__TcpServer->registerCommand(CMD_PLAYERDATA, __AppPlayer);
		 //客户端注册指令
		for(u32 i = 0; i < len; i++)
		{
			auto client = __TcpGame[0];
			//client->registerCommand(1000, __AppPlayer);
			if( i == 0)//这部分职能只由CenterServer处理
			{
				client->registerCommand(CMD_REIGSTER, __AppPlayer);
				client->registerCommand(CMD_LOGIN, __AppPlayer);
				client->registerCommand(CMD_MOVE, __AppPlayer);
				client->registerCommand(CMD_PLAYERDATA, __AppPlayer);
			}
			else//其他服务器处理,可以在这里注册
			{
				
			}
			//如有两个服务器都需要处理的指令,可以在这里注册


		}
		LOG_MSG("GateServer start ok...%d-%d \n", func
			::__ServerInfo->ID,func::__ServerInfo->Port);

		while(true)
		{
			onUpdate();
#ifdef ____WIN32_
			Sleep(5);//2ms
#else
			usleep(5);
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
