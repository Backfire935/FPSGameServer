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
		if(func::__ServerInfo == nullptr) return;
		int concount = 0;
		int securitycount = 0;
		__TcpServer->getSecurityCount(concount, securitycount);//��ӡ��ȫ��������
		sprintf_s(printfstr, "DBServer [%d-%d] connect:%d, security:%d", func::__ServerInfo->ID, func::__ServerInfo->Port,concount, securitycount);
		SetWindowTextA(GetConsoleWindow(),printfstr);
		
		temp_time = (int)time(NULL);
#endif
	}

	void onUpdate()
	{
		if(__TcpServer == nullptr) return;
		//��ͣ�ļ�����ӣ�����ָ��,��ͣ��Ͷ������
		__TcpServer->parseCommand();
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

		__AppTest = new AppTest();

		//Sleep(5000);
		//__TcpServer->stopServer();
		
		LOG_MSG("DBServer start ok...%d-%d \n", func
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
