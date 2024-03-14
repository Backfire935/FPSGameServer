#include "AppTest.h"

#include "../../share/ShareFunction.h"

#ifdef ____WIN32_
#include"protobuff\xxx.pb.h"
#pragma comment(lib,"libprotobufd.lib")
#pragma comment(lib,"libprotocd.lib")
#endif

namespace app
{
    IContainer* __AppTest = nullptr;


    AppTest::AppTest()
    {
        
    }

    AppTest::~AppTest()
    {
        
    }

    void AppTest::onInit()
    {
        IContainer::onInit();
    }

    void AppTest::onUpdate()
    {
        IContainer::onUpdate();
    }

    void ontestData_protobuff(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
    {
#ifdef ____WIN32_
	    u8 length = 0;
        ts->read(c->ID, length);
        char* buffer = new char[length];
        ts->read(c->ID, buffer, length);
        test::Testxxx x;
    	x.ParseFromArray(buffer, length);

        delete[] buffer;
        LOG_MSG("protobuff :%d-%d-%d \n",x.aa(),x.bb(),x.cc());
#endif
    }

//#pragma pack(push,packing)
//#pragma pack(1)
//    struct testData
//    {
//        u32 curttime;
//        s32 job;
//        u8  aa[100];
//    };
//#pragma pack(pop,packing)
//
//    void ontestData(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
//    {
//        testData ttdata;
//
//        s32 index = 0;
//        ts->read(c->ID, index);
//        ts->read(c->ID, &ttdata, sizeof(testData));
//
//
//        LOG_MSG("AppTest data:%d --id:%d / time:%f job:%d  arr:%d / %d / %d \n", index,sizeof(ttdata), ttdata.curttime, ttdata.job, ttdata.aa[0], ttdata.aa[33], ttdata.aa[99]);
//
//        ts->begin(c->ID, 1000);
//        ts->sss(c->ID,index);
//        ts->sss(c->ID, &ttdata,sizeof(testData));
//        ts->end(c->ID);
//    }

    void Test_1000(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
    {
        u32 gate_socketfd;
        u16 gate_port;
        char name[20];
        char key[20];

        ts->read(c->ID, gate_socketfd);
        ts->read(c->ID, gate_port);
        ts->read(c->ID, name, 20);
        ts->read(c->ID, key, 20);
        if (__TcpDB->getData()->state < func::C_Connect)
        {
            LOG_MSG("DB server not connect...\n");
            return;
        }

        share::S_LOGIN_1000 login;
        login.gate_socketfd     = gate_socketfd;
        login.gate_port            = gate_port;
        login.center_socketfd = c->socketfd;
        login.center_port    = c->port;


        __TcpDB->begin(1000);
        __TcpDB->sss(name, 20);
        __TcpDB->sss(key, 20);
        __TcpDB->sss(&login, sizeof(share::S_LOGIN_1000));//
        __TcpDB->end();
    }

    bool AppTest::onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd)
    {
        if(ts->isSecure_F_Close(c->ID, func::S_ConnectSecure))
        {
            LOG_MSG("AppTest err... line:%d \n ",__LINE__);
            return false;
        }

        switch (cmd)
        {
        case 1000:
            Test_1000(ts, c);
            break;
            //case 2000:
                //ontestData_protobuff(ts,c);
                //break;
        }
        
        return false;
    }

    void OnRecv_1000(net::ITcpClient* tc)
    {
        share::S_LOGIN_1000 login;
        tc->read(&login, sizeof(share::S_LOGIN_1000));

        auto c = __TcpServer->client((SOCKET)login.center_socketfd, true);
    	if (c == nullptr)
    	{
    		LOG_MSG("AppTest err... line:%d \n ", __LINE__);
			return;
		}

        __TcpServer->begin(c->ID, 1000);
        __TcpServer->sss(c->ID, &login, sizeof(share::S_LOGIN_1000));
    	__TcpServer->end(c->ID);
    }

    bool AppTest::onClientCommand(net::ITcpClient* tc, const u16 cmd)
    {
        auto c = tc->getData();
        if (c->state < func::C_ConnectSecure) return false;

        switch (cmd)
        {
	        case 1000:
                OnRecv_1000(tc);
			break;
        }

        return true;
    }
}


