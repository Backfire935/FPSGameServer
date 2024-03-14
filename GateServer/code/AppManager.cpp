#include "AppManager.h"

#include <ctime>

#include "AppGlobal.h"
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
		if (func::__ServerInfo == nullptr) return;
		int concount = 0;
		int securitycount = 0;
		__TcpServer->getSecurityCount(concount, securitycount);//��ӡ��ȫ��������
		sprintf_s(printfstr, "GateServer [%d-%d] connect:%d, security:%d", func::__ServerInfo->ID, func::__ServerInfo->Port, concount, securitycount);
		SetWindowTextA(GetConsoleWindow(),printfstr);
		
		temp_time = (int)time(NULL);
#endif
	}

	void onUpdate()
	{
		if(__TcpServer == nullptr) return;
		//��ͣ�ļ�����ӣ�����ָ��,��ͣ��Ͷ������
		__TcpServer->parseCommand();
		//__TcpCenter->parseCommand();

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

		//�����µ�����
		__TcpServer->setOnClientAccept(onClientAccept);
		//���ð�ȫ����
		__TcpServer->setOnClientSecureConnect(onClientSecureConnect);
		//����ʧȥ����
		__TcpServer->setOnClientDisConnect(onClientDisconnect);
		//���ó�ʱ����
		__TcpServer->setOnClientTimeout(onClientTimeout);
		//�����쳣����
		__TcpServer->setOnClientExcept(onClientExcept);
		//���з����
		__TcpServer->runServer(1);//���������߳�

		
		
		 int len = func::__ServerListInfo.size();
		 __TcpGame.reserve(len);
		 for (u32 i = 0; i < len; i++)
		 {
			 auto xml = func::__ServerListInfo[i];
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
				 __TcpCenter = client;
			 }
		 }

		__AppTest = new AppTest();
		//服务端注册指令
		__TcpServer->registerCommand(1000, __AppTest);
		 //客户端注册指令
		for(u32 i = 0; i < len; i++)
		{
			auto client = __TcpGame[i];
			client->registerCommand(1000, __AppTest);
		}
		LOG_MSG("GateServer start ok...%d-%d \n", func
			::__ServerInfo->ID,func::__ServerInfo->Port);

		while(true)
		{
			onUpdate();
#ifdef ____WIN32_
			Sleep(5);//2ms����һ��
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
