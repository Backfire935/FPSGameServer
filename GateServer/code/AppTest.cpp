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
    void Test_1000(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
    {
        char name[20];
        char key[20];
        ts->read(c->ID, name, 20);
        ts->read(c->ID, key, 20);
        if(__TcpCenter->getData()->state < func::C_Connect)
        {
            LOG_MSG("Center server not connect...\n");
            return;
        }

        __TcpCenter->begin(1000);
        __TcpCenter->sss((u32)c->socketfd);
        __TcpCenter->sss(c->port);
        __TcpCenter->sss(name, 20);
        __TcpCenter->sss(key, 20);
    	__TcpCenter->end();
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
            Test_1000(ts,c);
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
        auto c = __TcpServer->client((SOCKET)login.gate_socketfd, true);
        LOG_MSG("%d %d %d %d \n ", login.center_port, login.center_socketfd, login.memid,login.rolenum);
        if (c == nullptr || c->port != login.gate_port )
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


