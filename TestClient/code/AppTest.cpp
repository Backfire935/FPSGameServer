#include "AppTest.h"

#include <thread>

#ifndef ____WIN32_
#include<sys/timeb.h>
#endif

#ifdef ____WIN32_
#include "protobuff\xxx.pb.h"
#pragma comment(lib,"libprotobufd.lib")
#pragma comment(lib,"libprotocd.lib")
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

    //序列化数据测试
    void sendProtoBuff(net::ITcpClient* client)
    {
	    test::Testxxx xx;
        xx.set_aa(100);
        xx.set_bb(32123);
        xx.set_cc(12321);

        int length = xx.ByteSize();
        char* c  = new char[length];
        //序列化为字符数组
        xx.SerializeToArray(c, length);

        client->begin(2000);
        client->sss((u8)length);
        client->sss(c, length);
        client->end();

        delete [] c;

    }

    int temptime = 0;
    testData ttdata;

    void AppTest::onUpdate()
    {
        s32 tempTime = (s32)time(NULL) - temptime;
        if (tempTime < 2)return;
        temptime = (s32)time(NULL);

        for(int i = 0; i< TESTCONNECT ; i++)
        {
            auto c = __TcpGame[i];
            if (c->getData()->state < func::C_ConnectSecure) 
                continue;

            memset(&ttdata, 0, sizeof(testData));

#ifdef ____WIN32_
            ttdata.curttime = clock();
            ttdata.job = 4;
#else
            struct timeb tp;
            ftime(&tp);
            ttdata.curttime = tp.time;
            ttdata.job = tp.millitm;
#endif

            ttdata.aa[0] = 11;
            ttdata.aa[33] = 34;
            ttdata.aa[99] = 93;

            c->begin(1000);
            c->sss(i);
            c->sss(&ttdata, sizeof(testData));
            c->end();

            sendProtoBuff(c);
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

        if(index == 0)//只显示第一个连入的客户端数据，避免信息过多
        {
#ifdef ____WIN32_
            int ftime = clock() - ttdata.curttime;
            LOG_MSG("AppTest : %d-%d-%d ftime:%d \n", ttdata.aa[0], ttdata.aa[33],ttdata.aa[99], ftime);
#else
            struct timeb tp;
            ftime(&tp);
            s32 fftime = (ttdata.curttime - tp.time) * 1000 + ttdata.job - tp.millitm;
            LOG_MSG("AppTest :%d-%d-%d ftime:%d \n", ttdata.aa[0], ttdata.aa[33], ttdata.aa[99], ftime);
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

         case 2000:
             ontest(tc);
             break;
         }

         return true;
    }
}


