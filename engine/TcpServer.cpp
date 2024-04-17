#include "TcpServer.h"

#ifdef  ____WIN32_

#include "IOPool.h"
namespace net
{
	ITcpServer* NewTcpServer()
	{
		return new TcpServer();
	}
	
	TcpServer::TcpServer()
	{
		m_ThreadNum = 0;
		m_AcceptEx = NULL;
		m_GetAcceptEx = NULL;
		listenfd = INVALID_SOCKET;
		m_CompletePort = NULL;
		m_ConnectCount = 0;
		m_SecurityCount = 0;
		m_IsRunning = false;
		onAcceptEvent = nullptr;
		onDisconnectEvent = nullptr;
		onExceptEvent = nullptr;
		onSecureEvent = nullptr;
		onTimeOutEvent = nullptr;
	}

	TcpServer::~TcpServer()
	{
		//TCPServer在整个生命周期内不会被删除
	}



	//运行服务器 入口
	void TcpServer::runServer(int num)
	{
		//1.创建连接用户（10000个）
		Linkers = new HashArray<S_CLIENT_BASE>(func::__ServerInfo->MaxConnect);
		for(int i=0; i< Linkers->length; i ++)
		{
			auto c = Linkers->Value(i);
			c->Init();
		}
		//2.创建连接用户索引
		LinkerIndexs = new HashArray<S_CLIENT_BASE_INDEX>(MAX_USER_SOCKETFD);
		for(int i = 0; i< LinkerIndexs->length; i++)
		{
			auto c = LinkerIndexs->Value(i);
			c->Reset();
		}
		//3.初始化指令集
		initCommands();
		//4.初始化socket
		int errorcode = initSocket();
		if(errorcode < 0)
		{
			LOG_MSG("initSocket is err... %d \ in line %d", errorcode, __LINE__);
			RELEASE_HANDLE(m_CompletePort);
			RELEASE_SOCKET(listenfd);
			if (errorcode != -2) WSACleanup();//套接字加载失败 清理一下
			return;
		}

		//5.初始化投递
		initPost();
		//6.调用工作线程
		this->runThread(num);
	}

	void TcpServer::stopServer()
	{
		for(int i=0; i<m_ThreadNum ;i++)
		{
			PostQueuedCompletionStatus(m_CompletePort, 0 ,(DWORD)1, NULL);
		}
		m_IsRunning = false;
		RELEASE_HANDLE(m_CompletePort);//释放句柄
		RELEASE_SOCKET(listenfd);//释放套接字
		WSACleanup();//卸载网络库

		LOG_MSG("Stop Server Success!!!\n");
	}


	s32 TcpServer::initSocket()
	{
		//1.创建完成端口 如果函数成功，则返回值是 I/O 完成端口的句柄：
		m_CompletePort = CreateIoCompletionPort(
			INVALID_HANDLE_VALUE, //打开的文件句柄或 INVALID_HANDLE_VALUE。句柄必须是支持重叠 I / O 的对象。
			NULL, //如果此参数为 NULL，则函数将创建新的 I/O 完成端口,如果 FileHandle 参数有效，则将其与新的 I/O 完成端口相关联。 否则不会发生文件句柄关联。 如果成功，函数会将句柄返回到新的 I/O 完成端口。
			0 ,
			0); //操作系统允许并发处理 I / O 完成端口的 I / O 完成数据包的最大线程数如果此参数为零，则系统允许并发运行与系统中处理器数一样多的线程。
		if (m_CompletePort == NULL) return -1;
		//2.加载套接字DLL
		WSADATA	wsData;
		int errorcode = WSAStartup(MAKEWORD(2,2), &wsData);
		//加载socket套接字库失败
		if (errorcode != 0) return -2;
		//3.创建套接字
		listenfd = WSASocket(AF_INET, SOCK_STREAM, 0 , NULL , 0 , WSA_FLAG_OVERLAPPED);
		if (listenfd == INVALID_SOCKET) return -3;
		//4.设置非阻塞模式
		unsigned long u1 = 1;
		errorcode = ioctlsocket(listenfd, FIONBIO, (unsigned long*)&u1);
		if (errorcode == SOCKET_ERROR) return -4;
		//5.关闭读写端到数据
		int size = 0;
		setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF , (char*)&size, sizeof(int));//关闭连接
		setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int));//关闭连接
		//6.监听和完成端口绑定
		HANDLE handle = CreateIoCompletionPort((HANDLE)listenfd, m_CompletePort, DWORD( func::SC_WAIT_ACCEPT), 0);
		if (handle == nullptr) return -5;
		//7.绑定套接字
		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;//IPV4
		serAddr.sin_port = htons(func::__ServerInfo->Port); //监听xml的这个端口号 将u_short从主机转换为 TCP/IP 网络字节顺序 (这是 big-endian) 。
		serAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//监听本机所有IP 将u_long从主机转换为 TCP/IP 网络字节顺序 (这是 big-endian) 。
		errorcode = ::bind(listenfd, (struct sockaddr*)&serAddr, sizeof(serAddr) );//给服务端绑定地址信息结构体 将本地地址与套接字相关联。
		if (errorcode == SOCKET_ERROR) return -6;
		//8.监听套接字
		errorcode = listen(listenfd, SOMAXCONN);//等待队列 已经完成三次握手的数量
		if (errorcode == SOCKET_ERROR) return -7;
		//9.导出函数指针
		GUID GuidAcceptEx			= WSAID_ACCEPTEX;
		GUID GuidGetAcceptEx	= WSAID_GETACCEPTEXSOCKADDRS;
		DWORD	dwBytes				= 0;
		if(m_AcceptEx == nullptr)
		{
			errorcode = WSAIoctl(
							listenfd,
							SIO_GET_EXTENSION_FUNCTION_POINTER,
							&GuidAcceptEx,
							sizeof(GuidAcceptEx),
							&m_AcceptEx,
							sizeof(m_AcceptEx),
							&dwBytes,
							NULL,
							NULL
			);
		}

		if (m_AcceptEx == nullptr || errorcode == SOCKET_ERROR) return -8;

		if (m_GetAcceptEx == nullptr)
		{
			errorcode = WSAIoctl(
				listenfd,
				SIO_GET_EXTENSION_FUNCTION_POINTER,
				&GuidGetAcceptEx,
				sizeof(GuidGetAcceptEx),
				&m_GetAcceptEx,
				sizeof(m_GetAcceptEx),
				&dwBytes,
				NULL,
				NULL
			);
		}
		if (m_GetAcceptEx == nullptr || errorcode == SOCKET_ERROR) return -9;
		//操作成功
		return 0;
	}

	//初始化投递
	void TcpServer::initPost()
	{
		for(int i=0; i< func::__ServerInfo->MaxAccept; i++)
		{
			postAccept();
		}
	}
	


	s32 TcpServer::postAccept()
	{
		SOCKET socketfd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (socketfd == INVALID_SOCKET) return -1;
		//设置非阻塞模式
		unsigned long ul = 1;
		int errorcode = ioctlsocket( socketfd, FIONBIO, (unsigned long*)& ul );
		if (errorcode == SOCKET_ERROR)
		{
			closeSocket(socketfd, NULL, 1001);
			return -2;
		}
		//获取对象回收池
		AcceptContext* context = AcceptContext::pop();
		context->m_Mode = func::SC_WAIT_ACCEPT;
		context->setSocket(listenfd, socketfd);
		unsigned long dwBytes = 0;

			//1、监听socket
			//2、接受连接socket
			//3、接受缓冲区 a、客户端发来第一组数据 b、server地址 c、client地址
			//4、0不会等到数据到来直接返回 非0等待数据
			//5、本地地址大小；长度必须为地址长度 + 16字节
			//6、远端地址大小；长度必须为地址长度 + 16字节
			//7、同步方式才有用 我们是异步IO没用，不用管；
			//8、本次重叠I / O所要用到的重叠结构
		bool isaccept = m_AcceptEx(
			context->listenfd, 
			context->m_Socket,
			context->m_buf,
			0,
			sizeof(SOCKADDR_IN) +16,
			sizeof(SOCKADDR_IN) + 16,
			&dwBytes,
			&context->m_OverLapped
			);
		if(isaccept == false)
		{
			int error = WSAGetLastError();
			if( ERROR_IO_PENDING != error )//如果没有投递成功，关闭socket
			{
				closeSocket(socketfd, NULL, 1002);
				AcceptContext::push(context);
				return -3;
			}
		}



		return 0;
	}

	s32 TcpServer::onAccept(void* context)
	{
		AcceptContext* acc = (AcceptContext*)context;
		if (acc == nullptr) return -1;
		SOCKADDR_IN* ClientAddr = NULL;
		SOCKADDR_IN* LocalAddr = NULL;
		int remoteLen = sizeof(SOCKADDR_IN);//远端地址长度
		int localLen = sizeof(SOCKADDR_IN);//本地地址长度
		int errorcode = 0;

		//1、指向传递给AcceptEx函数接收第一块数据的缓冲区
		//2、缓冲区大小，必须和传递给AccpetEx函数的一致
		//3、本地地址大小，必须和传递给AccpetEx函数一致
		//4、远程地址大小，必须和传递给AccpetEx函数一致
		//5、用来返回连接的本地地址
		//6、用来返回本地地址的长度
		//7、用来返回远程地址
		//8、用来返回远程地址的长度
		m_GetAcceptEx(//函数指针来获取新的连接数据
			acc->m_buf,
			0,
			sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16,
			(LPSOCKADDR*)&LocalAddr,
			&localLen,
			(LPSOCKADDR*)&ClientAddr,
			&remoteLen
		);

		//设置更新属性
		//将监听socket（listenfd）的属性复制到新的acc->m_Socket上
		setsockopt(acc->m_Socket, SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenfd, sizeof(listenfd));
		//设置发送接收缓冲区
		int rece = func::__ServerInfo->ReceOne;
		int send = func::__ServerInfo->SendOne;
		setsockopt(acc->m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)& send, sizeof(send));
		setsockopt(acc->m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)& rece, sizeof(rece));//重新设置了当前socket的接收缓冲区和发送缓冲区
		
		//设置心跳包
		int err = this->setHeartCheck(acc->m_Socket);
		if(err < 0) return -2;

		//socket绑定到完成端口
		HANDLE handle = CreateIoCompletionPort((HANDLE)acc->m_Socket, m_CompletePort, (DWORD)func::SC_WAIT_ACCEPT, 0);
		if (handle == nullptr) return -3;

		//获取玩家连接数据的地址
		S_CLIENT_BASE_INDEX * cindex = getClientIndex(acc->m_Socket);
		if(cindex == nullptr) return -4;

		//获取玩家数据
		S_CLIENT_BASE* c = getFreeLinker();
		if (c == nullptr)
		{
			//获取不到空闲结构说明服务器已满
			LOG_MSG("Server is full!!! ...\n");
			return -5;
		}

		//c->ID = cindex->index;
		cindex->index = c->ID;//让索引的index为玩家的id

		//保存数据
		memcpy(c->ip, inet_ntoa(ClientAddr->sin_addr), MAX_IP_LEN);//将网络地址转换成字符串格式 "127.0.0.1"
		c->socketfd		= acc->m_Socket;
		c->port			= ntohs(ClientAddr->sin_port); //将网络字节序转换成本机字节序
		c->state		= func::S_Connect;
		c->time_Connect = (int)time(NULL);
		c->time_Heart	= (int)time(NULL);

		//将接收保存下来的数据投递出去
		int ret = this->postRecv(acc->m_Socket);
		if(ret != 0)
		{
			c->Reset();  
			return -6;
		}

		//随机加密异或码
		srand(time(NULL));//生成随机种子
		u8 rcode = rand() % 125 +1;
		//封包绝对不要放在工作线程，这里是一个例外，此时和客户端还没有产生一个安全的连接，设计上在主线程封包
		this->begin(c->ID, CMD_RCODE);//调用封包方法,客户端连接到服务器了，马上给客户端发送一条消息
		this->sss(c->ID, rcode);
		this->end(c->ID);
		c->rCode = rcode;//设置服务端rcode=随机生成的rcode
		updateConnect(true);
		
		if(onAcceptEvent != nullptr) this->onAcceptEvent(this, c, 0);
		AcceptContext::push(acc);//投递出去后就回收
		this->postAccept();//投递新的连接
		
		return 0;
	}

	//投递接收的数据
	s32 TcpServer::postRecv(SOCKET s)
	{
		RecvContext* context = RecvContext::pop();
		context->m_Socket = s;
		context->m_Mode = func::SC_WAIT_RECV;
		unsigned long bytes = 0;
		unsigned long flag = 0;
		//1、 操作的套接字
		//2、 接收缓冲区
		//3、 wsaBuf数组中WSABUF结构的数目
		//4、 如果接收操作立即完成,返回函数调用所接收到的字节数
		//5、 用来控制套接字的行为 一般设置为0
		//6、 重叠结构
		//7、 一个指向接收操作结束后调用的例程的指针,一般填NULL
		int err = WSARecv(
			context->m_Socket,
			&context->m_wsaBuf,
			1,
			&bytes,
			&flag,
			&(context->m_OverLapped),
			NULL
		);
		
		if(err == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if( ERROR_IO_PENDING != error )//如果没有投递成功，关闭socket
				{
					RecvContext::push(context);
					return -1;
				}
			//return -1;
		}
		return 0;
	}

	s32 TcpServer::onRecv(void* context, s32 recvBytes, u32 tid)
	{
		RecvContext* rece = (RecvContext*)context;
		if(rece == NULL) return -1;

		S_CLIENT_BASE* c = client(rece->m_Socket, true);//通过socketid获得玩家索引 再通过索引获得玩家数据 
		if(c == nullptr)
		{
			//工作线程只关闭读写端，主线程才统一关闭socket
			shutDown(rece->m_Socket, 0, NULL ,1001 );
			RecvContext::push(rece);
			return -1;
		}
		c->threadID = tid;//方便后面输出查看id信息，没多大意义
		
		s32 errorcode = onRecv_SavaData(c, rece->m_wsaBuf.buf, recvBytes);
		if(errorcode != 0)
		{
			c->is_RecvCompleted = true;
			shutDown(rece->m_Socket, 0, NULL ,1002 );
			RecvContext::push(rece);
			return -2;
		}

		int ret = this->postRecv(rece->m_Socket);
		if(ret != 0)
		{
			c->is_RecvCompleted = true;
			shutDown(rece->m_Socket, 0, NULL ,1003 );
			RecvContext::push(rece);
			return -2;
		}
		c->is_RecvCompleted = true;
		RecvContext::push(rece);
		
		return 0;
	}

	//保存数据
	s32 TcpServer::onRecv_SavaData(S_CLIENT_BASE* c, char* buf, s32 recvBytes)
	{
		if(buf == nullptr) return -1;
		//生产者线程判断
		if(c->recv_Head == c->recv_Tail)//说明消耗完毕，重新初始化,重复循环利用
		{
			c->recv_Tail = 0;
			c->recv_Head = 0;
		}
		//说明缓冲区满了
		if(c->recv_Tail + recvBytes > func::__ServerInfo->ReceMax) return -2;

		memcpy( &c->recvBuf[c->recv_Tail], buf, recvBytes );//复制接收到的信息到recvBuf的尾部
		c->recv_Tail += recvBytes; //复制后尾部需要偏移
		return 0;
	}

	s32 TcpServer::postSend(S_CLIENT_BASE* c)
	{
		if(c->is_SendCompleted == false) return -1;
		if(c->ID < 0 || c->state == func::S_Free || c->closeState == func::S_CLOSE_SHUTDOWN || c->socketfd == INVALID_SOCKET) return -2;
		if(c->send_Tail <= c->send_Head) return -3;
		
		s32 sendBytes = c->send_Tail - c->send_Head;
		if(sendBytes <= 0) return -4;
		if(sendBytes > func::__ServerInfo->SendOne) sendBytes = func::__ServerInfo->SendOne;//不能超过一次最大发送的数量
		
		SendContext* context = SendContext::pop();//从对象池取出一个
		context->m_Mode = func::SC_WAIT_SEND;
		context->setSend(c->socketfd, &c->sendBuf[c->send_Head], sendBytes);//发送
		
		unsigned long dwBytes = 0;
		int err = WSASend(
		context->m_Socket,
		&context->m_wsaBuf,
		1,
		&dwBytes,
		0,
		&(context->m_OverLapped),
		NULL
		);
		
		if(err == SOCKET_ERROR)
		{
			int error = WSAGetLastError();//此时若是正常 返回的错误码是正常
			if( ERROR_IO_PENDING != error )//如果没有投递成功，关闭socket
				{
				shutDown(context->m_Socket, 0, c, 1004);
				SendContext::push(context);
				return -4;
				}
		}
		//调用成功
		return 0;
	}

	s32 TcpServer::onSend(void* context, s32 sendBytes)
	{
		SendContext* sc = (SendContext*)context;
		if(sc == nullptr) return -1;
		 if(sc->m_wsaBuf.len != sendBytes)
		 {
		 	shutDown(sc->m_Socket, 0, nullptr, 1005);
		 	SendContext::push(sc);
		 	return -1;
		 }

		//查找连接数据
		S_CLIENT_BASE* c = client(sc->m_Socket, true);
		if(c == nullptr)
		{
			shutDown(sc->m_Socket, 0, nullptr, 1006);
			SendContext::push(sc);
			return -1;
		}

		if(c->ID < 0 || c->state == func::S_Free || c->closeState == func::S_CLOSE_SHUTDOWN || c->socketfd == INVALID_SOCKET)
		{
			c->is_SendCompleted = true;
			SendContext::push(sc);
			return -2;
		}

		c->send_Head += sendBytes;//发送成功了发送头就要偏移发送的数据量的大小
		c->is_SendCompleted = true;
		SendContext::push(sc);
		return 0;
	}

	//****************************************************
	//关闭socket
	s32 TcpServer::closeSocket(SOCKET sfd, S_CLIENT_BASE* c, int kind)
	{
		if (sfd == SOCKET_ERROR || sfd == INVALID_SOCKET || sfd == NULL) return -1;

		if(c != nullptr)
		{
			if (c->state == func::S_Free) return -1;//如果已经释放了，是为了防止重复调用
			if(c->state >= func::S_ConnectSecure)
			{
				this->updateSecurityConnect(false);//计数器自减1
			}
		}

		switch(kind)
		{
		case 1001:
		case 1002:
		case 3004:
			RELEASE_SOCKET(sfd);//释放socket
			break;
		default:
			this->updateConnect(false);//有人掉线或者离开的时候，先把socket关掉
			shutdown(sfd, SD_BOTH);
			RELEASE_SOCKET(sfd);//释放socket
			break;
		}

		if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, c, kind);//派发到业务层
		/*if(c!= nullptr)//再来找数据结构体
		{
			if(c->state == func::S_Connect || c->state == func::S_ConnectSecure)
			{
				c->Reset();//直接把这个socket对应的连接的数据初始化掉
			}
		}*/
		return 0;

	}

	//永远的关闭读写端
	void TcpServer::shutDown(SOCKET sfd, s32 mode, S_CLIENT_BASE* c, int kind)
	{
		if(c != nullptr)
		{

			if (c->state == func::S_Free) return;
			if (c->closeState == func::S_CLOSE_SHUTDOWN) return;
			c->shutdown_kind = kind;
			c->time_Close = (int)time(NULL);
			c->closeState = func::S_CLOSE_SHUTDOWN;

			shutdown(sfd, SD_BOTH);
			CancelIoEx((HANDLE)sfd, nullptr); //取消IO操作，撤销socket上的投递数据

			if (onExceptEvent != nullptr) this->onExceptEvent(this, c, kind+300);
			return;
		}
		//传入的c为空的话，就去用方法查找这个socket对应的S_CLIENT_BASE
		auto c2 = client(sfd, true);
		if(c2 == nullptr)
		{
			LOG_MSG("find socketfd is err:%d in line %d \n", int(sfd), __LINE__);
			return;
		}
		if (c2->state == func::S_Free) return;
		if (c2->closeState == func::S_CLOSE_SHUTDOWN) return;
		//判断模式
		switch(mode)
		{
		case func::SC_WAIT_RECV://c是空值 认为是接收完毕了
			c2->is_RecvCompleted = true;
			break;
		case func::SC_WAIT_SEND://c是空值 认为是发送完毕了
			c2->is_SendCompleted = true;
			break;
		}
		c2->shutdown_kind = kind;
		c2->time_Close = (int)time(NULL);
		c2->closeState = func::S_CLOSE_SHUTDOWN;
		shutdown(sfd, SD_BOTH);
		CancelIoEx((HANDLE)sfd, nullptr); //取消IO操作，撤销socket上的投递数据
		if (onExceptEvent != nullptr) this->onExceptEvent(this, c2, kind+400);
	}


}

#endif

