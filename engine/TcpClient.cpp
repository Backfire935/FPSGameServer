#include "TcpClient.h"

#ifndef ____WIN32_
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include<string.h>

namespace net
{
    TcpClient::TcpClient()
    {
#ifdef ____WIN32_
        socketfd = INVALID_SOCKET;
#else
        socketfd = -1;
#endif
    //��ʼ������ָ��
    onAcceptEvent   =nullptr;
    onSecureEvent   =nullptr;
    onDisconnectEvent   =nullptr;
    onExceptEvent   =nullptr;
    }
    
    TcpClient::~TcpClient()
    {
#ifdef ____WIN32_
        RELEASE_SOCKET(socketfd);
        WSACleanup();//�ͷ��׽�����Դ 
#else
        close(socketfd);
#endif
    }

    //���пͻ������
    void TcpClient::runClient(u32 sid, char* ip, int port)
    {
        m_data.Init(sid);
        m_data.time_AutoConnect = 0;
        if( ip != NULL) strcpy(m_data.ip, ip);
        if(port > 0) m_data.port = port;
        //��ʼ��socket
        s32 err = initSocket();
        if(err < 0)
        {
            LOG_MSG("----------client is error:%d",err);
            return;
        }
        //����
        runThread();
        //��ʼ��ָ�
        initCommands();
        
    }

    bool TcpClient::setNonblockingSocket()
    {
#ifdef ____WIN32_
    unsigned long u1 = 1;
    s32 errorcode = ioctlsocket(socketfd, FIONBIO, (unsigned long*)& u1);//����socketΪ������
        if(errorcode == SOCKET_ERROR)
        {
            LOG_MSG("--------------- client errorcode is error.. %d line:%d",errorcode, __LINE__);
            return false;
        }
        return true;
#else
        int flags = fcntl(socketfd, F_GETFL);
        if(flags < 0) return false;
        flags |= O_NONBLOCK;
        if(fcntl(socketfd, F_SETFL, flags) < 0) return false; //F_SETFL�����ļ���־ flags�����ļ�
        return true;
#endif
        
    }
    
    s32 TcpClient::initSocket()
    {
#ifdef ____WIN32_
        //1.��ʼ��Windows sockets dll
        WSADATA wsData;
        s32 errorcode = WSAStartup(MAKEWORD(2,2), &wsData);
        if(errorcode !=0)
        {
            WSACleanup();
            return -1;
        }
        
#endif
        socketfd = socket(AF_INET, SOCK_STREAM, 0);
        m_data.socketfd = socketfd;
        //printf("socketfd is %ld", socketfd);
        if(socketfd < 0)
        {
            int fd = socketfd;
            LOG_MSG("--------------------- client socket is error..%d line:%d",fd, __LINE__);
            return -1;
        }
        //2.�����׽��ַ�����ģʽ
        setNonblockingSocket();

        //3.���÷��ͽ��ջ�����
        int rece = func::__ClientInfo->ReceOne;
        int send = func::__ClientInfo->SendOne;
        setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char*)& rece, sizeof(rece));
        setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (char*)& send, sizeof(send));

        return 0;
    }

    //�Զ�����
    void TcpClient::onAutoConnect()
    {
        if(m_data.port < 1000) return;
        
        auto c = getData();
        s32 tempTime = (s32)time(NULL) - m_data.time_AutoConnect;//time_AutoConnect��ʼ����0
        if(tempTime >= func::__ClientInfo->AutoTime)
        {
            c->reset();
            int fd = socketfd;
            LOG_MSG("-----------client autoConnect...%d threadid:%d \n", fd, getData()->ID);

            connectServer();
            m_data.time_AutoConnect = (s32)time(NULL);
        }
    }

    bool TcpClient::connectServer()
    {
        if(m_data.port < 1000) return false;
        m_data.state = func::C_ConnectTry;
        struct sockaddr_in addrServer;
        //��������ȡ������IP��ַ
#ifdef ____WIN32_
        addrServer.sin_addr.S_un.S_addr = inet_addr(m_data.ip);//�����ֽ���ת��Ϊ�����ֽ��� windows�µ�ת������
#else
        addrServer.sin_addr.s_addr = inet_addr(m_data.ip);//Linux�µ�ת������
#endif
        addrServer.sin_family = AF_INET;
        addrServer.sin_port = htons( m_data.port );//ʹ�������ֽ���
        //���ӷ�����
        int value = connect(socketfd, (struct sockaddr*)& addrServer, sizeof(addrServer));
        if(value == 0)//Ϊ0��ʾ���ӳɹ�
        {
            m_data.state = func::C_Connect;//���ӳɹ���״̬���ó����ӳɹ�״̬
            if(onAcceptEvent != nullptr) onAcceptEvent(this,1);
            return true;
        }

        if(value < 0)
        {
#ifdef ____WIN32_
            int err = WSAGetLastError();
            //ָ����һ����Ч����
            if(err == WSAEINVAL)
            {
                LOG_MSG("----------------cleint ָ����һ����Ч����.%d-%d\n", m_data.serverID, m_data.port);
                return false;
            }
            //������������������,���������У���һ���������
            if(err == WSAEWOULDBLOCK)
            {
                connect_Select();
                return false;
            }
            //�����Ѿ����
            if(err == WSAEISCONN)
            {
                m_data.state = func::C_Connect;
                if(onAcceptEvent != nullptr) onAcceptEvent(this,2);
                return true;
            }
#else
            if(errno == EINTR || errno == EAGAIN )
            {
                return false;
            }
            //������������������,���������У���һ���������
            if(errno == EINPROGRESS)
            {
                connect_Select();
                return false;
            }
            if(errno == EISCONN)
            {
                m_data.state = func::C_Connect;
                if(onAcceptEvent != nullptr) onAcceptEvent(this,2);
                return true;
            }
#endif
        }
        this->disconnectServer(1009, "[connect fail]");
        return false;
    }

    void TcpClient::disconnectServer(const s32 errcode, const char* err)
    {
        LOG_MSG("ʧȥ����������ӣ�ԭ��%s, m_data.state=%d,socketfd=%d\n",err,m_data.state, socketfd);
        if( m_data.state == func::C_Free ) return;
        if(socketfd == -1) return;

#ifdef  ____WIN32_
        if (socketfd == INVALID_SOCKET) return;
        RELEASE_SOCKET(socketfd);
#else
        close(socketfd);
        socketfd = -1;
#endif

        m_data.reset();
        initSocket();
    }

   

    ITcpClient* NewTcpClient()
    {
        return new TcpClient();
    }
    

    void TcpClient::connect_Select()
    {
    }



    int TcpClient::onRecv()
    {
        auto c = this->getData();
        memset(c->recvBuf_Temp, 0, func::__ClientInfo->ReceOne);
        int receBytes = recv(socketfd, c->recvBuf_Temp, func::__ClientInfo->ReceOne, 0);
        if(receBytes > 0)
        {
            int err = onSaveData(receBytes);
            if(err < 0)
            {
                this->disconnectServer(1007, "recvFull...");
                return err;
            }
            return 0;
        }
        if(receBytes == 0)
        {
            this->disconnectServer(1003, "receBytes=0...");
            return -1;
        }
#ifdef ____WIN32_
        if(receBytes < 0)
        {
            int err = WSAGetLastError();
            switch(err)
            {
            case WSAEINTR:
            case WSAEWOULDBLOCK:
                return 0;
            default:
                LOG_MSG("1.recv error...%d-%d \n", (int)c->socketfd, err);
                this->disconnectServer(1001, "[recv error]");
                return -1;
            }
        }
#else
        if(receBytes < 0)
        {
            switch(errno)
            {
            case EINTR://�ź�
            case EAGAIN:
                return 0;
                defualt:
                this->disconnectServer(1001, "server close-1");
                return -1;
            }
        }
#endif
        return 0;
    }

    //������ ��������
    int TcpClient::onSaveData(int recvBytes)
    {
        auto c = this->getData();
        if(c->recv_Tail == c->recv_Head)
        {
            c->recv_Tail = 0;
            c->recv_Head = 0;
        }
        if(c->recv_Tail + recvBytes >=func::__ClientInfo->ReceMax)//�������
        {
            printf("------------client .rece full %d-%d.. \n", c->recv_Head, c->recv_Tail);
            return -1;
        }
        memcpy(&c->recvBuf[c->recv_Tail], c->recvBuf_Temp, recvBytes);//��������
        c->recv_Tail += recvBytes;//ƫ��
        
        return 0;
    }
    
    int TcpClient::onSend()
    {
        auto c = this->getData();
        if(c->send_Tail <= c->send_Head) return 0;
        if(c->state < func::C_Connect) return -1;

        int sendlen = c->send_Tail - c->send_Head;//βƫ���� - ͷƫ����
        if(sendlen < 1) return -1;
        int sendbytes =  send(socketfd, &c->sendBuf[c->send_Head], sendlen, 0);
        if (sendbytes > 0)//���ͳɹ�
        {
            c->send_Head += sendbytes;
            return 0;
        }
#ifdef ____WIN32_
        int err = WSAGetLastError();
        switch(err)
        {
        case WSAEINTR:
        case WSAEWOULDBLOCK:
            return 0;
        default:
            LOG_MSG("2.recv error...%d-%d \n", (int)c->socketfd, err);
            this->disconnectServer(4002, "send close-1");
            return -1;
        }
#else
        if(sendbytes < 0)
        {
            switch(errno)
            {
            case EINTR://�ź�
            case EAGAIN:
                return 0;
                defualt:
                this->disconnectServer(4002, "send close-1");
                return -1;
            }
        }
#endif
      
        return 0;
        
    }
    




    

}

