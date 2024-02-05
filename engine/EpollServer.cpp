
#pragma once

#include"EpollServer.h"

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

