#ifndef __INETBASE_H
#define  __INETBASE_H

#include "IDefine.h"

namespace net
{
	class ITcpClient;
#pragma pack(push, packing)
	#pragma pack(1)

	//客户端连接数据索引
	struct S_CLIENT_BASE_INDEX
	{
		s32	index;	//记住玩家连接数据的下标
		inline void Reset() { index = -1; }
	};

	//客户端连接数据
	struct S_CLIENT_BASE
	{
	#ifdef ____WIN32_	//支持跨平台
		SOCKET socketfd;
	#else
		int	socketfd;
	#endif

		s8	state; //有符号一字节，记住当前连接状态
		s8	closeState; // 记录关闭状态
		char	ip[MAX_IP_LEN];
		u16	port;
		s32	ID;//当前玩家连接数据ID，也是索引下标
		u8	 rCode;//对数据进行异或，防止知道数据明文是什么,起到混淆的作用
		s32 memid;//在DB返回或者客户端申请登录的时候，存入memid，仅在gate这一级有用,因为只有gate的client才有memid

	//游戏纪录数据 
		s32	ClientID;//各个功能服务器ID
		u8	 ClientType;//0 1-DB 2-Center 3-Game 4-Gate 5-Login
	
		//生产者--接收数据
		//消费者--解析数据
		bool		is_RecvCompleted;//数据是否接收完成
		char*		recvBuf;//存放接收到的数据，读取到的客户端的数据，存放进，开辟的空间大小是初始化时ConfigXML的ReceMax值
		//char*		recvBuf_Temp;//存放一次最大允许接收的数据的大小，大小是ConfigXML的ReceOne
		s32			recv_Head;//接收数据头的偏移量 生产者
		s32			recv_Tail;//接收数据尾的偏移量 消费者
		s32			recv_TempHead;//临时头
		s32			recv_TempTail;//临时尾

		//生产者--封包
		//消费者--发送数据
		bool		is_Sending;//是否正在发送数据封包
		bool		is_SendCompleted;//是否发送完毕
		char*		sendBuf;//发送缓冲区,大小是ConfigXML的SendMax
		s32			send_Head;//发送数据头的偏移量 消费者 滑动窗口
		s32			send_Tail;//发送数据尾的偏移量 生产者
		s32			send_TempTail;//

		//时间类
		s32			time_Connect;//存放连接时间
		s32			time_Heart;//记录心跳时间
		s32			time_Close;//关闭时间
		u8			threadID;//线程ID
		s32			shutdown_kind;//安全关闭服务器，打印输出信息
		char		md5[MAX_MD5_LEN];//MD5码安全验证

		void	Init();
		void	Reset();
	#ifdef ____WIN32_
		inline  bool isT(SOCKET _fd)
		{
			if (socketfd == _fd) return true;	
			return false;
		}
	#else // ____WIN32_
		inline bool isT(int fd)
		{
			if (socketfd == fd) return true;
			return false;
		}
	#endif
	};

	//连接服务器数据结构
	struct S_SERVER_BASE
	{
#ifdef ____WIN32_
		SOCKET	socketfd;
#else
		int			socketfd;
#endif
		s32			ID;
		char			ip[16];
		u16			port;
		s32			serverID;
		u8			serverType;

		u8			state;
		u8			rCode;

		//生产者--接收数据
	//消费者--解析数据
		char*		recvBuf;//存放接收到的数据，读取到的客户端的数据，存放进，开辟的空间大小是初始化时ConfigXML的ReceMax值
		char*		recvBuf_Temp;//存放一次最大允许接收的数据的大小，大小是ConfigXML的ReceOne
		s32			recv_Head;//接收数据头的偏移量 生产者
		s32			recv_Tail;//接收数据尾的偏移量 消费者
		s32			recv_TempHead;//临时头
		s32			recv_TempTail;//临时尾
		bool			is_Recved;//数据是否接收完成

		//生产者--封包
	//消费者--发送数据
		char*		sendBuf;//发送缓冲区,大小是ConfigXML的SendMax
		s32			send_Head;//发送数据头的偏移量 消费者 滑动窗口
		s32			send_Tail;//发送数据尾的偏移量 生产者
		s32			send_TempTail;//
		bool			is_Sending;//是否正在发送数据封包
		bool			is_SendCompleted;//是否发送完毕

		//时间类
		s32			time_AutoConnect;//存放连接时间
		s32			time_Heart;//记录心跳时间
		char			md5[MAX_MD5_LEN];//MD5码安全验证

		void	Init(int sid);
		void	reset();
	};


#pragma pack(pop, packing)

	class ITcpServer;
	class ITcpClient;
	typedef void(*TCPSERVERNOTIFY_EVENT) (ITcpServer* tcp, S_CLIENT_BASE *c , const s32 code);//服务器失去连接，异常连接，安全连接的时候，将事件派发到业务层
	typedef void(*TCPSERVERNOTIFYERRO_EVENT) (ITcpServer* tcp, S_CLIENT_BASE* c, const s32 code, const char* err);
	typedef void(*TCPCLIENTNOTIFY_EVENT) (ITcpClient* tcp, const s32 code);

	class ITcpServer
	{
	public:
		virtual ~ITcpServer() {}
		virtual void runServer(int num) = 0;
		virtual void stopServer() =0;
		
	#ifdef ____WIN32_
		virtual	S_CLIENT_BASE * client(SOCKET fd, bool issecurity) = 0;//通过socketID先查索引，然后精确定位当前S_CLIENT_BASE的数据,通过索引查找是最快的
	#else
		virtual	S_CLIENT_BASE* client(int fd, bool issecurity) = 0;
	#endif
		virtual	S_CLIENT_BASE* client(int index) = 0; //第二种办法，直接传索引值精确定位

		virtual bool isID_T(const s32 id) = 0;
		virtual bool isSecure_T(const s32 id, s32 secure) = 0;
		virtual bool isSecure_F_Close(const s32 id, s32 secure) = 0;//如果不是安全连接就直接关掉
		virtual void parseCommand() = 0;
		virtual void getSecurityCount(int& connectnum,int& securitycount) = 0;
		
		virtual void begin(const int id, const u16 cmd) = 0;// 玩家数据索引下标 和 头指令
		virtual void end(const int id) = 0;
		
		virtual void sss(const int id, const s8 v) = 0;
		virtual void sss(const int id, const u8 v) = 0;
		
		virtual void sss(const int id, const s16 v) = 0;
		virtual void sss(const int id, const u16 v) = 0;
		
		virtual void sss(const int id, const s32 v) = 0;
		virtual void sss(const int id, const u32 v) = 0;

		virtual void sss(const int id, const s64 v) = 0;
		virtual void sss(const int id, const u64 v) = 0;

		virtual void sss(const int id, const bool v) = 0;
		virtual void sss(const int id, const f32 v) = 0;
		virtual void sss(const int id, const f64 v) = 0;
		virtual void sss(const int id, void* v, const u32 len) = 0;

		virtual void read(const int id, s8& v) = 0;
		virtual void read(const int id, u8& v) = 0;

		virtual void read(const int id, s16& v) = 0;
		virtual void read(const int id, u16& v) = 0;

		virtual void read(const int id, s32& v) = 0;
		virtual void read(const int id, u32& v) = 0;

		virtual void read(const int id, s64& v) = 0;
		virtual void read(const int id, u64& v) = 0;

		virtual void read(const int id, f32& v) = 0;
		virtual void read(const int id, f64& v) = 0;

		virtual void read(const int id, bool& v) = 0;
		virtual void read(const int id, void* v, const u32 len) = 0;

		virtual void setOnClientAccept(TCPSERVERNOTIFY_EVENT event) =0;
		virtual void setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event) =0;//安全连接
		virtual void setOnClientDisConnect(TCPSERVERNOTIFY_EVENT event) =0;//失去连接
		virtual void setOnClientTimeout(TCPSERVERNOTIFY_EVENT event) =0;//超时连接
		virtual void setOnClientExcept(TCPSERVERNOTIFY_EVENT event) =0;//异常
		virtual void registerCommand(int cmd, void* container) =0;//注册消息
		
		
	};

	class ITcpClient
	{
	public:
		virtual ~ITcpClient(){}
		virtual S_SERVER_BASE* getData() = 0;
#ifdef ____WIN32_
		virtual SOCKET getSocket() = 0;
#else
		virtual int getSocket() = 0;
#endif

		virtual void runClient(u32 sid, char* ip, int port) = 0;
		virtual bool connectServer() = 0;
		virtual void disconnectServer(const s32 errcode, const char* err) = 0;

		virtual void begin( const u16 cmd) = 0;// 玩家数据索引下标 和 头指令
		virtual void end() = 0;

		virtual void sss( const s8 v) = 0;
		virtual void sss( const u8 v) = 0;

		virtual void sss( const s16 v) = 0;
		virtual void sss( const u16 v) = 0;

		virtual void sss(const s32 v) = 0;
		virtual void sss( const u32 v) = 0;

		virtual void sss(const s64 v) = 0;
		virtual void sss(const u64 v) = 0;

		virtual void sss( const bool v) = 0;
		virtual void sss( const f32 v) = 0;
		virtual void sss(const f64 v) = 0;
		virtual void sss( void* v, const u32 len) = 0;

		virtual void read( s8& v) = 0;
		virtual void read(u8& v) = 0;

		virtual void read(s16& v) = 0;
		virtual void read( u16& v) = 0;

		virtual void read( s32& v) = 0;
		virtual void read(u32& v) = 0;

		virtual void read( s64& v) = 0;
		virtual void read( u64& v) = 0;

		virtual void read( f32& v) = 0;
		virtual void read( f64& v) = 0;

		virtual void read(bool& v) = 0;
		virtual void read(void* v, const u32 len) = 0;

		virtual void parseCommand() = 0;
		virtual void registerCommand(int cmd, void* container) = 0;//注册消息
		virtual void setOnConnect(TCPCLIENTNOTIFY_EVENT event) = 0;
		virtual void setOnSecure(TCPCLIENTNOTIFY_EVENT event) = 0;//安全连接
		virtual void setOnDisConnect(TCPCLIENTNOTIFY_EVENT event) = 0;//失去连接
		virtual void setOnExcept(TCPCLIENTNOTIFY_EVENT event) = 0;//异常
	};

	extern ITcpServer* NewTcpServer();//生成一个ITcpServer对象
	extern ITcpClient* NewTcpClient();//生成一个ITcpClient对象
}

#endif
