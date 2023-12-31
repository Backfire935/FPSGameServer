#ifndef __TCPCLIENT_H
#define  __TCPCLIENT_H

#include "INetBase.h"
#include<mutex>
#include<thread>

namespace net
{
	class TcpClient: public ITcpClient
	{
	private:
#ifdef ____WIN32_
		SOCKET socketfd;
#else
		int socketfd;
#endif

		S_SERVER_BASE	m_data;
		std::shared_ptr<std::thread> m_workthread;//客户端用一个线程就可以

		TCPCLIENTNOTIFY_EVENT	onAcceptEvent;//接受连接
		TCPCLIENTNOTIFY_EVENT	onSecureEvent;//安全连接
		TCPCLIENTNOTIFY_EVENT	onDisconnectEvent;//失去连接
		TCPCLIENTNOTIFY_EVENT	onExceptEvent;//异常连接

		s32		initSocket();
		bool		setNonblockingSocket();//设置非阻塞socket

		void		connect_Select();//通过select模型连接服务端
		void		onAutoConnect();//失去连接后经过一段时间会尝试自动连接

		int		onRecv();//接收数据
		int		onSend();//发送数据
		int		onSaveData(int recvBytes);//保存数据

		void		onHeart();//客户端会不停的给服务端发送心跳包
		void		parseCommand(u16 cmd);//解析消息体内容
		void		initCommands();//初始化注册消息体

		void		runThread();
		static	void	run(TcpClient* tcp);

	public:
		TcpClient();
		virtual ~TcpClient();
		virtual inline S_SERVER_BASE* getData() { return &m_data; };
#ifdef ____WIN32_
		virtual inline SOCKET getSocket() { return socketfd; };
#else
		virtual inline int getSocket() { return socketfd; };
#endif

		virtual void runClient(u32 sid, char* ip, int port) ;
		virtual bool connectServer() ;
		virtual void disconnectServer(const s32 errcode, const char* err) ;

		virtual void begin(const u16 cmd) ;// 玩家数据索引下标 和 头指令
		virtual void end() ;

		virtual void sss(const s8 v) ;
		virtual void sss(const u8 v) ;

		virtual void sss(const s16 v) ;
		virtual void sss(const u16 v) ;

		virtual void sss(const s32 v) ;
		virtual void sss(const u32 v) ;

		virtual void sss(const s64 v) ;
		virtual void sss(const u64 v) ;

		virtual void sss(const bool v) ;
		virtual void sss(const f32 v) ;
		virtual void sss(const f64 v) ;
		virtual void sss(void* v, const u32 len) ;

		virtual void read(s8& v) ;
		virtual void read(u8& v) ;

		virtual void read(s16& v) ;
		virtual void read(u16& v) ;

		virtual void read(s32& v) ;
		virtual void read(u32& v) ;

		virtual void read(s64& v) ;
		virtual void read(u64& v) ;

		virtual void read(f32& v) ;
		virtual void read(f64& v) ;

		virtual void read(bool& v) ;
		virtual void read(void* v, const u32 len) ;

		virtual void parseCommand() ;
		virtual void registerCommand(int cmd, void* container) ;//注册消息
		virtual void setOnConnect(TCPCLIENTNOTIFY_EVENT event) ;
		virtual void setOnSecure(TCPCLIENTNOTIFY_EVENT event) ;//安全连接
		virtual void setOnDisConnect(TCPCLIENTNOTIFY_EVENT event) ;//失去连接
		virtual void setOnExcept(TCPCLIENTNOTIFY_EVENT event) ;//异常
	};
}


#endif
