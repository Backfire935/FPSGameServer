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
    //初始化函数指针
    onAcceptEvent   =nullptr;
    onSecureEvent   =nullptr;
    onDisconnectEvent   =nullptr;
    onExceptEvent   =nullptr;
    }
    
    TcpClient::~TcpClient()
    {
#ifdef ____WIN32_
        RELEASE_SOCKET(socketfd);
        WSACleanup();//释放套接字资源 
#else
        close(socketfd);
#endif
    }

    //运行客户端入口
    void TcpClient::runClient(u32 sid, char* ip, int port)
    {
        m_data.Init(sid);
        m_data.time_AutoConnect = 0;
        if( ip != NULL) strcpy(m_data.ip, ip);
        if(port > 0) m_data.port = port;
        //初始化socket
        s32 err = initSocket();
        if(err < 0)
        {
            LOG_MSG("----------client is error:%d",err);
            return;
        }
        //运行
        runThread();
        //初始化指令集
        initCommands();
        
    }

    bool TcpClient::setNonblockingSocket()
    {
#ifdef ____WIN32_
    unsigned long u1 = 1;
    s32 errorcode = ioctlsocket(socketfd, FIONBIO, (unsigned long*)& u1);//设置socket为非阻塞
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
        if(fcntl(socketfd, F_SETFL, flags) < 0) return false; //F_SETFL设置文件标志 flags设置文件
        return true;
#endif
        
    }
    
    s32 TcpClient::initSocket()
    {
#ifdef ____WIN32_
        //1.初始化Windows sockets dll
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
        //2.设置套接字非阻塞模式
        setNonblockingSocket();

        //3.设置发送接收缓冲区
        int rece = func::__ClientInfo->ReceOne;
        int send = func::__ClientInfo->SendOne;
        setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char*)& rece, sizeof(rece));
        setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (char*)& send, sizeof(send));

        return 0;
    }

    //自动连接
    void TcpClient::onAutoConnect()
    {
        if(m_data.port < 1000) return;
        
        auto c = getData();
        s32 tempTime = (s32)time(NULL) - m_data.time_AutoConnect;//time_AutoConnect初始化是0
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
        //接下来获取服务器IP地址
#ifdef ____WIN32_
        addrServer.sin_addr.S_un.S_addr = inet_addr(m_data.ip);//本机字节序转换为网络字节序 windows下的转换方法
#else
        addrServer.sin_addr.s_addr = inet_addr(m_data.ip);//Linux下的转换方法
#endif
        addrServer.sin_family = AF_INET;
        addrServer.sin_port = htons( m_data.port );//使用网络字节序
        //连接服务器
        int value = connect(socketfd, (struct sockaddr*)& addrServer, sizeof(addrServer));
        if(value == 0)//为0表示连接成功
        {
            m_data.state = func::C_Connect;//连接成功后将状态设置成连接成功状态
            if(onAcceptEvent != nullptr) onAcceptEvent(this,1);
            return true;
        }

        if(value < 0)
        {
#ifdef ____WIN32_
            int err = WSAGetLastError();
            //指定了一个无效参数
            if(err == WSAEINVAL)
            {
                LOG_MSG("----------------cleint 指定了一个无效参数.%d-%d\n", m_data.serverID, m_data.port);
                return false;
            }
            //这个操作不能马上完成,正在连接中，是一个正常情况
            if(err == WSAEWOULDBLOCK)
            {
                connect_Select();
                return false;
            }
            //连接已经完成
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
            //这个操作不能马上完成,正在连接中，是一个正常情况
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
        LOG_MSG("失去与服务器连接，原因%s, m_data.state=%d,socketfd=%d\n",err,m_data.state, socketfd);
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
            case EINTR://信号
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

    //生产者 保存数据
    int TcpClient::onSaveData(int recvBytes)
    {
        auto c = this->getData();
        if(c->recv_Tail == c->recv_Head)
        {
            c->recv_Tail = 0;
            c->recv_Head = 0;
        }
        if(c->recv_Tail + recvBytes >=func::__ClientInfo->ReceMax)//缓存溢出
        {
            printf("------------client .rece full %d-%d.. \n", c->recv_Head, c->recv_Tail);
            return -1;
        }
        memcpy(&c->recvBuf[c->recv_Tail], c->recvBuf_Temp, recvBytes);//拷贝数据
        c->recv_Tail += recvBytes;//偏移
        
        return 0;
    }
    
    int TcpClient::onSend()
    {
        auto c = this->getData();
        if(c->send_Tail <= c->send_Head) return 0;
        if(c->state < func::C_Connect) return -1;

        int sendlen = c->send_Tail - c->send_Head;//尾偏移量 - 头偏移量
        if(sendlen < 1) return -1;
        int sendbytes =  send(socketfd, &c->sendBuf[c->send_Head], sendlen, 0);
        if (sendbytes > 0)//发送成功
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
            case EINTR://信号
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

