#include"EpollServer.h"

#ifndef ____WIN32_
#include <sys/socket.h>
#include <sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>

namespace net
{
    S_CLIENT_BASE* EpollServer::getFreeLinker()
    {
        std::lock_guard<std::mutex> guard(this->m_FindLinkMutex);
        if (m_LinkIndex >= Linkers->length) m_LinkIndex = 0;
        int curindex = m_LinkIndex;
        for(int i = curindex; i<Linkers->length; i++)
        {
            m_LinkIndex++;
            auto c = Linkers->Value(i);
            if(c->state == func::S_Free)
            {
                c->Reset();
                c->ID = i;
                c->state = func::S_Connect;
                return c;
            }
        }
        return nullptr;
    }

    S_CLIENT_BASE* EpollServer::client(int socketfd, bool issecurity)
    {
        if (socketfd < 0 || socketfd >= MAX_USER_SOCKETFD) return nullptr;
        S_CLIENT_BASE_INDEX* cindex = LinkersIndex->Value(socketfd);
        if(cindex == nullptr) return nullptr;
        if (cindex->index < 0) return nullptr;
        S_CLIENT_BASE* c = Linkers->Value(cindex->index);
        if (c == nullptr)  return nullptr;
        if( issecurity )
        {
	        if( ! c->isT(socketfd))return nullptr;
        }
        return c;
    }

    S_CLIENT_BASE* EpollServer::client(int id)
    {
        if(id <0 || id>= Linkers->length) return nullptr;
        auto c = Linkers->Value(id);
        if(c == nullptr)  return nullptr;
        return c;
    }

    bool EpollServer::isID_T(const s32 id)
    {
        if (id < 0 || id >= Linkers->length) return false;
        return true;
    }

    bool EpollServer::isSecure_T(const s32 id, s32 secure)
    {
        if (id < 0 || id >= Linkers->length) return false;
        auto c = Linkers->Value(id);
        if (c == nullptr)  return false;
        if (c->state < secure) return false;

        return true;
    }

    bool EpollServer::isSecure_F_Close(const s32 id, s32 secure)
    {
        if (id < 0 || id >= Linkers->length) return false;
        auto c = Linkers->Value(id);
        if (c == nullptr)  return false;
        if (c->state >= secure) return false;
        //不是安全连接的话就关闭连接
        shutDown(c->socketfd, 0, c, 2006);
        return true;
    }

}

#endif
