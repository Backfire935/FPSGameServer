#include "AppTest.h"

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
        IContainer::onInit();
    }

    void AppTest::onUpdate()
    {
        IContainer::onUpdate();
    }

    void ontestData(net::ITcpServer* ts, net::S_CLIENT_BASE* c)
    {
        testData ttdata;
        s32 index = 0;
        ts->read(c->ID, index);
        ts->read(c->ID, &ttdata, sizeof(testData));


        LOG_MSG("AppTest data:%d --id:%d/ time:%f job:%d  arr:%d/%d/%d \n", index, ttdata,ttdata.curttime, ttdata.job, ttdata.aa[0], ttdata.aa[33], ttdata.aa[99]);

        ts->begin(c->ID, 1000);
        ts->sss(c->ID,index);
        ts->sss(c->ID, &ttdata,sizeof(testData));
        ts->end(c->ID);
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
            ontestData(ts,c);
            break;
        }
        
        return false;
    }
}


