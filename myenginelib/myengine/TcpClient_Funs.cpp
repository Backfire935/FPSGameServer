
#include"TcpClient.h"
#include"IContainer.h"

#include<string.h>
 namespace net
{
     //ҵ���ָ���
    std::vector<IContainer*> __Commands;
     
     
     void TcpClient::initCommands()
     {
         //����Ѿ���ʼ������ �Ͳ��ٽ�����
        if(__Commands.size() == MAX_COMMAND_LEN) return;

         __Commands.reserve(MAX_COMMAND_LEN);
         //���Ԫ��
         for(int i = 0; i< MAX_COMMAND_LEN; i++)
         {
             __Commands.emplace_back(nullptr);//����ָ��
         }
         
     }

     void TcpClient::registerCommand(int cmd, void* container)
     {
         if( cmd > MAX_COMMAND_LEN) return;
         IContainer * icon = (IContainer*)container;
         if(icon == nullptr) return;
         __Commands[cmd] = icon;
     }
     
     //����ָ��
     void TcpClient::parseCommand()
     {
         if(socketfd < 0) return;
         auto c= getData();
         if(c->state < func::C_Connect)return;

         //1.����������
         onHeart();
         //2.����ָ��
         while(c->recv_Tail - c->recv_Head > 7)
         {
             //1.����ͷ
             char head[2];
             head[0] = c->recvBuf[c->recv_Head] ^ c->rCode;
             head[1] = c->recvBuf[c->recv_Head +1 ] ^c->rCode;
             //���ͷָ���������˵���������Ƕ������Ϣ������Ϊ�Ǹ��Ƿ����ӣ���Ҫ�رյ�
             if(head[0] != func::__ClientInfo->Head[0] || head[1] != func::__ClientInfo->Head[1])
             {
                disconnectServer(2001,"head error...");
                 return;
             }
             s32 c1 = (*(u32*)(c->recvBuf + c->recv_Head + 2)) ^ c->rCode;//������ε���û��򣬽���֮ǰ���ܵ���Ϣ
             u16 cmd = (*(u16*)(c->recvBuf + c->recv_Head + 6)) ^ c->rCode;

             //2.���Ȳ��� ��Ҫ�ȴ�
             if(c->recv_Tail < c->recv_Head + c1) break;//�ȴ� ֱ�����ݵĵ���
             
             c->recv_TempHead = c->recv_Head +8;
             c->recv_TempTail = c->recv_Head + c1;
             //������Ϣ��
             parseCommand(cmd);
             if(c->state < func::C_Connect) return;
             //4.���Ӷ�ȡ����
             c->recv_Head += c1;//һֱ����ֱ���������
             
         }
         //3.��������
         this->onSend();
     }

     void TcpClient::parseCommand(u16 cmd)
     {
         if(cmd < 65000)
         {
             //���е���˵��ָ��û��ע��
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
         case CMD_RCODE://���͵��Ǽ�����
             {
                 auto c = getData();
                 read(c->rCode);

                 char a[20];
                 sprintf(a, "%s_%d",func::__ClientInfo->SafeCode, c->rCode);
                 memset(c->md5, 0, sizeof(c->md5));

                 if(func::MD5str != NULL) func::MD5str(c->md5, (unsigned char*)a, strlen(a));//����һ��MD5��

                 //����MD5����֤
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
                     //1 �汾���� 2 MD5����
                     if(onExceptEvent != nullptr) onExceptEvent(this, kind);
                     break;
                 }

                 c->state = func::C_ConnectSecure;
                 if(onSecureEvent != nullptr) onSecureEvent(this,0);
             }
             break;
             
         }
     }

     //ֻ���ڲ�����ȫ���ӵ�ʱ��Ż������˷���������
     void TcpClient::onHeart()
     {
         auto c = getData();
         if(c->state < func::C_ConnectSecure) return;

         s32 tempTime = (s32)time(NULL) - m_data.time_Heart;
         if(tempTime >= func::__ClientInfo->HeartTime)
         {
             m_data.time_Heart = (s32)time(NULL);//ˢ��ʱ��

             this->begin(CMD_HEART);//���������
             this->sss((u32)123);
             this->end();
         }
     }
     
    void TcpClient::begin(const u16 cmd)
    {
        auto c = getData();
        //ͷβ���
        if(c->send_Head == c->send_Tail)
        {
            //��ʱû�п��Է��͵������ˣ�����
            c->send_Head = 0;
            c->send_Tail = 0;
        }
        c->send_TempTail = c->send_Tail;
        //�������ĸ������Ϳ��Է����
        if(c->state >= func::C_Connect &&
            c->is_Sending == false &&
           socketfd > 0 && 
            c->send_TempTail + 8 <= func::__ClientInfo->SendMax )
        {
            //��ʼ���
            c->is_Sending = true;
            c->sendBuf[c->send_Tail + 0] = func::__ClientInfo->Head[0] ^ c->rCode;//����װ�����ֽڵ���Ϣͷ
            c->sendBuf[c->send_Tail + 1] = func::__ClientInfo->Head[1] ^ c->rCode;//����װ�����ֽڵ���Ϣͷ
			
            u16 newcmd = cmd ^ c->rCode;
            char* a = (char*)&newcmd;

            c->sendBuf[c->send_Tail + 6] = a[0];//����װ��Ϣβ
            c->sendBuf[c->send_Tail + 7] = a[1];//����װ��Ϣβ

            c->send_TempTail += 8;//ƫ�ư˸��ֽ�
            return;
        }
        //��������Ļ�
        disconnectServer(6001, "b error...");
    }

    void TcpClient::end()
    {
        auto c = getData();
        //�������
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
        c->sendBuf[c->send_Tail + 2] = a[0];//����װ��Ϣβ
        c->sendBuf[c->send_Tail + 3] = a[1];//����װ��Ϣβ
        c->sendBuf[c->send_Tail + 4] = a[2];//����װ��Ϣβ
        c->sendBuf[c->send_Tail + 5] = a[3];//����װ��Ϣβ

        //���ֵ����
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

     //���
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
