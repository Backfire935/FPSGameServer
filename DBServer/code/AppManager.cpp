#include "AppManager.h"
#include "AppGlobal.h"
#include "../../share/ShareFunction.h"

#include "AppTest.h"
#include "AppPlayer.h"
#include "DBManager.h"
#ifndef  ____WIN32_
#include <unistd.h>
#endif

#define TESTCONNECT 1

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


	//打印信息
	void printInfo()
	{
#ifdef  ____WIN32_
		int tempTime = (int)time(NULL) - temp_time;
		if (tempTime < 1) return;

		int concount = 0;
		int securtiycount = 0;
		__TcpServer->getSecurityCount(concount, securtiycount);


		sprintf_s(printfstr, "connect-%d  security-%d", concount, securtiycount);
		SetWindowTextA(GetConsoleWindow(), printfstr);
#endif
	}
	void onUpdate()
	{
		if (__TcpServer == nullptr) return;

		__TcpServer->parseCommand();
		__AppPlayer->onUpdate();
		__DBManager->update();

		printInfo();

	}

	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		if (func::__ServerListInfo.size() < 1) return;



		//创建服务器 启动
		__TcpServer = net::NewTcpServer();
		__TcpServer->setOnClientAccept(onClientAccept);
		__TcpServer->setOnClientSecureConnect(onClientSecureConnect);
		__TcpServer->setOnClientDisConnect(onClientDisconnect);
		__TcpServer->setOnClientTimeout(onClientTimeout);
		__TcpServer->setOnClientExcept(onClientExcept);
		__TcpServer->runServer(2);

		//__AppTest = new AppTest();
		__AppPlayer = new AppPlayer();
		__TcpServer->registerCommand(CMD_REIGSTER, __AppPlayer);
		__TcpServer->registerCommand(CMD_LOGIN, __AppPlayer);
		__TcpServer->registerCommand(CMD_MOVE, __AppPlayer);
		__TcpServer->registerCommand(CMD_PLAYERDATA, __AppPlayer);
		__TcpServer->registerCommand(9999, __AppPlayer);

		__DBManager = new DBManager();
		__DBManager->InitDB();

		while (true)
		{
			onUpdate();
#ifdef  ____WIN32_
			Sleep(5);
#else
			usleep(2);
#endif
		}

	}

	int run()
	{
		if (__AppManager == nullptr)
		{
			__AppManager = new AppManager();
			__AppManager->init();
		}

		return 0;
	}
}

