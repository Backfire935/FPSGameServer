#include "TcpServer.h"

#ifdef  ____WIN32_

#include "IOPool.h"

std::vector<IContainer*> __Commands;

namespace net
{

	void TcpServer::initCommands()
	{
		__Commands.reserve(MAX_COMMAND_LEN);//�����ڴ�ռ� ���������������С�洢���ȡ�
		for(int i=0; i< MAX_COMMAND_LEN; i++)//��64λϵͳ�� ָ��ռ�˸��ֽ�
		{
			__Commands.emplace_back(nullptr);//��һ���͵ع����Ԫ����ӵ�����ĩβ��
		}
	}

	//ע����Ϣ
	void TcpServer::registerCommand(int cmd, void* container)
	{
		if(cmd >= MAX_COMMAND_LEN ) return;
		IContainer* icon = (IContainer*)container;
		if(icon == nullptr) return;
		__Commands[cmd] = icon;
		
	}

	//**********************************************
	//����� ��ҵ�����µ� ÿ֡���µ�
	void TcpServer::parseCommand()//����ָ��
	{
		//�������е��������
		for(int i =0; i<Linkers->length; i++)
		{
			//�������һ������������״̬ �Ͳ��������������������
			auto c = Linkers->Value(i);
			if(c->ID == -1) continue;//û�����
			if(c->state == func::S_Free)continue;
			if(c->state == func::S_NeedSave) continue;

			checkConnect(c);
			if(c->closeState == func::S_CLOSE_SHUTDOWN) continue;

			parseCommand(c);
			//���÷�������
			this->postSend(c);
		}
	}

	void TcpServer::parseCommand(S_CLIENT_BASE* c)
	{
		if(c->is_RecvCompleted == false) return;

		while(c->recv_Tail - c->recv_Head > 7)//���˸��ֽڶ�û�� ֤������һ�������İ�
		{
			char head[2];//�����ֽڵ���Ϣͷ
			head[0] = c->recvBuf[c->recv_Head] ^ c->rCode;
			head[1] = c->recvBuf[c->recv_Head +1] ^ c->rCode;
			if(head[0] != func::__ServerInfo->Head[0] || head[1] != func::__ServerInfo->Head[1])
			{
				shutDown(c->socketfd, 0, c, 2001);
				return;
			}
			//һ���������ͬһ�����ڶ��ε���û���
			u32 length	=   (*(u32*) (c->recvBuf + c->recv_Head + 2 )) ^ c->rCode;//ͨ��u32ָ���ٽ�����û�ԭ������ֵ �����ֽڵ���Ϣͷ �ĸ��ֽڵ���Ϣ����
			u16 cmd		=	(*(u16*) (c->recvBuf + c->recv_Head + 6 )) ^ c->rCode;//�����ֽڵ�ͷָ��
			if(c->recv_Tail < c->recv_Head + length) break;//˵������ͷָ���ǲ�ȱ��,��Ϣ������������Ҫ�˳�ȥֱ�����ݵ���

			//��Ϣͷ��Ϣ����ͷָ��������� ������������Ϣ��
			c->recv_TempHead = c->recv_Head + 8;
			c->recv_TempTail = c->recv_Head + length;//������Ϣ��ж೤

			//������Ϣ��
			parseCommand(c,cmd);
			if(c->state < func::S_Connect)
			{
				return;
			}

			//�����ɹ�
			c->recv_Head += length;//c->recv_Head׷�ϵ�c->recv_TempTail
			
		}
		c->is_RecvCompleted = false;
	}

	int times = 1;
	//ҵ���߼���
	void TcpServer::parseCommand(S_CLIENT_BASE* c, u16 cmd)
	{
		c->time_Heart = (int)time(NULL);//�յ��κ���Ϣ ����������������ʱ��

		if(cmd <65000)//˵����һ��ҵ���̵߳�ͷָ��
		{
			
			if(cmd == CMD_HEART)
			{
				c->time_Heart = (int)time(NULL);
				u32 value = 0;
				read(c->ID, value);

				//�ж��Ƿ��ǿͻ��˺�����֮����е�������
				if(value == 1)//����ҿͻ��˷��͵�������ֵ��1
				{
					LOG_MSG("�յ������� %d ����:%d ���ǵ�%d��\n",value,c->ID,times);
					times++;
					begin(c->ID, CMD_HEART);//����˷���������
					sss(c->ID, 1);
					end(c->ID);
				}
				
				//ͨ��ע����Ϣ�ķ�ʽ �ɷ���ҵ���߼���
				return;
			}

			auto container =__Commands[cmd];
			if(container == nullptr)
			{
				LOG_MSG("command not register... %d \n", cmd);
				return;
			}

			//��������ķ��� ����Ϣ��ת�ӵ��ⲿҵ���
			container->onServerCommand(this, c, cmd);
			return;
		}

		switch(cmd)
		{
		case CMD_SECURITY://��ȫ���� ���������Զ�����Ϣ���Э��淶
			char a[20];
			sprintf_s(a, "%s_%d", func::__ServerInfo->SafeCode, c->rCode);//��ȫ��ͼ������������һ���ַ�������
			memset(c->md5, 0, sizeof(c->md5));//�ȸ�ʽ��
			if(func::MD5str != NULL) func::MD5str(c->md5, (unsigned char*)a, strlen(a));

			char str5[MAX_MD5_LEN];
			memset(str5, 0, MAX_MD5_LEN);

			//�������������ڷֲ�ʽ�͸��ؾ���������,�������ͷ�����֮��ͨ�Ż�����
			u32 version = 0;
			u32 c_id = 0;
			u8 c_type = 0;
			
			read(c->ID, c_id);//����
			read(c->ID, c_type);//
			read(c->ID, version);//
			read(c->ID, str5, MAX_MD5_LEN);//

			if(version != func::__ServerInfo->Version)//�汾�Ų���ȵĻ�
			{
				//���������߳̽����ģ����Կ��Է��
				//���ͻ��˷���һ�����ݻ�ȥ
				begin(c->ID, CMD_SECURITY);
				sss(c->ID, (u16)1 );//���ؿͻ���1����汾��һ��
				end(c->ID);
				return;
			}

			//����MD5����֤�Ϳ�����Ϊ��һ����Ծ��������
			int error = stricmp(c->md5, str5);//�ַ���ƥ�䣬ƥ�����˷��ص���0
			if(error != 0)
			{
				begin(c->ID, CMD_SECURITY);
				sss(c->ID, (u16)2 );//����2����MD5��ȫ����֤ûͨ��
				end(c->ID);
				return;
			}

			//֤����һ����ȫ������,���治���ٽ��������ӵ��ж�
			c->state = func::S_ConnectSecure;
			begin(c->ID, CMD_SECURITY);
			sss(c->ID, (u16)0 );//����0����ȫ���ӳɹ�
			end(c->ID);

			c->ClientID = c_id;
			c->ClientType = c_type;

			this->updateSecurityConnect(true);//���µ�ǰ��ȫ��������

			if(onSecureEvent != nullptr) this->onSecureEvent(this, c,0);//�ɷ�һ����ȫ�����¼���ҵ���
			break;
		}
		
	}

	void TcpServer::checkConnect(S_CLIENT_BASE* c)
	{
		s32 temp = 0;
		if(c->closeState == func::S_CLOSE_SHUTDOWN)//�Ѿ��رյĻ�
		{
			temp = (int)time(NULL) - c->time_Close;
			if(c->is_RecvCompleted && c->is_SendCompleted)
			{
				closeSocket(c->socketfd, c, 2001);//�Ѿ��ر����ӵĻ����ͷ�socket
			}
			else if(temp > 10000)//�����������û�н��ջ��߷�����ϵĻ� ��ʱ
			{
				closeSocket(c->socketfd, c, 2005);//�Ѿ��ر����ӵĻ����ͷ�socket
			}
			return;
		}
		
		//��鰲ȫ���� 
		temp = (int)time(NULL) - c->time_Connect;
		if(c->state == func::S_Connect )
		{
			if(temp > 100)//�������ʮ�뻹��������״̬�Ļ�
			{
				if(this->onTimeOutEvent != nullptr ) this->onTimeOutEvent(this, c, 2102);//�ɷ���ʱ�¼���ȥ
				shutDown(c->socketfd, 0, c, 2102);
				return;
			}
		}

		//���������
		temp = (int)time(NULL) - c->time_Heart;
		if(temp > func::__ServerInfo->HeartTime)
		{
			if(this->onTimeOutEvent != nullptr ) this->onTimeOutEvent(this, c, 2003);//�ɷ���ʱ�¼���ȥ
			shutDown(c->socketfd, 0, c, 2003);
			return;
		}
		
	}
	
	S_CLIENT_BASE* TcpServer::client(SOCKET _socketfd, bool issecurity)
	{
		if(_socketfd < 0 || _socketfd >= MAX_USER_SOCKETFD) return nullptr;
		auto cindex = LinkerIndexs->Value(_socketfd);
		if(cindex == nullptr) return nullptr;
		if(cindex->index < 0) return nullptr;
		auto c = Linkers->Value(cindex->index);
		if(c == nullptr)
		{
			LOG_MSG("client c == null %d-%d line:%d", (int)_socketfd, cindex->index, __LINE__);
			return nullptr;
		}
		if(issecurity)
		{
			//���socketfd��c����socketid�ǲ���һ��ֵ
			if(!c->isT(_socketfd)) return nullptr;
		}
		
		return c;
		
	}

	S_CLIENT_BASE* TcpServer::client(const int id)
	{
		if(id < 0 || id >= Linkers->length) return nullptr;
		
		auto c = Linkers->Value(id);
		return c;
	}

	bool TcpServer::isID_T(const s32 id)
	{
		if(id <0 || id >= Linkers->length)return false;//���ID�±�ĺϷ���
		return true;
	}

	bool TcpServer::isSecure_T(const s32 id, s32 secure)
	{
		if(id <0 || id >= Linkers->length)return false;//���ID�±�ĺϷ���
		auto c = Linkers->Value(id);
		if(c->state < secure) return false;//С�ڰ�ȫ״̬

		return true;
	}

	bool TcpServer::isSecure_F_Close(const s32 id, s32 secure)
	{
		if(id <0 || id >= Linkers->length)return false;//���ID�±�ĺϷ���
		auto c = Linkers->Value(id);
		if(c->state >= secure) return false;//���ڰ�ȫ״̬

		//��Ȼ��Ҫ�ر�
		shutDown(c->socketfd, 0, c, 2006);
		return true;
	}



	void TcpServer::getSecurityCount(int& connectnum, int& securitycount)
	{
		connectnum		= m_ConnectCount;
		securitycount	= m_SecurityCount;
		
	}

	//������������
	void TcpServer::begin(const int id, const u16 cmd)
	{
		auto c = client(id);
		if(c== nullptr)return;
		if(c->send_Head == c->send_Tail)
		{
			//��ʱû�п��Է��͵������ˣ�����
			c->send_Head = 0;
			c->send_Tail = 0;
		}
		c->send_TempTail = c->send_Tail;
		//�������ĸ������Ϳ��Է����
		if(c->state > func::S_Free && c->is_Sending == false && c->socketfd != INVALID_SOCKET && c->send_TempTail + 8 <= func::__ServerInfo->SendMax )
		{
			c->is_Sending = true;
			c->sendBuf[c->send_Tail + 0] = func::__ServerInfo->Head[0] ^ c->rCode;//����װ�����ֽڵ���Ϣͷ
			c->sendBuf[c->send_Tail + 1] = func::__ServerInfo->Head[1] ^ c->rCode;//����װ�����ֽڵ���Ϣͷ
			//if (cmd != 65001 && cmd != 65002) printf("\n");
			u16 newcmd = cmd ^ c->rCode;
			char* a = (char*)&newcmd;

			c->sendBuf[c->send_Tail + 6] = a[0];//����װ��Ϣβ
			c->sendBuf[c->send_Tail + 7] = a[1];//����װ��Ϣβ

			c->send_TempTail += 8;//ƫ�ư˸��ֽ�
			return;
		}
		//��������Ļ�
		shutDown(c->socketfd, 0, c, 2004);
		
	}

	void TcpServer::end(const int id)
	{
		auto c = client(id);
		if(c== nullptr)return;

		//������
		if(c->is_Sending == false || c->send_Tail +8 > func::__ServerInfo->SendMax || c->send_TempTail +8 >func::__ServerInfo->SendMax || c->send_Tail >= c->send_TempTail)
		{
			shutDown(c->socketfd, 0, c, 2005);
			return;
		}

		c->is_Sending = false;//����ɹ�
		//������Ϣ����
		u32 len = (c->send_TempTail - c->send_Tail) ^ c->rCode;//����ץ��
		char* a = (char*)&len;
		//������Ϣ����
		c->sendBuf[c->send_Tail + 2] = a[0];//����װ��Ϣβ
		c->sendBuf[c->send_Tail + 3] = a[1];//����װ��Ϣβ
		c->sendBuf[c->send_Tail + 4] = a[2];//����װ��Ϣβ
		c->sendBuf[c->send_Tail + 5] = a[3];//����װ��Ϣβ

		c->send_Tail = c->send_TempTail;//�����β��ƫ����Ϣ��ĳ���ָ������λ��
	}

	void TcpServer::sss(const int id, const s8 v)
	{
		//һ���ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +1 <= func::__ServerInfo->SendMax)
		{
			c->sendBuf[c->send_TempTail] = v ; // v ^ c->rCode
			c->send_TempTail ++;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const u8 v)
	{
		//һ���ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +1 <= func::__ServerInfo->SendMax)
		{
			c->sendBuf[c->send_TempTail] = v;
			c->send_TempTail ++;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const s16 v)
	{
		//�����ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +2 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//����д��for������д��Ϊ��ֱ��ѧϰ
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->send_TempTail +=2;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const u16 v)
	{
		//�����ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +2 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//����д��for������д��Ϊ��ֱ��ѧϰ
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->send_TempTail +=2;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const s32 v)
	{
		//�ĸ��ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +4 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//����д��for������д��Ϊ��ֱ��ѧϰ
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->sendBuf[c->send_TempTail+2] = p[2];
			c->sendBuf[c->send_TempTail+3] = p[3];
			c->send_TempTail +=4;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const u32 v)
	{
		//�ĸ��ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +4 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//����д��for������д��Ϊ��ֱ��ѧϰ
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->sendBuf[c->send_TempTail+2] = p[2];
			c->sendBuf[c->send_TempTail+3] = p[3];
			c->send_TempTail +=4;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const s64 v)
	{
		//�˸��ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +8 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//����д��for������д��Ϊ��ֱ��ѧϰ
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->sendBuf[c->send_TempTail+2] = p[2];
			c->sendBuf[c->send_TempTail+3] = p[3];
			c->sendBuf[c->send_TempTail+4] = p[4];
			c->sendBuf[c->send_TempTail+5] = p[5];
			c->sendBuf[c->send_TempTail+6] = p[6];
			c->sendBuf[c->send_TempTail+7] = p[7];
			c->send_TempTail +=8;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const u64 v)
	{
		//�˸��ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +8 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//����д��for������д��Ϊ��ֱ��ѧϰ
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->sendBuf[c->send_TempTail+2] = p[2];
			c->sendBuf[c->send_TempTail+3] = p[3];
			c->sendBuf[c->send_TempTail+4] = p[4];
			c->sendBuf[c->send_TempTail+5] = p[5];
			c->sendBuf[c->send_TempTail+6] = p[6];
			c->sendBuf[c->send_TempTail+7] = p[7];
			c->send_TempTail +=8;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const bool v)
	{
		//����һ���ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +1 <= func::__ServerInfo->SendMax)
		{
			
			c->sendBuf[c->send_TempTail] = v;//����д��for������д��Ϊ��ֱ��ѧϰ
			c->send_TempTail +=1;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const f32 v)
	{
		//�ĸ��ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +4 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//����д��for������д��Ϊ��ֱ��ѧϰ
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->sendBuf[c->send_TempTail+2] = p[2];
			c->sendBuf[c->send_TempTail+3] = p[3];
			c->send_TempTail +=4;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, const f64 v)
	{
		//�˸��ֽڳ�
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +8 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//����д��for������д��Ϊ��ֱ��ѧϰ
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->sendBuf[c->send_TempTail+2] = p[2];
			c->sendBuf[c->send_TempTail+3] = p[3];
			c->sendBuf[c->send_TempTail+4] = p[4];
			c->sendBuf[c->send_TempTail+5] = p[5];
			c->sendBuf[c->send_TempTail+6] = p[6];
			c->sendBuf[c->send_TempTail+7] = p[7];
			c->send_TempTail +=8;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	void TcpServer::sss(const int id, void* v, const u32 len)
	{
		//���ͽṹ��
		auto c = client(id);
		if(c== nullptr)return;
		if(c->is_Sending && c->send_TempTail +len < func::__ServerInfo->SendMax)
		{
			memcpy(&c->sendBuf[c->send_TempTail], v , len);//��������
			c->send_TempTail += len;
			return;
		}
		c->is_Sending = false;//ʧ�ܵĻ�
	}

	//***************************************
	//��֤�ͻ�����Ч��
	bool isValidClient(S_CLIENT_BASE* c, int len)
	{
		//�Ƿ�
		if(c->ID == -1 || c->state == func::S_Free || c->recv_TempTail ==0 || c->recvBuf == nullptr )//|| c->recv_TempHead + len > c->recv_TempTail)
		{
			return false;
		}
		return true;
	}
	
	void TcpServer::read(const int id, s8& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 1) == false)//��֤��ͨ��
		{
			v = 0;
			return;
		}
		//��֤ͨ��
		v = (*(s8*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead++;
		
	}

	void TcpServer::read(const int id, u8& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 1) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(u8*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead++;
	}

	void TcpServer::read(const int id, s16& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 2) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(s16*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=2;
	}

	void TcpServer::read(const int id, u16& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 2) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(u16*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=2;
	}

	void TcpServer::read(const int id, s32& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 4) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(s32*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=4;
	}

	void TcpServer::read(const int id, u32& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 4) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(u32*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=4;
	}

	void TcpServer::read(const int id, s64& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 8) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(s64*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=8;
	}

	void TcpServer::read(const int id, u64& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 8) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(u64*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=8;
	}

	void TcpServer::read(const int id, f32& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 4) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(f32*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=4;
	}

	void TcpServer::read(const int id, f64& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 8) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(f64*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=8;
	}

	void TcpServer::read(const int id, bool& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 1) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		//��֤ͨ��
		v = (*(bool*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=1;
	}

	void TcpServer::read(const int id, void* v, const u32 len)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, len) == false)//��֤��ͨ��
			{
			v = 0;
			return;
			}
		memcpy(v, &c->recvBuf[c->recv_TempHead], len);
		c->recv_TempHead+=len;
	}

	void TcpServer::setOnClientAccept(TCPSERVERNOTIFY_EVENT event)
	{
		onAcceptEvent = event;
	}

	void TcpServer::setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event)
	{
		onSecureEvent = event;
	}

	void TcpServer::setOnClientDisConnect(TCPSERVERNOTIFY_EVENT event)
	{
		onDisconnectEvent = event;
	}

	void TcpServer::setOnClientTimeout(TCPSERVERNOTIFY_EVENT event)
	{
		onTimeOutEvent = event;
	}

	void TcpServer::setOnClientExcept(TCPSERVERNOTIFY_EVENT event)
	{
		onExceptEvent = event;
	}

	//ÿ�����µ����ӵ�ʱ�򣬻�ȡһ�����е����ݽṹ
	S_CLIENT_BASE* TcpServer::getFreeLinker()
	{
		std::lock_guard<std::mutex> guard(this->m_findlink_mutex);
		for(int i = 0; i< Linkers->length; i++)
		{
			S_CLIENT_BASE* client = Linkers->Value(i);
			if(client->state == func::S_Free)//Ѱ�ҵ�һ������״̬�Ľṹ��
			{
				client->Reset();
				client->ID = i;
				client->state = func::S_Connect;
				return client;
			}
		}
		//û�ҵ���������
		return nullptr;
	}

	//�Զ���Ľṹ�壬����TCP������
	typedef struct tacp_keepalive
	{
		unsigned long onoff;
		unsigned long keepalivetime;
		unsigned long keepaliveinterval;
	}TCP_KEEPALIVE,*PTCP_KEEPALIVE;
	
	//���ڼ��ͻȻ���ߡ�ֻ������Windows2000����ƽ̨
	//�ͻ���Ҳ��Ҫwin2000����ƽ̨
	int TcpServer::setHeartCheck(SOCKET s)
	{
		DWORD dwError = 0L,dwBytes = 0;
		TCP_KEEPALIVE sKA_Settings = {0}, sReturned = {0};
		//sKA_Settings
		sKA_Settings.onoff = 1;
		sKA_Settings.keepalivetime = 5500;//��5.5s�ڱ��ִ�� ΢��
		sKA_Settings.keepaliveinterval = 1000;//���û�лظ����·���
		dwError = WSAIoctl(
			s,
			SIO_KEEPALIVE_VALS,
			&sKA_Settings,
			sizeof(sKA_Settings),
			&sReturned,
			sizeof(sReturned),
			&dwBytes,
			NULL,
			NULL
			);

		if(dwError == SOCKET_ERROR)
		{
			dwError = WSAGetLastError();
			LOG_MSG("SetHeartCheck->WSAIoctl()�������󣬴������: %d \n", dwError);
			return -1;
		}
		
		return 0;
	}
	
	
}

#endif
