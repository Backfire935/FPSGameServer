#include "AppManager.h"

#include <ctime>

#include "AppGlobal.h"
#include "AppPlayer.h"
#include "../../share/ShareFunction.h"
#include "AppTest.h"

#ifndef ____WIN32_
#include <unistd.h>
#endif

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
		if(func::__ServerInfo == nullptr) return;
		
		int concount = 0;
		int securitycount = 0;
		__TcpServer->getSecurityCount(concount, securitycount);//��ӡ��ȫ��������
		sprintf_s(printfstr, "CenterServer [%d-%d] connect:%d, security:%d", func::__ServerInfo->ID, func::__ServerInfo->Port, concount, securitycount);
		SetWindowTextA(GetConsoleWindow(),printfstr);
		
		temp_time = (int)time(NULL);
#endif
	}

	void onUpdate()
	{
		if(__TcpServer == nullptr) return;
		__TcpServer->parseCommand();
		__TcpDB->parseCommand();
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
		__TcpServer->runServer(1);

		auto xml = func::__ServerListInfo[0];
		 __TcpDB = net::NewTcpClient();
		 __TcpDB->setOnConnect(onConnect);
		 __TcpDB->setOnSecure(onSecureConnect);
		 __TcpDB->setOnDisConnect(onDisconnect);
		 __TcpDB->setOnExcept(onExcept);

		 __TcpDB->runClient(xml->ID, xml->IP, xml->Port);
		 __TcpDB->getData()->ID = 0;

		//__AppTest = new AppTest();
		__AppPlayer = new AppPlayer();
		__TcpServer->registerCommand(CMD_REIGSTER, __AppPlayer);
		__TcpServer->registerCommand(CMD_LOGIN, __AppPlayer);
		__TcpServer->registerCommand(CMD_PLAYERDATA, __AppPlayer);
		__TcpServer->registerCommand(CMD_LEAVE, __AppPlayer);

		__TcpDB->registerCommand(CMD_REIGSTER, __AppPlayer);
		__TcpDB->registerCommand(CMD_LOGIN, __AppPlayer);
		__TcpDB->registerCommand(CMD_MOVE, __AppPlayer);
		__TcpDB->registerCommand(CMD_PLAYERDATA, __AppPlayer);
		//Sleep(5000);
		//__TcpServer->stopServer();
		
		LOG_MSG("CenterServer start ok...%d-%d \n", func
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
