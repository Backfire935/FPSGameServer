 #include "INetBase.h"

#include<time.h>
#include<string>

#ifdef ____WIN32_
#else
#include<limits.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#endif

namespace func
{
	//声明全局变量
	char FileExePath[MAX_EXE_LEN];//存放文件路径
	ConfigXML* __ServerInfo = NULL; //服务端XML指针变量
	ConfigXML* __ClientInfo = NULL; //客户端XML指针变量
	std::vector<ServerListXML*> __ServerListInfo; //需要连接到不同服务器的时候存放到这个数组里，有一组数据就只连接一个服务器，有两组就连接两个
	void(*MD5str) (char* output, unsigned char* input, int Len) = NULL; //MD5编码验证用
	bool InitData() //初始化数据函数
	{
		memset(FileExePath, 0 , MAX_EXE_LEN);
		#ifdef ____WIN32_
			GetModuleFileNameA(NULL,(LPSTR)FileExePath, MAX_EXE_LEN);//获取当前应用程序exe所处的绝对路径
			std::string str(FileExePath);
			size_t pos = str.find_last_of("\\");//为了在路径中去掉exe文件名，先找到文件名的位置
			str = str.substr(0, pos+1);
			memcpy(FileExePath, str.c_str(), MAX_EXE_LEN); //返回当前字符串的首字符地址,string转换成char类型
		#else
			int ret = readlink("/proc/self/exe", FileExePath, MAX_EXE_LEN);
			std::string str(FileExePath);

			size_t pos = str.find_last_of("/");//为了在路径中去掉exe文件名，先找到文件名的位置
			str = str.substr(0, pos+1);
			memcpy(FileExePath, str.c_str(), MAX_EXE_LEN); //返回当前字符串的首字符地址,string转换成char类型
		
		#endif

		LOG_MSG("FileExePath is :%s \n",FileExePath);//最后打印路径
		
		return true;
	}

	u8 GetServerType(s32 sid)
	{
		if(sid >= 1000 && sid<2000) return func::S_TYPE_DB;
		else if (sid >= 2000 &&	sid < 3000) return func::S_TYPE_CENTER;
		else if (sid >= 3000 &&	sid < 4000) return func::S_TYPE_GAME;
		else if (sid >= 4000 &&	sid < 5000) return func::S_TYPE_GATE;
		else if (sid >= 5000 &&	sid < 6000) return func::S_TYPE_LOGIN;
		return func::S_TYPE_USER;
	}

	const char* getShutDownError(int id)
	{
		switch (id)
		{
		case 1001: return "onRecv c ==NULL";
		case 1002: return "onRecv savadata full";
		case 1003: return "postRecv error";
		case 1004: return "postSend error";
		case 1005: return "onSend len != sendBytes";
		case 1006: return "onSend c==NULL";

		case 2001: return "head error";
		case 2002: return "timeout security error";
		case 2003: return "timeout heart error";
		case 2004: return "begin error";
		case 2005: return "end error";
		case 2006: return "closeclient error";

		case 3001: return "iscomplete=false";
		case 3002: return "iscomplete=false overlapped=NULL";
		case 3003: return "recvBytes=0";
		case 3004: return "onaccept";
		case 3005: return "sendBytes=0";

			default:
				return "no";
		}
	}

	 const char*  getCloseSocketError(int id)
	{
		switch (id)
		{
		case 1001: return "postAccept1";
		case 1002: return "postAccept2";
		case 2001: return "closeSocket";
		case 2002: return "closeSocket5";
		case 3001: return "onAccept";
		default:
			return "no";
		}
	}

	void SetConsoleColor(u16 index)
	{
#ifdef ____WIN32_
		//0 黑色 1 蓝色 去网上找找吧都有的
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), index);
#endif
	}
	
}

namespace net
{
	void S_CLIENT_BASE::Init()
	{
		//开辟空间
		recvBuf =	new char[func::__ServerInfo->ReceMax];
		sendBuf	=	new char[func::__ServerInfo->SendMax];
		Reset();
	}

	//重置一下数据
	void S_CLIENT_BASE::Reset()
	{
	#ifdef  ____WIN32_
		socketfd = INVALID_SOCKET;
	#else
		socketfd = -1;
	#endif

	port = 0;
	ID = -1;
	rCode = func::__ServerInfo->RCode;
	//接收数据初始化
	recv_Head =	0;
	recv_Tail =	0;
	recv_TempHead = 0;
	recv_TempTail =	0;
	is_RecvCompleted = false;
		ClientID = 0;
		ClientType = 0;

	//发送数据初始化
	send_Head = 0;
	send_Tail = 0;
	send_TempTail = 0;
	is_Sending = false;
	is_SendCompleted = true;
	//时间类
	threadID = 0;
	time_Connect = (int)time(NULL);
	time_Heart = (int)time(NULL);
	time_Close = (int)time(NULL);

	memset(recvBuf, 0, func::__ServerInfo->ReceMax);
	memset(sendBuf, 0, func::__ServerInfo->SendMax);
	memset(ip, 0, MAX_IP_LEN);

	closeState = 0;
	state = func::S_Free;
	}

	void S_SERVER_BASE::Init(int sid)//连接的时候去连接哪个服务器ID
	{
		ID						= 0;
		serverID			= sid;
		serverType		= func::GetServerType(sid);
		recvBuf				= new char[func::__ClientInfo->ReceMax];
		sendBuf			= new char[func::__ClientInfo->SendMax];
		recvBuf_Temp	= new char[func::__ClientInfo->ReceOne];

		port = 0;
		memset(ip, 0, 16);

		reset();
	}

	void S_SERVER_BASE::reset()
	{
		state						= 0;
		rCode					= func::__ClientInfo->RCode;
		recv_Head			= 0;
		recv_Tail				= 0;
		recv_TempHead	= 0;
		recv_TempTail		= 0;
		is_Recved				= false;

		send_Head			= 0;
		send_Tail				= 0;
		send_TempTail		= 0;
		is_Sending			= false;
		is_SendCompleted = true;

		time_Heart = (int)time(NULL);
		time_AutoConnect = (int)time(NULL);

		memset(recvBuf, 0, func::__ClientInfo->ReceMax);
		memset(sendBuf, 0, func::__ClientInfo->SendMax);
		memset(recvBuf_Temp, 0, func::__ClientInfo->ReceOne);
	}
}


