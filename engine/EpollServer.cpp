

#include"EpollServer.h"

#ifndef ____WIN32_
#include <sys/socket.h>
#include <sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>

namespace net
{
    EpollServer::EpollServer()
    {
        m_IsRunning = false;
        m_ConnectNum = 0;
    	m_SecurityCount = 0;
    	m_ThreadNum = 0;
    	m_LinkNum = 0;

    	listenfd = -1;
    	epollfd = -1;

    	onAcceptEvent = nullptr;
    	onSecurityEvent = nullptr;
    	onTimeoutEvent = nullptr;
    	onDisconnectEvent = nullptr;
    	onExceptEvent = nullptr;
    	
    }

    bool setNonBlocking(int fd)
    {
	    int flags = fcntl(fd, F_GETFL);
        if (flags < 0) return false;
    	flags |= O_NONBLOCK;//设置为非阻塞标记
        if(fcntl(fd, F_SETFL, flags) < 0) return false;
        return true;
    }

    int EpollServer::add_event(int epollfd, int socketfd, int events)
    {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = socketfd;
        int value = epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &ev);
        return value;
    }

    int EpollServer::delete_event(int epollfd, int socketfd, int events)
    {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = socketfd;
        int value = epoll_ctl(epollfd, EPOLL_CTL_DEL, socketfd, &ev);
        return value;
    }



    //1.调用socket函数创建socket（侦听socket）
	//2.调用bind函数将socket绑定到某个ip和端口的二元组上
	//3.调用listen函数开启侦听
	//4.当用客户端请求连接之后，调用accept函数接受连接，产生一个新的socket
	//5.基于新产生的socket调用send或recv函数开始与客户端进行数据交流
	//6.通信结束后，调用close函数关闭侦听socket
    int EpollServer::initSocket()
    {
    	listenfd = socket(AF_INET, SOCK_STREAM, 0);//IPV4 流式套接字
        setNonBlocking(listenfd);//设置为非阻塞
        int rece = 0;
        int send = 0;
        setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, (const int*)& rece, sizeof(int));
        setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, (const int*)& send, sizeof(int));

        //启动端口号的重复绑定
        int flag = 1;
    	int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));//设置端口复用

        //绑定socket
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(func::__ServerInfo->Port);
        addr.sin_addr.s_addr = INADDR_ANY;

        //绑定监听socket到地址和端口号上
        ret = bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
        if(ret == -1)
        {
	        perror("bind error");
            exit(1);
        }
        //开始监听
        listen(listenfd, SOMAXCONN);
        //创建epoll 并注册
        epollfd = epoll_create(1111);//随便填一个>0的数就行
        add_event(epollfd, listenfd, EPOLLIN);//监听读事件

    	return 0;
    }



    void EpollServer::runServer(int num)
    {
    	//开辟接收缓冲区空间
    	for(int i = 0; i<num;i++ )
    	{
    		recvBuf[i] = new char[func::__ServerInfo->ReceOne];
    	}

    	//初始化玩家数组
    	Linkers = new HashArray<S_CLIENT_BASE>(func::__ServerInfo->MaxConnect);

    	for(int i=0;i<Linkers->length;i++)
    	{
    		S_CLIENT_BASE* c = Linkers->Value(i);
    		c->Init();
    	}
		//初始化玩家索引数组
    	LinkersIndex = new HashArray<S_CLIENT_BASE_INDEX>(MAX_USER_SOCKETFD);
    	for(int i=0;i<LinkersIndex->length;i++)
    	{
    		S_CLIENT_BASE_INDEX* cIndex = LinkersIndex->Value(i);
    		cIndex->Reset();
    	}
    	//初始化socket
        initSocket();
        //调用初始化线程
        runThread(num);

    }

    void EpollServer::stopServer()
    {
    }

    S_CLIENT_BASE* EpollServer::client(int fd, bool issecurity)
    {
    }

    S_CLIENT_BASE* EpollServer::client(int index)
    {
    }

    bool EpollServer::isID_T(const s32 id)
    {
    }

    bool EpollServer::isSecure_T(const s32 id, s32 secure)
    {
    }

    bool EpollServer::isSecure_F_Close(const s32 id, s32 secure)
    {
    }

    void EpollServer::parseCommand()
    {
    }

    void EpollServer::getSecurityCount(int& connectnum, int& securitycount)
    {
    }

    void EpollServer::begin(const int id, const u16 cmd)
    {
    }

    void EpollServer::end(const int id)
    {
    }

    void EpollServer::sss(const int id, const s8 v)
    {
    }

    void EpollServer::sss(const int id, const u8 v)
    {
    }

    void EpollServer::sss(const int id, const s16 v)
    {
    }

    void EpollServer::sss(const int id, const u16 v)
    {
    }

    void EpollServer::sss(const int id, const s32 v)
    {
    }

    void EpollServer::sss(const int id, const u32 v)
    {
    }

    void EpollServer::sss(const int id, const s64 v)
    {
    }

    void EpollServer::sss(const int id, const u64 v)
    {
    }

    void EpollServer::sss(const int id, const bool v)
    {
    }

    void EpollServer::sss(const int id, const f32 v)
    {
    }

    void EpollServer::sss(const int id, const f64 v)
    {
    }

    void EpollServer::sss(const int id, void* v, const u32 len)
    {
    }

    void EpollServer::read(const int id, s8& v)
    {
    }

    void EpollServer::read(const int id, u8& v)
    {
    }

    void EpollServer::read(const int id, s16& v)
    {
    }

    void EpollServer::read(const int id, u16& v)
    {
    }

    void EpollServer::read(const int id, s32& v)
    {
    }

    void EpollServer::read(const int id, u32& v)
    {
    }

    void EpollServer::read(const int id, s64& v)
    {
    }

    void EpollServer::read(const int id, u64& v)
    {
    }

    void EpollServer::read(const int id, f32& v)
    {
    }

    void EpollServer::read(const int id, f64& v)
    {
    }

    void EpollServer::read(const int id, bool& v)
    {
    }

    void EpollServer::read(const int id, void* v, const u32 len)
    {
    }

    void EpollServer::setOnClientAccept(TCPSERVERNOTIFY_EVENT event)
    {
    }

    void EpollServer::setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event)
    {
    }

    void EpollServer::setOnClientDisConnect(TCPSERVERNOTIFY_EVENT event)
    {
    }

    void EpollServer::setOnClientTimeout(TCPSERVERNOTIFY_EVENT event)
    {
    }

    void EpollServer::setOnClientExcept(TCPSERVERNOTIFY_EVENT event)
    {
    }

    void EpollServer::registerCommand(int cmd, void* container)
    {
    }
}

#endif
