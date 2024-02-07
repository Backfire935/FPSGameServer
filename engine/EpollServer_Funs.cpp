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

}

#endif
