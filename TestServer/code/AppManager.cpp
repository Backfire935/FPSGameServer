#include "AppManager.h"

#include <ctime>

#include "AppGlobal.h"
#include "../../share/ShareFunction.h"
#include "AppTest.h"
#include "AppPlayer.h"
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

		int concount = 0;
		int securitycount = 0;
		__TcpServer->getSecurityCount(concount, securitycount);//打印安全连接数量
		sprintf_s(printfstr, "connect:%d, security:%d", concount, securitycount);
		SetWindowTextA(GetConsoleWindow(),printfstr);
		
		temp_time = (int)time(NULL);
#endif
	}

	void onUpdate()
	{
		if(__TcpServer == nullptr) return;
		//不停的检查连接，解析指令,不停的投递数据
		__TcpServer->parseCommand();
		printInfo();
	}
	
	void AppManager::init()
	{
		bool isload = share::InitData();
		if (!isload) return;
		if (func::__ServerListInfo.size() < 1) return;

		__TcpServer = net::NewTcpServer();

		//设置新的连接
		__TcpServer->setOnClientAccept(onClientAccept);
		//设置安全连接
		__TcpServer->setOnClientSecureConnect(onClientSecureConnect);
		//设置失去连接
		__TcpServer->setOnClientDisConnect(onClientDisconnect);
		//设置超时连接
		__TcpServer->setOnClientTimeout(onClientTimeout);
		//设置异常连接
		__TcpServer->setOnClientExcept(onClientExcept);
		//运行服务端
		__TcpServer->runServer(2);//开辟两个线程

		__AppTest = new AppTest();
		__AppPlayer = new AppPlayer();
		__TcpServer->registerCommand(CMD_LOGIN, __AppPlayer);
		__TcpServer->registerCommand(CMD_MOVE, __AppPlayer);
		__TcpServer->registerCommand(CMD_PLAYERDATA, __AppPlayer);
		//Sleep(5000);
		//__TcpServer->stopServer();
		
		while(true)
		{
			onUpdate();
#ifdef ____WIN32_
			Sleep(5);//2ms休眠一次
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
