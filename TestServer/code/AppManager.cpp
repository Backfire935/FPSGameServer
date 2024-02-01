#include "AppManager.h"

#include <ctime>

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

	void printInfo()
	{
		int temptime = (int)time(NULL) - temp_time;
		if(temptime < 1) return;

		int concount = 0;
		int securitycount = 0;
		__TcpServer->getSecurityCount(concount, securitycount);//��ӡ��ȫ��������
		sprintf_s(printfstr, "connect:%d, security:%d", concount, securitycount);
		SetWindowTextA(GetConsoleWindow(),printfstr);
		
		temp_time = (int)time(NULL);
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
		share::InitData();

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
		__TcpServer->runServer(2);//���������߳�

		__AppTest = new AppTest();
		__TcpServer->registerCommand(1000, __AppTest);
		__TcpServer->registerCommand(2000, __AppTest);
		//Sleep(5000);
		//__TcpServer->stopServer();
		
		while(true)
		{
			onUpdate();
			Sleep(5);//2ms����һ��
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
