#ifndef ____EPOLLSERVER_H
#define ____EPOLLSERVER_H

#ifndef ____WIN32_

#include "INetBase.h"
#include "IContainer.h"
#include<condition_variable>
#include<thread>
#include<list>
#include<netinet/in.h>


namespace net
{
	class EpollServer : public ITcpServer
	{
	public:
		EpollServer();
		virtual ~EpollServer() {};

	private:
		u32	m_ConnectNum;//连接数
		u32	m_SecurityCount;//安全连接数
		u32	m_ThreadNum;
		u32	m_LinkNum;
		bool	m_IsRunning;

		//条件变量
		std::condition_variable m_AcceptCond;
		std::condition_variable m_RecvCond;

		//互斥量
		std::mutex m_ConnectMutex;
		std::mutex m_SecurityMutex;
		std::mutex m_AcceptMutex;
		std::mutex m_RecvMutex;
		std::mutex m_FindLinkMutex;

		//需要开三个线程
		std::shared_ptr<std::thread> m_ManagerThread;//管理器线程
		std::shared_ptr<std::thread> m_AcceptThread;//新连接线程
		std::shared_ptr<std::thread> m_RecvThread;//接收数据线程
		//游戏服务器人数相比中心服务器连接较少，所以发送数据的部分可以写在主线程或业务线程来处理，而不是单独开一个发送数据线程

		std::list<int>	m_Socketfds;//有新的数据连接的时候，就把socketfd放进这个链表里面
		int					listenfd;
		int					epollfd;
		char* recvBuf[10];//临时中转变量，存储从接收数据的线程中获得的数据
		HashArray<S_CLIENT_BASE>* Linkers;
		HashArray<S_CLIENT_BASE_INDEX>* LinkersIndex;

		TCPSERVERNOTIFY_EVENT onAcceptEvent;
		TCPSERVERNOTIFY_EVENT onSecurityEvent;
		TCPSERVERNOTIFY_EVENT onTimeoutEvent;
		TCPSERVERNOTIFY_EVENT onDisconnectEvent;
		TCPSERVERNOTIFY_EVENT onExceptEvent;

	private:
		int initSocket();
		int add_event(int epollfd, int socketfd, int events);
		int delete_event(int epollfd, int socketfd, int events);


	public:
		inline  void	updateSecurityCount(bool isadd)
		{
			std::lock_guard<std::mutex> guard(m_SecurityMutex);
			if (isadd)m_SecurityCount++;
			else m_SecurityCount--;
		}

		inline  void	updateConnectCount(bool isadd)
		{
			std::lock_guard<std::mutex> guard(m_ConnectMutex);
			if (isadd)m_ConnectNum++;
			else m_ConnectNum--;
		}

		inline S_CLIENT_BASE_INDEX* GetClientIndex(const int socketfd)
		{
			if (socketfd < 0 || socketfd >= MAX_USER_SOCKETFD)return NULL;
			S_CLIENT_BASE_INDEX* c = LinkersIndex->Value(socketfd);
			return c;
		}

	public:
		virtual void runServer(int num) ;//参数是传入的线程数量
		virtual void stopServer() ;

		virtual	S_CLIENT_BASE* client(int fd, bool issecurity) ;//通过socketID先查索引，然后精确定位当前S_CLIENT_BASE的数据,通过索引查找是最快的
		virtual	S_CLIENT_BASE* client(int index) ; //第二种办法，直接传索引值精确定位

		virtual bool isID_T(const s32 id) ;
		virtual bool isSecure_T(const s32 id, s32 secure) ;
		virtual bool isSecure_F_Close(const s32 id, s32 secure) ;//如果不是安全连接就直接关掉
		virtual void parseCommand() ;
		virtual void getSecurityCount(int& connectnum, int& securitycount) ;

		virtual void begin(const int id, const u16 cmd) ;// 玩家数据索引下标 和 头指令
		virtual void end(const int id) ;

		virtual void sss(const int id, const s8 v) ;
		virtual void sss(const int id, const u8 v) ;

		virtual void sss(const int id, const s16 v) ;
		virtual void sss(const int id, const u16 v) ;

		virtual void sss(const int id, const s32 v) ;
		virtual void sss(const int id, const u32 v) ;

		virtual void sss(const int id, const s64 v) ;
		virtual void sss(const int id, const u64 v) ;

		virtual void sss(const int id, const bool v) ;
		virtual void sss(const int id, const f32 v) ;
		virtual void sss(const int id, const f64 v) ;
		virtual void sss(const int id, void* v, const u32 len) ;

		virtual void read(const int id, s8& v) ;
		virtual void read(const int id, u8& v) ;

		virtual void read(const int id, s16& v) ;
		virtual void read(const int id, u16& v) ;

		virtual void read(const int id, s32& v) ;
		virtual void read(const int id, u32& v) ;

		virtual void read(const int id, s64& v) ;
		virtual void read(const int id, u64& v) ;

		virtual void read(const int id, f32& v) ;
		virtual void read(const int id, f64& v) ;

		virtual void read(const int id, bool& v) ;
		virtual void read(const int id, void* v, const u32 len) ;

		virtual void setOnClientAccept(TCPSERVERNOTIFY_EVENT event) ;
		virtual void setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event) ;//安全连接
		virtual void setOnClientDisConnect(TCPSERVERNOTIFY_EVENT event) ;//失去连接
		virtual void setOnClientTimeout(TCPSERVERNOTIFY_EVENT event) ;//超时连接
		virtual void setOnClientExcept(TCPSERVERNOTIFY_EVENT event) ;//异常
		virtual void registerCommand(int cmd, void* container) ;//注册消息


	};

	
}

#endif


#endif

