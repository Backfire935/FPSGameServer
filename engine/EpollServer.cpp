

#include"EpollServer.h"


#ifndef ____WIN32_
#include <sys/socket.h>
#include <sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include <cstring>


namespace net
{
    ITcpServer* NewTcpServer()
    {
        return new EpollServer();
    }

    EpollServer::EpollServer()
    {
        m_IsRunning = false;
        m_ConnectNum = 0;
    	m_SecurityCount = 0;
    	m_ThreadNum = 0;
    	m_LinkIndex = 0;

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

    //接收新的连接
    void EpollServer::onAccept()
    {
    	struct sockaddr_in addr;//存放客户端的IP端口号
    	socklen_t client_len = sizeof(addr);
    	int clientfd = accept(listenfd, (struct sockaddr*)& addr, &client_len);
    	if(clientfd == -1)//没有新的连接
    	{
    		//输出一个错误
    		perror("accept error:");
    		return;
    	}
        S_CLIENT_BASE* c = getFreeLinker();//获取一个空闲的玩家数据
        S_CLIENT_BASE_INDEX* cindex = getClientIndex(clientfd);//获取一个空闲的玩家索引
        if( c == nullptr)
        {
            closeSocket(clientfd, nullptr, 3004);
            return;
        }
        if ( cindex == NULL)
        {
            c->Reset();
            closeSocket(clientfd, nullptr, 3004);
            return;
        }
        cindex->index = c->ID;//索引等于玩家数据的ID
        c->socketfd = clientfd;
        c->time_Connect = (int)time(NULL);
		c->port = ntohs(addr.sin_port);//网络字节序转换为主机字节序
        c->time_Heart = (int)time(NULL);//心跳时间
        c->state = func::C_Connect;
        memcpy(c->ip, inet_ntoa(addr.sin_addr), MAX_IP_LEN);//将网络字节序转换为点分十进制

        setNonBlocking(clientfd);//设置为非阻塞socket
		add_event(epollfd, c->socketfd, EPOLLIN | EPOLLET);//注册监听读事件 以边缘触发方式

        this->updateConnectCount(true);

        srand(time(NULL));
        u8 rcode = rand() % 100 + 1;
        begin(c->ID, CMD_RCODE);
        sss(c->ID, rcode);
        end(c->ID);
        c->rCode = rcode;

        if (onAcceptEvent != nullptr) this->onAcceptEvent(this, c ,0);//回调函数

    }

    //接收数据
    void EpollServer::onRecv(int socketfd, int threadid)
    {
        if (threadid >= 10) return;
    	S_CLIENT_BASE* c = client(socketfd, true);
    	if (c == nullptr) return;

        memset( recvBuf[threadid], 0, func::__ServerInfo->ReceOne);
        while (true)//收到的数据可能大于缓冲区大小，所以要循环接收,一直读到出错为止
        {
			int len = recv(socketfd, recvBuf[threadid], func::__ServerInfo->ReceOne, 0);//接收数据
            if (len < 0)//出错
            {
                if(errno == EINTR) continue;//这种不用管,数据没收完，正常情况
                else if (errno == EAGAIN) break;//没有数据了
				else
				{
					shutDown(socketfd, 0, c, 1001);
					return;
				}
            }
            else if(len == 0)//对端关闭了连接
            {
            	shutDown(socketfd, 0, c, 3003);
				return;
			}
			//正常情况
			
        	int error = onRecv_SaveData(c, recvBuf[threadid], len);//保存数据
        	if (error < 0)//出错
        	{
        		shutDown(socketfd, 0, c, 1002);
        		return;
        	}
            if(len < func::__ServerInfo->ReceOne) break;//没有数据了，收到的数据小于缓冲区大小
        }
		
		
    }

    //保存数据
    int EpollServer::onRecv_SaveData(S_CLIENT_BASE* c, char* buf, int recvBytes)
    {
        if (buf == nullptr) return -1;
        if(c->recv_Tail == c->recv_Head)//缓冲区为空，数据读取完毕
        {
        	c->recv_Head = 0;
			c->recv_Tail = 0;
		}

        if (c->recv_Tail + recvBytes > func::__ServerInfo->ReceMax)//缓冲区满了
            return -1;

        memcpy(&c->recvBuf[c->recv_Tail], buf, recvBytes);//将数据拷贝到缓冲区
    	c->recv_Tail += recvBytes;//尾指针后移
        c->is_RecvCompleted = true;//接收完成
        return 0;
        
    }

    //发送数据
    int EpollServer::onSend(S_CLIENT_BASE* c)
    {
        if(c->ID < 0 || c->state == func::S_Free || c->socketfd == -1) return -1;
        if(c->send_Tail <= c->send_Head) return 0;//没有数据需要发送，都发送完了
        int sendlen = c->send_Tail - c->send_Head;
        if(sendlen <= 0) return 0;


        int send_bytes = send(c->socketfd, &c->sendBuf[c->send_Head], sendlen, 0);//发送数据
        if (send_bytes < 0)//出错
        {
            if (errno == EINTR) return 1;
            else if (errno == EAGAIN) return 1;
            else
            {
                shutDown(c->socketfd, 0, c, 1006);
                return -2;
            }

        }
        else if (send_bytes == 0)//被动关闭了这个连接
        {
            shutDown(c->socketfd, 0, c, 3005);
            return -3;
        }
        //发送成功
        c->send_Head += send_bytes;//头指针后移
        c->is_SendCompleted = true;//发送完成
        return 0;

#pragma region("使用while不停发送的参考")
   //     while(true)//发送数据可能大于缓冲区大小，所以要循环发送,一直发送到出错为止
   //     {
	  //      int send_bytes = send(c->socketfd, &c->sendBuf[c->send_Head], sendlen, 0);//发送数据
   //         if(send_bytes < 0)//出错
   //         {
   //             if (errno == EINTR) continue;
   //             else if (errno == EAGAIN) return 1;
			//	else
			//	{
			//		shutDown(c->socketfd, 0, c, 1006);
			//		return -1;
			//	}
			//}
			//else if(send_bytes == 0)//被动关闭了这个连接
			//{
			//	shutDown(c->socketfd, 0, c, 3005);
			//	return -1;
   //         }
   //         //发送成功
   //         c->send_Head += send_bytes;//头指针后移
   //         if (c->send_Head == c->send_Tail) break;//发送完了
   //         sendlen = c->send_Tail - c->send_Head;//剩余的数据

   //     }
   //     c->is_SendCompleted = true;//发送完成
#pragma endregion

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
        setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, (const int*)& rece, sizeof(int));//设置接收缓冲区大小
        setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, (const int*)& send, sizeof(int));//设置发送缓冲区大小

        //启动端口号的重复绑定
        int flag = 1;
    	int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));//设置端口复用

        //绑定socket
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;//IPV4
        addr.sin_port = htons(func::__ServerInfo->Port);//端口号
        addr.sin_addr.s_addr = INADDR_ANY;//IP地址

        //绑定监听socket到地址和端口号上
        ret = bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));//绑定
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
        //初始化指令集
        initCommands();
    }



    int EpollServer::closeSocket(int socketfd, S_CLIENT_BASE* c, int kind)
    {
        if (socketfd == -1) return -1;
        if(c != nullptr)
        {
            if (c->state == func::S_Free) return 0;
            if(c->state == func::S_ConnectSecure)
            {
                this->updateSecurityCount(false);
            }
            auto cindex = getClientIndex(socketfd);
            if (cindex != nullptr) cindex->Reset();
        }

        this->updateConnectCount(false);
        delete_event(epollfd, socketfd, EPOLLIN | EPOLLET);
        close(socketfd);
        if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, c, kind);

        //初始化
        if(c->state == func::S_Connect || c->state == func::S_ConnectSecure)
        c->Reset();
        return 0;
    }

    //关闭socket
    void EpollServer::shutDown(int socketfd, const int mode, S_CLIENT_BASE* c, int kind)
    {
        if( c!= nullptr )
        {
	        if(c->state == func::S_Free) return;//空闲状态不做处理
            if (c->closeState == func::S_CLOSE_SHUTDOWN) return; //正在关闭状态
            c->shutdown_kind = kind;
            c->closeState = func::S_CLOSE_SHUTDOWN;
            shutdown(socketfd, SHUT_RDWR);

            if (onExceptEvent != nullptr) this->onExceptEvent(this, c, kind);
            return;
        }

        auto c2 = client(socketfd, true);
        if (c2 == nullptr) return;

        if (c2->state == func::S_Free) return;//空闲状态不做处理
        if (c2->closeState == func::S_CLOSE_SHUTDOWN) return; //正在关闭状态
        c2->shutdown_kind = kind;
        c2->closeState = func::S_CLOSE_SHUTDOWN;
        shutdown(socketfd, SHUT_RDWR);

        if (onExceptEvent != nullptr) this->onExceptEvent(this, c2, kind);
    }
}

#endif
