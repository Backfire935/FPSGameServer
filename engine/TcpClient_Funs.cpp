
#include"TcpClient.h"
#include"IContainer.h"

#include<string.h>
 namespace net
{
     //业务层指令集合
    std::vector<IContainer*> __Commands;
     
     
     void TcpClient::initCommands()
     {
         //如果已经初始化过了 就不再进来了
        if(__Commands.size() == MAX_COMMAND_LEN) return;

         __Commands.reserve(MAX_COMMAND_LEN);
         //填充元素
         for(int i = 0; i< MAX_COMMAND_LEN; i++)
         {
             __Commands.emplace_back(nullptr);//填充空指针
         }
         
     }

     void TcpClient::registerCommand(int cmd, void* container)
     {
         if( cmd > MAX_COMMAND_LEN) return;
         IContainer * icon = (IContainer*)container;
         if(icon == nullptr) return;
         __Commands[cmd] = icon;
     }
     
     //解析指令
     void TcpClient::parseCommand()
     {
         if(socketfd < 0) return;
         auto c= getData();
         if(c->state < func::C_Connect)return;

         //1.发送心跳包
         onHeart();
         //2.解析指令
         while(c->recv_Tail - c->recv_Head > 7)
         {
             //1.解析头
             char head[2];
             head[0] = c->recvBuf[c->recv_Head] ^ c->rCode;
             head[1] = c->recvBuf[c->recv_Head +1 ] ^c->rCode;
             //如果头指令解析错误，说明不是我们定义的消息包，认为是个非法连接，需要关闭掉
             if(head[0] != func::__ClientInfo->Head[0] || head[1] != func::__ClientInfo->Head[1])
             {
                disconnectServer(2001,"head error...");
                 return;
             }
             s32 c1 = (*(u32*)(c->recvBuf + c->recv_Head + 2)) ^ c->rCode;//异或两次等于没异或，解密之前加密的信息
             u16 cmd = (*(u16*)(c->recvBuf + c->recv_Head + 6)) ^ c->rCode;

             //2.长度不够 需要等待
             if(c->recv_Tail < c->recv_Head + c1) break;//等待 直到数据的到来
             
             c->recv_TempHead = c->recv_Head +8;
             c->recv_TempTail = c->recv_Head + c1;
             //解析消息体
             parseCommand(cmd);
             if(c->state < func::C_Connect) return;
             //4.增加读取长度
             c->recv_Head += c1;//一直解析直到解析完成
             
         }
         //3.发送数据
         this->onSend();
     }

     void TcpClient::parseCommand(u16 cmd)
     {
         if(cmd < 65000)
         {
             //运行到这说明指令没有注册
             auto container = __Commands[cmd];
             if(container == nullptr)
             {
                 LOG_MSG("--------- client command not register... %d \n", cmd);
                 return;
             }
             container->onClientCommand(this, cmd);
             return;
         }

         switch (cmd)
         {
         case CMD_RCODE://发送的是加密码
             {
                 auto c = getData();
                 read(c->rCode);

                 char a[20];
                 sprintf(a, "%s_%d",func::__ClientInfo->SafeCode, c->rCode);
                 memset(c->md5, 0, sizeof(c->md5));

                 if(func::MD5str != NULL) func::MD5str(c->md5, (unsigned char*)a, strlen(a));//生成一个MD5码

                 //发送MD5码验证
                 begin(CMD_SECURITY);
                 sss(func::__ServerInfo->ID);
                 sss(func::__ServerInfo->Type);
                 sss(func::__ClientInfo->Version);
                 sss(c->md5,MAX_MD5_LEN);
                 end();
             }
             break;

         case CMD_SECURITY:
             {
                 auto c = getData();
                 u16 kind = 0;
                 read(kind);
                 if(kind > 0)
                 {
                     //1 版本不对 2 MD5错误
                     if(onExceptEvent != nullptr) onExceptEvent(this, kind);
                     break;
                 }

                 c->state = func::C_ConnectSecure;
                 if(onSecureEvent != nullptr) onSecureEvent(this,0);
             }
             break;
             
         }
     }

     //只有在产生安全连接的时候才会给服务端发送心跳包
     void TcpClient::onHeart()
     {
         auto c = getData();
         if(c->state < func::C_ConnectSecure) return;

         s32 tempTime = (s32)time(NULL) - m_data.time_Heart;
         if(tempTime >= func::__ClientInfo->HeartTime)
         {
             m_data.time_Heart = (s32)time(NULL);//刷新时间

             this->begin(CMD_HEART);//心跳包封包
             this->sss((u32)123);
             this->end();
         }
     }
     
    void TcpClient::begin(const u16 cmd)
    {
        auto c = getData();
        //头尾相等
        if(c->send_Head == c->send_Tail)
        {
            //此时没有可以发送的数据了，清零
            c->send_Head = 0;
            c->send_Tail = 0;
        }
        c->send_TempTail = c->send_Tail;
        //满足这四个条件就可以封包了
        if(c->state >= func::C_Connect &&
            c->is_Sending == false &&
           socketfd > 0 && 
            c->send_TempTail + 8 <= func::__ClientInfo->SendMax )
        {
            //开始封包
            c->is_Sending = true;
            c->sendBuf[c->send_Tail + 0] = func::__ClientInfo->Head[0] ^ c->rCode;//异或封装两个字节的消息头
            c->sendBuf[c->send_Tail + 1] = func::__ClientInfo->Head[1] ^ c->rCode;//异或封装两个字节的消息头
			
            u16 newcmd = cmd ^ c->rCode;
            char* a = (char*)&newcmd;

            c->sendBuf[c->send_Tail + 6] = a[0];//异或封装消息尾
            c->sendBuf[c->send_Tail + 7] = a[1];//异或封装消息尾

            c->send_TempTail += 8;//偏移八个字节
            return;
        }
        //发生错误的话
        disconnectServer(6001, "b error...");
    }

    void TcpClient::end()
    {
        auto c = getData();
        //错误情况
        if(c->state == func::C_Free ||
            c->is_Sending == false ||
            socketfd < 0 ||
            c->send_Tail + 8 > func::__ClientInfo->SendMax ||
            c->send_TempTail >func::__ClientInfo->SendMax ||
            c->send_Tail >= c->send_TempTail)
        {
            disconnectServer(6002,"e error...");
            return;
        }

        c->is_Sending = false;
        u32 len = (c->send_TempTail - c->send_Tail) ^ c->rCode;
        char* a = (char*)& len;
        c->sendBuf[c->send_Tail + 2] = a[0];//异或封装消息尾
        c->sendBuf[c->send_Tail + 3] = a[1];//异或封装消息尾
        c->sendBuf[c->send_Tail + 4] = a[2];//异或封装消息尾
        c->sendBuf[c->send_Tail + 5] = a[3];//异或封装消息尾

        //最后赋值结束
        c->send_Tail = c->send_TempTail;
    }

    void TcpClient::sss(const s8 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 1 < func::__ClientInfo->SendMax)
        {
            c->sendBuf[c->send_TempTail] = v;
            c->send_TempTail++;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const u8 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 1 < func::__ClientInfo->SendMax)
        {
            c->sendBuf[c->send_TempTail] = v;
            c->send_TempTail++;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const s16 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 2 < func::__ClientInfo->SendMax)
        {
            char* p = (char*)& v;
            for(int i = 0;i < 2; i++)
                c->sendBuf[c->send_TempTail + i] = p[i];
            c->send_TempTail+=2;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const u16 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 2 < func::__ClientInfo->SendMax)
        {
            char* p = (char*)& v;
            for(int i = 0;i < 2; i++)
                c->sendBuf[c->send_TempTail + i] = p[i];
            c->send_TempTail+=2;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const s32 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 4 < func::__ClientInfo->SendMax)
        {
            char* p = (char*)& v;
            for(int i = 0;i < 4; i++)
                c->sendBuf[c->send_TempTail + i] = p[i];
            c->send_TempTail+=4;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const u32 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 4 < func::__ClientInfo->SendMax)
        {
            char* p = (char*)& v;
            for(int i = 0;i < 4; i++)
                c->sendBuf[c->send_TempTail + i] = p[i];
            c->send_TempTail+=4;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const s64 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 8 < func::__ClientInfo->SendMax)
        {
            char* p = (char*)& v;
            for(int i = 0;i < 8; i++)
                c->sendBuf[c->send_TempTail + i] = p[i];
            c->send_TempTail+=8;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const u64 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 8 < func::__ClientInfo->SendMax)
        {
            char* p = (char*)& v;
            for(int i = 0;i < 8; i++)
                c->sendBuf[c->send_TempTail + i] = p[i];
            c->send_TempTail+=8;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const bool v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 1 < func::__ClientInfo->SendMax)
        {
            char* p = (char*)& v;
            c->sendBuf[c->send_TempTail] = v;
            c->send_TempTail+=1;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const f32 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 4 < func::__ClientInfo->SendMax)
        {
            char* p = (char*)& v;
            for(int i = 0;i < 4; i++)
                c->sendBuf[c->send_TempTail + i] = p[i];
            c->send_TempTail+=4;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(const f64 v)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + 8 < func::__ClientInfo->SendMax)
        {
            char* p = (char*)& v;
            for(int i = 0;i < 8; i++)
                c->sendBuf[c->send_TempTail + i] = p[i];
            c->send_TempTail+=8;
            return;
        }

        c->is_Sending = false;
    }

    void TcpClient::sss(void* v, const u32 len)
    {
        auto c = getData();
        if(c->is_Sending && c->send_TempTail + len < func::__ClientInfo->SendMax)
        {
            memcpy(&c->sendBuf[c->send_TempTail],v,len);
            c->send_TempTail +=len;
            return;
        }
        c->is_Sending = false;
        
    }

     //解包
     bool isValid(S_SERVER_BASE* c,s32 len)
     {
         if(c->state == func::C_Free || c->recv_TempTail == 0 || c->recv_TempHead + len > c->recv_TempTail )
         {
             return false;
         }
         return true;
     }
     
    void TcpClient::read(s8& v)
    {
         auto c = getData();
         if(isValid(c,1) == false)
         {
             v = 0;
             return;
         }
         v = (*(s8*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead++;
    }

    void TcpClient::read(u8& v)
    {
         auto c = getData();
         if(isValid(c,1) == false)
         {
             v = 0;
             return;
         }
         v = (*(u8*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead++;
    }

    void TcpClient::read(s16& v)
    {
         auto c = getData();
         if(isValid(c,2) == false)
         {
             v = 0;
             return;
         }
         v = (*(s16*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead+=2;
    }

    void TcpClient::read(u16& v)
    {
         auto c = getData();
         if(isValid(c,2) == false)
         {
             v = 0;
             return;
         }
         v = (*(u16*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead+=2;
    }

    void TcpClient::read(s32& v)
    {
         auto c = getData();
         if(isValid(c,4) == false)
         {
             v = 0;
             return;
         }
         v = (*(s32*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead+=4;
    }

    void TcpClient::read(u32& v)
    {
         auto c = getData();
         if(isValid(c,4) == false)
         {
             v = 0;
             return;
         }
         v = (*(u32*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead+=4;
    }

    void TcpClient::read(s64& v)
    {
         auto c = getData();
         if(isValid(c,8) == false)
         {
             v = 0;
             return;
         }
         v = (*(s64*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead+=8;
    }

    void TcpClient::read(u64& v)
    {
         auto c = getData();
         if(isValid(c,8) == false)
         {
             v = 0;
             return;
         }
         v = (*(u64*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead+=8;
    }

    void TcpClient::read(f32& v)
    {
         auto c = getData();
         if(isValid(c,4) == false)
         {
             v = 0;
             return;
         }
         v = (*(f32*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead+=4;
    }

    void TcpClient::read(f64& v)
    {
         auto c = getData();
         if(isValid(c,8) == false)
         {
             v = 0;
             return;
         }
         v = (*(f64*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead+=8;
    }

    void TcpClient::read(bool& v)
    {
         auto c = getData();
         if(isValid(c,1) == false)
         {
             v = 0;
             return;
         }
         v = (*(bool*)(c->recvBuf + c->recv_TempHead));
         c->recv_TempHead+=1;
    }

    void TcpClient::read(void* v, const u32 len)
    {
         auto c = getData();
         if(isValid(c,len) == false)
         {
             v = 0;
             return;
         }
      memcpy(v, &c->recvBuf[c->recv_TempHead], len);
         c->recv_TempHead += len;
    }


    void TcpClient::setOnConnect(TCPCLIENTNOTIFY_EVENT event)
    {
         onAcceptEvent = event;
    }

    void TcpClient::setOnSecure(TCPCLIENTNOTIFY_EVENT event)
    {
         onSecureEvent = event;
    }

    void TcpClient::setOnDisConnect(TCPCLIENTNOTIFY_EVENT event)
    {
         onDisconnectEvent = event;
    }

    void TcpClient::setOnExcept(TCPCLIENTNOTIFY_EVENT event)
    {
         onExceptEvent = event;
    }
}

