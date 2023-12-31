#include "AppTest.h"

#include <thread>

#ifndef ____WIN32_
#include<sys/timeb.h>
#endif

namespace app
{
    IContainer* __AppTest = nullptr;

#pragma pack(push,packing)
#pragma pack(1)
    struct testData
    {
        u32 curttime;
        s32 job;
        u8  aa[100];
    };
#pragma pack(pop,packing)

    AppTest::AppTest()
    {
        
    }

    AppTest::~AppTest()
    {
        
    }

    void AppTest::onInit()
    {
        
    }

    int temptime = 0;
    testData ttdata;

    void AppTest::onUpdate()
    {
        s32 tempTime = (s32)time(NULL) - temptime;
        if (temptime < 1)return;
        temptime = (s32)time(NULL);

        for(int i = 0; i< TESTCONNECT ; i++)
        {
            auto c = __TcpGame[i];
            if (c->getData()->state < func::C_ConnectSecure) continue;

            memset(&ttdata, 0, sizeof(ttdata));

#ifdef ____WIN32_
            ttdata.curttime = clock();
#else
            struct timeb tp;
            ftime(&tp);
            ttdata.curttime = tp.time;
            ttdata.job = tp.millitm;
#endif

            c->begin(1000);
            c->sss(i);
            c->sss(&ttdata, sizeof(testData));
            c->end();
        }
    }
    
    
    bool AppTest::onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
    {
  
        return false;
    }


    void ontest(net::ITcpClient* tc)
    {
        s32 index = 0;
        tc->read(index);
        tc->read(&ttdata, sizeof(testData));

        if(index == 0)
        {
#ifdef ____WIN32_
            int ftime = clock() - ttdata.curttime;
            LOG_MSG("AppTest : %d-%d-%d ftime:%d \n", ttdata.aa[0], ttdata.aa[33],ttdata.aa[99], ftime);
#else
            struct timeb tp;
            ftime(&tp);
            s32 fftime = (ttdata.curttime - tp.time) * 1000 + ttdata.job - tp.millitm;
            LOG_MSG("AppTest :ftime:%d \n", fftime);
#endif
            
        }
    }
    
    bool AppTest::onClientCommand(net::ITcpClient* tc, const u16 cmd)
    {
         auto c = tc->getData();
        if(c->state < func::C_ConnectSecure) return false;

         switch (cmd)
         {
         case 1000:
             ontest(tc);
             break;
         }
	    
    }
}


