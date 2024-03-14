#ifndef __IDEFINE_H
#define  __IDEFINE_H

#include <vector>

#ifdef  ____WIN32_
#include<WinSock2.h>
#pragma  comment (lib,"ws2_32")
#endif


#define MAX_USER_SOCKETFD	100000 //最大连接数量
#define MAX_EXE_LEN			200 //exe路径最大长度
#define MAX_FILENAME_LEN	250 //文件名最大长度
#define SIO_KEEPALIVE_VALS IOC_IN | IOC_VENDOR | 4 //Windows下心跳包的宏定义
#define MAX_MD5_LEN	35 //服务端和客户端进行通讯时进行MD5码验证的最大长度
#define MAX_IP_LEN	20 //IP的长度
#define MAX_COMMAND_LEN 65535//最大指令长度	

#define LOG_MSG printf //日志打印的宏

#define CMD_HEART		60000 //心跳包
#define CMD_RCODE		65001 //加密码
#define CMD_SECURITY	65002 //安全验证

#ifdef ____WIN32_
//定义一些释放指针句柄handle socket的宏
#define RELEASE_POINTER(p) { if(p != NULL) {delete p; p = NULL;} }
#define RELEASE_HANDLE(h) { if(h != NULL && h != INVALID_HANDLE_VALUE) {CloseHandle(h);} h = INVALID_HANDLE_VALUE; }
#define RELEASE_SOCKET(s) {if( s != INVALID_SOCKET) { closesocket(s); s = INVALID_SOCKET; }}
#endif

typedef signed char         s8;
typedef signed short		s16;
typedef signed int			s32;
typedef signed long long	s64;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long	u64;
typedef float				f32;
typedef double				f64;

namespace  func
{
	enum SOCKET_CLOSE
	{
		S_CLOSE_FREE = 0,
		S_CLOSE_ACCEPT = 1,//连接出错关闭
		S_CLOSE_SHUTDOWN = 2 ,//关闭连接
		S_CLOSE_CLOSE = 3, //正式关闭
	};

	//IOCP投递信息状态
	enum SOCKET_CONTEXT_STATE
	{
		SC_WAIT_ACCEPT = 0,
		SC_WAIT_RECV = 1,
		SC_WAIT_SEND = 2,
		SC_WAIT_RESET = 3,
	};

	//定义服务器的socket状态 是否需要保存，是否安全，保存中....
	enum S_SOCKET_STATE
	{
		S_Free = 0,
		S_Connect = 1,
		S_ConnectSecure = 2,
		S_Login = 3,
		S_NeedSave = 4,
		S_Saving = 5,
	};

	enum C_SOCKET_STATE
	{
		C_Free = 0,
		C_ConnectTry = 1,
		C_Connect = 2,
		C_ConnectSecure = 3,
		C_Login = 4,
	};

	enum S_SERVER_TYPE
	{
		S_TYPE_USER = 0x00,
		S_TYPE_DB ,
		S_TYPE_CENTER ,
		S_TYPE_GAME ,
		S_TYPE_GATE ,
		S_TYPE_LOGIN ,
	};

	//缓存最大数据量 IP 端口 版本号验证 啥的
	struct ConfigXML
	{
		s32	ID; //当前应用程序启动的ID号是多少
		u8	Type;//当前运行的服务器是个什么类型的服务器,ds服务器，中心服务器，地图服务器
		u16	Port; //端口号
		s32	MaxPlayer; //最大玩家
		s32	MaxConnect; //当前服务器允许的最大连接数量
		u8	RCode; //缓存客户端发来的消息数据，做一些加密的运算啥的
		s32	Version; //版本号，用于在连接的过程中验证
		s32	ReceOne;	//服务端接收数据的时候一次最多接收多少数据
		s32	ReceMax; //客户端新连接所开辟缓存空间的大小
		s32	SendOne; //一次最多发送多少字节的数据
		s32	SendMax; //最大发送缓冲区，当一次发不完的时候，就不允许发等下一次再发
		s32	HeartTime; //通过心跳来判断服务端和客户端连接是否有效，是否要释放
		s32	AutoTime; //自动重连时间
		s32	MaxAccept; //IOCP部分 
		s32	MaxRece; //IO对象回收池
		s32	MaxSend;
		char	SafeCode[20];//安全码 最大20字节,产生新连接时判断是否是合法连接
		char	Head[3];//验证消息头
		char	IP[MAX_IP_LEN];
	};

	//连接服务端用
	struct ServerListXML
	{
		s32	ID; //需要连接当前服务端的ID
		u16	Port; //连接服务器的端口号
		char	IP[MAX_IP_LEN]; //连接服务器的IP地址
	};

	//声明全局变量
	extern char FileExePath[MAX_EXE_LEN];//存放文件路径
	extern ConfigXML* __ServerInfo; //服务端XML指针变量
	extern ConfigXML* __ClientInfo; //客户端XML指针变量
	extern std::vector<ServerListXML*> __ServerListInfo; //需要连接到不同服务器的时候存放到这个数组里，有一组数据就只连接一个服务器，有两组就连接两个
	extern void(*MD5str) (char* output, unsigned char* input, int Len); //MD5编码验证用
	extern bool InitData(); //初始化数据函数

	extern u8 GetServerType(s32 sid);
	extern const char* getShutDownError(const s32 id);
	extern const char* getCloseSocketError(const s32 id);
	extern void SetConsoleColor(u16 index);

}

#endif


