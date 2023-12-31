#ifndef __IOPOOL_H
#define  __IOPOOL_H

//判断是不是win32平台下
#ifdef  ____WIN32_


#include "IDefine.h"
#define ACCEPT_BUF_LENGTH (sizeof(struct sockaddr_in) + 16 ) *2 //包含了远端地址和本地地址的信息长度所以*2  +16是Windows系统预留的16字节
#define POOL_SIZE 1000 //对象池的数量大小
class  IOContext
{
public:
	IOContext();
	~IOContext();

public:
	WSAOVERLAPPED	m_OverLapped;
	SOCKET					m_Socket;
	int							m_Mode; //表示当前投递的是什么模式
};

class AcceptContext : public IOContext
{
public:
	AcceptContext(int mode, SOCKET lisfd, SOCKET sfd);
	~AcceptContext(void);

public:
	SOCKET listenfd;
	unsigned char m_buf[ACCEPT_BUF_LENGTH];//存放本地地址和远端地址

public:
	void clear();//清理数据
	void setSocket(SOCKET lisfd, SOCKET sfd);//监听的socket和要投递的socket
	static AcceptContext* pop(); //通过方法直接获取一个对象实例
	static void push(AcceptContext* acc); //使用完放回对象回收池
	static int getCount();

};

class RecvContext : public IOContext
{
public:
	RecvContext(int mode);
	~RecvContext(void);

public:
	char* m_Buffs;//开辟好的空间，由xml配置的，接收数据的buff缓冲区
	WSABUF m_wsaBuf;//收到数据的时候，填充到m_wsaBuf，之后通知我们来这个变量取数据

public:
	void clear();//清理数据
	static RecvContext* pop(); //通过方法直接获取一个对象实例
	static void push(RecvContext* acc); //使用完放回对象回收池
	static int getCount();

};

class SendContext : public IOContext
{
public:
	SendContext(int mode);
	~SendContext(void);

public:
	char* m_Buffs;//开辟好的空间，由xml配置的，接收数据的buff缓冲区
	WSABUF m_wsaBuf;//收到数据的时候，填充到m_wsaBuf，之后通知我们来这个变量取数据

	public:
	void clear();//清理数据
	int setSend(SOCKET fd, char* data, int sendBytes);
	static SendContext* pop(); //通过方法直接获取一个对象实例
	static void push(SendContext* acc); //使用完放回对象回收池
	static int getCount();

};

#endif

#endif

