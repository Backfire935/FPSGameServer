#ifndef __TCPSERVER_H
#define __TCPSERVER_H

#ifdef ____WIN32_

#include "INetBase.h"
#include "IContainer.h"
#include <mutex>
#include <thread>
#include <MSWSock.h>
#pragma comment(lib, "mswsock")


namespace net
{
	class TcpServer : public ITcpServer
	{
	public:
		TcpServer();
		virtual ~TcpServer();

	private:
		//定义基础数据
		s32	m_ConnectCount;//当前连接数
		s32	m_SecurityCount;//安全连接数
		bool	m_IsRunning;		//服务器是否正在运行
		s32	m_ThreadNum;	//线程数量
		SOCKET	listenfd;		//监听套接字句柄
		//定义IOCP数据
		HANDLE	m_CompletePort;	//完成端口句柄
		LPFN_ACCEPTEX	m_AcceptEx;//AcceptEx函数地址
		LPFN_GETACCEPTEXSOCKADDRS	m_GetAcceptEx;//获取客户端信息函数地址
		//定义工作线程
		std::shared_ptr<std::thread> m_workthread[10];//共享指针工作线程
		//定义互斥
		std::mutex	m_findlink_mutex;
		std::mutex	m_ConnectMutex;
		std::mutex	m_SecurityMutex;
		//连接玩家容器
		HashArray<S_CLIENT_BASE>* Linkers;//连接的玩家
		HashArray<S_CLIENT_BASE_INDEX>* LinkerIndexs;//连接玩家索引数组
		//函数指针托管
		TCPSERVERNOTIFY_EVENT	onAcceptEvent;
		TCPSERVERNOTIFY_EVENT	onSecureEvent;
		TCPSERVERNOTIFY_EVENT	onTimeOutEvent;
		TCPSERVERNOTIFY_EVENT	onDisconnectEvent;
		TCPSERVERNOTIFY_EVENT	onExceptEvent;

	private:

		s32 postAccept();
		s32 onAccept(void* context);

		s32 postRecv(SOCKET s);
		s32 onRecv(void * context, s32 recvBytes, u32 tid);//tid表示是哪个线程进来的
		s32 onRecv_SavaData(S_CLIENT_BASE* c, char* buf, s32 recvBytes);

		s32 postSend(S_CLIENT_BASE* c);
		s32 onSend(void *context, s32 sendBytes);
		
		s32	closeSocket(SOCKET sfd, S_CLIENT_BASE* c, int kind);//关闭释放socket
		void	shutDown(SOCKET sfd, s32 mode, S_CLIENT_BASE* c, int kind); //关闭socket读写端的方法，不接受数据，不读数据
		int setHeartCheck(SOCKET s);
		S_CLIENT_BASE* getFreeLinker();
		
		//定义一些私有api
		inline HANDLE getCompletePort() { return m_CompletePort; }
		inline void	updateSecurityConnect(	bool isadd)
		{
			//上锁保护
			std::lock_guard<std::mutex>	guard(this->m_SecurityMutex);
			if (isadd) m_SecurityCount++;
			else m_SecurityCount--;
		}

		inline void	updateConnect(bool isadd)
		{
			//上锁保护
			std::lock_guard<std::mutex>	guard(this->m_ConnectMutex);
			if (isadd) m_ConnectCount++;
			else m_ConnectCount--;
		}

		inline S_CLIENT_BASE_INDEX* getClientIndex(int sfd)
		{
			if (sfd < 0 || sfd > MAX_USER_SOCKETFD) return nullptr;
			auto c = LinkerIndexs->Value(sfd);
			return c;
		}

	private:
		s32 initSocket();
		void initPost();
		//初始化消息
		void initCommands();
		
		void runThread(int num);
		void parseCommand(S_CLIENT_BASE* c);
		void parseCommand(S_CLIENT_BASE* c, u16 cmd);
		void checkConnect(S_CLIENT_BASE* c);
		
		static void run(TcpServer* tcp, int id);//自定义的线程ID

	public:
		virtual	void runServer(int num);
		virtual void stopServer();
		virtual	S_CLIENT_BASE* client(SOCKET socketfd, bool issecurity);//通过socketID先查索引，然后精确定位当前S_CLIENT_BASE的数据,通过索引查找是最快的
		virtual	S_CLIENT_BASE* client(const int id); //第二种办法，直接传索引值精确定位
		virtual bool isID_T(const s32 id);
		virtual bool isSecure_T(const s32 id, s32 secure);
		virtual bool isSecure_F_Close(const s32 id, s32 secure);//如果不是安全连接就直接关掉
		virtual void parseCommand();
		virtual void getSecurityCount(int& connectnum,int& securitycount);
		
		virtual void begin(const int id, const u16 cmd);// 玩家数据索引下标 和 头指令
		virtual void end(const int id);

		//商用的话消息体部分也需要异或处理
		virtual void sss(const int id, const s8 v) ;
		virtual void sss(const int id, const u8 v);
		
		virtual void sss(const int id, const s16 v);
		virtual void sss(const int id, const u16 v) ;
		
		virtual void sss(const int id, const s32 v) ;
		virtual void sss(const int id, const u32 v) ;

		virtual void sss(const int id, const s64 v) ;
		virtual void sss(const int id, const u64 v) ;

		virtual void sss(const int id, const bool v) ;
		virtual void sss(const int id, const f32 v) ;
		virtual void sss(const int id, const f64 v) ;
		virtual void sss(const int id, void* v, const u32 len) ;

		virtual void read(const int id, s8& v);
		virtual void read(const int id, u8& v);

		virtual void read(const int id, s16& v);
		virtual void read(const int id, u16& v);

		virtual void read(const int id, s32& v);
		virtual void read(const int id, u32& v);

		virtual void read(const int id, s64& v);
		virtual void read(const int id, u64& v);

		virtual void read(const int id, f32& v);
		virtual void read(const int id, f64& v);

		virtual void read(const int id, bool& v);
		virtual void read(const int id, void* v, const u32 len);

		virtual void setOnClientAccept(TCPSERVERNOTIFY_EVENT event);
		virtual void setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event);//安全连接
		virtual void setOnClientDisConnect(TCPSERVERNOTIFY_EVENT event);//失去连接
		virtual void setOnClientTimeout(TCPSERVERNOTIFY_EVENT event);//超时连接
		virtual void setOnClientExcept(TCPSERVERNOTIFY_EVENT event);//异常
		virtual void registerCommand(int cmd, void* container);//注册消息
		
	};



}

#endif


#endif
