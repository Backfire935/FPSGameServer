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
		//TCPServer���������������ڲ��ᱻɾ��
	}



	//���з����� ���
	void TcpServer::runServer(int num)
	{
		//1.���������û���10000����
		Linkers = new HashArray<S_CLIENT_BASE>(func::__ServerInfo->MaxConnect);
		for(int i=0; i< Linkers->length; i ++)
		{
			auto c = Linkers->Value(i);
			c->Init();
		}
		//2.���������û�����
		LinkerIndexs = new HashArray<S_CLIENT_BASE_INDEX>(MAX_USER_SOCKETFD);
		for(int i = 0; i< LinkerIndexs->length; i++)
		{
			auto c = LinkerIndexs->Value(i);
			c->Reset();
		}
		//3.��ʼ��ָ�
		initCommands();
		//4.��ʼ��socket
		int errorcode = initSocket();
		if(errorcode < 0)
		{
			LOG_MSG("initSocket is err... %d \ in line %d", errorcode, __LINE__);
			RELEASE_HANDLE(m_CompletePort);
			RELEASE_SOCKET(listenfd);
			if (errorcode != -2) WSACleanup();//�׽��ּ���ʧ�� ����һ��
			return;
		}

		//5.��ʼ��Ͷ��
		initPost();
		//6.���ù����߳�
		this->runThread(num);
	}

	void TcpServer::stopServer()
	{
		for(int i=0; i<m_ThreadNum ;i++)
		{
			PostQueuedCompletionStatus(m_CompletePort, 0 ,(DWORD)1, NULL);
		}
		m_IsRunning = false;
		RELEASE_HANDLE(m_CompletePort);//�ͷž��
		RELEASE_SOCKET(listenfd);//�ͷ��׽���
		WSACleanup();//ж�������

		LOG_MSG("Stop Server Success!!!\n");
	}


	s32 TcpServer::initSocket()
	{
		//1.������ɶ˿� ��������ɹ����򷵻�ֵ�� I/O ��ɶ˿ڵľ����
		m_CompletePort = CreateIoCompletionPort(
			INVALID_HANDLE_VALUE, //�򿪵��ļ������ INVALID_HANDLE_VALUE�����������֧���ص� I / O �Ķ���
			NULL, //����˲���Ϊ NULL�������������µ� I/O ��ɶ˿�,��� FileHandle ������Ч���������µ� I/O ��ɶ˿�������� ���򲻻ᷢ���ļ���������� ����ɹ��������Ὣ������ص��µ� I/O ��ɶ˿ڡ�
			0 ,
			0); //����ϵͳ���������� I / O ��ɶ˿ڵ� I / O ������ݰ�������߳�������˲���Ϊ�㣬��ϵͳ������������ϵͳ�д�������һ������̡߳�
		if (m_CompletePort == NULL) return -1;
		//2.�����׽���DLL
		WSADATA	wsData;
		int errorcode = WSAStartup(MAKEWORD(2,2), &wsData);
		//����socket�׽��ֿ�ʧ��
		if (errorcode != 0) return -2;
		//3.�����׽���
		listenfd = WSASocket(AF_INET, SOCK_STREAM, 0 , NULL , 0 , WSA_FLAG_OVERLAPPED);
		if (listenfd == INVALID_SOCKET) return -3;
		//4.���÷�����ģʽ
		unsigned long u1 = 1;
		errorcode = ioctlsocket(listenfd, FIONBIO, (unsigned long*)&u1);
		if (errorcode == SOCKET_ERROR) return -4;
		//5.�رն�д�˵�����
		int size = 0;
		setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF , (char*)&size, sizeof(int));//�ر�����
		setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int));//�ر�����
		//6.��������ɶ˿ڰ�
		HANDLE handle = CreateIoCompletionPort((HANDLE)listenfd, m_CompletePort, DWORD( func::SC_WAIT_ACCEPT), 0);
		if (handle == nullptr) return -5;
		//7.���׽���
		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;//IPV4
		serAddr.sin_port = htons(func::__ServerInfo->Port); //����xml������˿ں� ��u_short������ת��Ϊ TCP/IP �����ֽ�˳�� (���� big-endian) ��
		serAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//������������IP ��u_long������ת��Ϊ TCP/IP �����ֽ�˳�� (���� big-endian) ��
		errorcode = ::bind(listenfd, (struct sockaddr*)&serAddr, sizeof(serAddr) );//������˰󶨵�ַ��Ϣ�ṹ�� �����ص�ַ���׽����������
		if (errorcode == SOCKET_ERROR) return -6;
		//8.�����׽���
		errorcode = listen(listenfd, SOMAXCONN);//�ȴ����� �Ѿ�����������ֵ�����
		if (errorcode == SOCKET_ERROR) return -7;
		//9.��������ָ��
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
		//�����ɹ�
		return 0;
	}

	//��ʼ��Ͷ��
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
		//���÷�����ģʽ
		unsigned long ul = 1;
		int errorcode = ioctlsocket( socketfd, FIONBIO, (unsigned long*)& ul );
		if (errorcode == SOCKET_ERROR)
		{
			closeSocket(socketfd, NULL, 1001);
			return -2;
		}
		//��ȡ������ճ�
		AcceptContext* context = AcceptContext::pop();
		context->m_Mode = func::SC_WAIT_ACCEPT;
		context->setSocket(listenfd, socketfd);
		unsigned long dwBytes = 0;

			//1������socket
			//2����������socket
			//3�����ܻ����� a���ͻ��˷�����һ������ b��server��ַ c��client��ַ
			//4��0����ȵ����ݵ���ֱ�ӷ��� ��0�ȴ�����
			//5�����ص�ַ��С�����ȱ���Ϊ��ַ���� + 16�ֽ�
			//6��Զ�˵�ַ��С�����ȱ���Ϊ��ַ���� + 16�ֽ�
			//7��ͬ����ʽ������ �������첽IOû�ã����ùܣ�
			//8�������ص�I / O��Ҫ�õ����ص��ṹ
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
			if( ERROR_IO_PENDING != error )//���û��Ͷ�ݳɹ����ر�socket
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
		int remoteLen = sizeof(SOCKADDR_IN);//Զ�˵�ַ����
		int localLen = sizeof(SOCKADDR_IN);//���ص�ַ����
		int errorcode = 0;

		//1��ָ�򴫵ݸ�AcceptEx�������յ�һ�����ݵĻ�����
		//2����������С������ʹ��ݸ�AccpetEx������һ��
		//3�����ص�ַ��С������ʹ��ݸ�AccpetEx����һ��
		//4��Զ�̵�ַ��С������ʹ��ݸ�AccpetEx����һ��
		//5�������������ӵı��ص�ַ
		//6���������ر��ص�ַ�ĳ���
		//7����������Զ�̵�ַ
		//8����������Զ�̵�ַ�ĳ���
		m_GetAcceptEx(//����ָ������ȡ�µ���������
			acc->m_buf,
			0,
			sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16,
			(LPSOCKADDR*)&LocalAddr,
			&localLen,
			(LPSOCKADDR*)&ClientAddr,
			&remoteLen
		);

		//���ø�������
		//������socket��listenfd�������Ը��Ƶ��µ�acc->m_Socket��
		setsockopt(acc->m_Socket, SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenfd, sizeof(listenfd));
		//���÷��ͽ��ջ�����
		int rece = func::__ServerInfo->ReceOne;
		int send = func::__ServerInfo->SendOne;
		setsockopt(acc->m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)& send, sizeof(send));
		setsockopt(acc->m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)& rece, sizeof(rece));//���������˵�ǰsocket�Ľ��ջ������ͷ��ͻ�����
		
		//����������
		int err = this->setHeartCheck(acc->m_Socket);
		if(err < 0) return -2;

		//socket�󶨵���ɶ˿�
		HANDLE handle = CreateIoCompletionPort((HANDLE)acc->m_Socket, m_CompletePort, (DWORD)func::SC_WAIT_ACCEPT, 0);
		if (handle == nullptr) return -3;

		//��ȡ����������ݵĵ�ַ
		S_CLIENT_BASE_INDEX * cindex = getClientIndex(acc->m_Socket);
		if(cindex == nullptr) return -4;

		//��ȡ�������
		S_CLIENT_BASE* c = getFreeLinker();
		if (c == nullptr)
		{
			//��ȡ�������нṹ˵������������
			LOG_MSG("Server is full!!! ...\n");
			return -5;
		}

		//c->ID = cindex->index;
		cindex->index = c->ID;//��������indexΪ��ҵ�id

		//��������
		memcpy(c->ip, inet_ntoa(ClientAddr->sin_addr), MAX_IP_LEN);//�������ַת�����ַ�����ʽ "127.0.0.1"
		c->socketfd		= acc->m_Socket;
		c->port			= ntohs(ClientAddr->sin_port); //�������ֽ���ת���ɱ����ֽ���
		c->state		= func::S_Connect;
		c->time_Connect = (int)time(NULL);
		c->time_Heart	= (int)time(NULL);

		//�����ձ�������������Ͷ�ݳ�ȥ
		int ret = this->postRecv(acc->m_Socket);
		if(ret != 0)
		{
			c->Reset();  
			return -6;
		}

		//������������
		srand(time(NULL));//�����������
		u8 rcode = rand() % 125 +1;
		//������Բ�Ҫ���ڹ����̣߳�������һ�����⣬��ʱ�Ϳͻ��˻�û�в���һ����ȫ�����ӣ�����������̷߳��
		this->begin(c->ID, CMD_RCODE);//���÷������,�ͻ������ӵ��������ˣ����ϸ��ͻ��˷���һ����Ϣ
		this->sss(c->ID, rcode);
		this->end(c->ID);
		c->rCode = rcode;//���÷����rcode=������ɵ�rcode
		updateConnect(true);
		
		if(onAcceptEvent != nullptr) this->onAcceptEvent(this, c, 0);
		AcceptContext::push(acc);//Ͷ�ݳ�ȥ��ͻ���
		this->postAccept();//Ͷ���µ�����
		
		return 0;
	}

	//Ͷ�ݽ��յ�����
	s32 TcpServer::postRecv(SOCKET s)
	{
		RecvContext* context = RecvContext::pop();
		context->m_Socket = s;
		context->m_Mode = func::SC_WAIT_RECV;
		unsigned long bytes = 0;
		unsigned long flag = 0;
		//1�� �������׽���
		//2�� ���ջ�����
		//3�� wsaBuf������WSABUF�ṹ����Ŀ
		//4�� ������ղ����������,���غ������������յ����ֽ���
		//5�� ���������׽��ֵ���Ϊ һ������Ϊ0
		//6�� �ص��ṹ
		//7�� һ��ָ����ղ�����������õ����̵�ָ��,һ����NULL
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
			if( ERROR_IO_PENDING != error )//���û��Ͷ�ݳɹ����ر�socket
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

		S_CLIENT_BASE* c = client(rece->m_Socket, true);//ͨ��socketid���������� ��ͨ���������������� 
		if(c == nullptr)
		{
			//�����߳�ֻ�رն�д�ˣ����̲߳�ͳһ�ر�socket
			shutDown(rece->m_Socket, 0, NULL ,1001 );
			RecvContext::push(rece);
			return -1;
		}
		c->threadID = tid;//�����������鿴id��Ϣ��û�������
		
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

	//��������
	s32 TcpServer::onRecv_SavaData(S_CLIENT_BASE* c, char* buf, s32 recvBytes)
	{
		if(buf == nullptr) return -1;
		//�������߳��ж�
		if(c->recv_Head == c->recv_Tail)//˵��������ϣ����³�ʼ��,�ظ�ѭ������
		{
			c->recv_Tail = 0;
			c->recv_Head = 0;
		}
		//˵������������
		if(c->recv_Tail + recvBytes > func::__ServerInfo->ReceMax) return -2;

		memcpy( &c->recvBuf[c->recv_Tail], buf, recvBytes );//���ƽ��յ�����Ϣ��recvBuf��β��
		c->recv_Tail += recvBytes; //���ƺ�β����Ҫƫ��
		return 0;
	}

	s32 TcpServer::postSend(S_CLIENT_BASE* c)
	{
		if(c->is_SendCompleted == false) return -1;
		if(c->ID < 0 || c->state == func::S_Free || c->closeState == func::S_CLOSE_SHUTDOWN || c->socketfd == INVALID_SOCKET) return -2;
		if(c->send_Tail <= c->send_Head) return -3;
		
		s32 sendBytes = c->send_Tail - c->send_Head;
		if(sendBytes <= 0) return -4;
		if(sendBytes > func::__ServerInfo->SendOne) sendBytes = func::__ServerInfo->SendOne;//���ܳ���һ������͵�����
		
		SendContext* context = SendContext::pop();//�Ӷ����ȡ��һ��
		context->m_Mode = func::SC_WAIT_SEND;
		context->setSend(c->socketfd, &c->sendBuf[c->send_Head], sendBytes);//����
		
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
			int error = WSAGetLastError();//��ʱ�������� ���صĴ�����������
			if( ERROR_IO_PENDING != error )//���û��Ͷ�ݳɹ����ر�socket
				{
				shutDown(context->m_Socket, 0, c, 1004);
				SendContext::push(context);
				return -4;
				}
		}
		//���óɹ�
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

		//������������
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

		c->send_Head += sendBytes;//���ͳɹ��˷���ͷ��Ҫƫ�Ʒ��͵��������Ĵ�С
		c->is_SendCompleted = true;
		SendContext::push(sc);
		return 0;
	}

	//****************************************************
	//�ر�socket
	s32 TcpServer::closeSocket(SOCKET sfd, S_CLIENT_BASE* c, int kind)
	{
		if (sfd == SOCKET_ERROR || sfd == INVALID_SOCKET || sfd == NULL) return -1;

		if(c != nullptr)
		{
			if (c->state == func::S_Free) return -1;//����Ѿ��ͷ��ˣ���Ϊ�˷�ֹ�ظ�����
			if(c->state >= func::S_ConnectSecure)
			{
				this->updateSecurityConnect(false);//�������Լ�1
			}
		}

		switch(kind)
		{
		case 1001:
		case 1002:
		case 3004:
			RELEASE_SOCKET(sfd);//�ͷ�socket
			break;
		default:
			this->updateConnect(false);//���˵��߻����뿪��ʱ���Ȱ�socket�ص�
			shutdown(sfd, SD_BOTH);
			RELEASE_SOCKET(sfd);//�ͷ�socket
			break;
		}

		if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, c, kind);//�ɷ���ҵ���
		/*if(c!= nullptr)//���������ݽṹ��
		{
			if(c->state == func::S_Connect || c->state == func::S_ConnectSecure)
			{
				c->Reset();//ֱ�Ӱ����socket��Ӧ�����ӵ����ݳ�ʼ����
			}
		}*/
		return 0;

	}

	//��Զ�Ĺرն�д��
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
			CancelIoEx((HANDLE)sfd, nullptr); //ȡ��IO����������socket�ϵ�Ͷ������

			if (onExceptEvent != nullptr) this->onExceptEvent(this, c, kind+300);
			return;
		}
		//�����cΪ�յĻ�����ȥ�÷����������socket��Ӧ��S_CLIENT_BASE
		auto c2 = client(sfd, true);
		if(c2 == nullptr)
		{
			LOG_MSG("find socketfd is err:%d in line %d \n", int(sfd), __LINE__);
			return;
		}
		if (c2->state == func::S_Free) return;
		if (c2->closeState == func::S_CLOSE_SHUTDOWN) return;
		//�ж�ģʽ
		switch(mode)
		{
		case func::SC_WAIT_RECV://c�ǿ�ֵ ��Ϊ�ǽ��������
			c2->is_RecvCompleted = true;
			break;
		case func::SC_WAIT_SEND://c�ǿ�ֵ ��Ϊ�Ƿ��������
			c2->is_SendCompleted = true;
			break;
		}
		c2->shutdown_kind = kind;
		c2->time_Close = (int)time(NULL);
		c2->closeState = func::S_CLOSE_SHUTDOWN;
		shutdown(sfd, SD_BOTH);
		CancelIoEx((HANDLE)sfd, nullptr); //ȡ��IO����������socket�ϵ�Ͷ������
		if (onExceptEvent != nullptr) this->onExceptEvent(this, c2, kind+400);
	}


}

#endif

