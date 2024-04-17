#include "TcpServer.h"

#ifdef  ____WIN32_

#include "IOPool.h"

std::vector<IContainer*> __Commands;

namespace net
{

	void TcpServer::initCommands()
	{
		__Commands.reserve(MAX_COMMAND_LEN);//开辟内存空间 保留向量对象的最小存储长度。
		for(int i=0; i< MAX_COMMAND_LEN; i++)//在64位系统下 指针占八个字节
		{
			__Commands.emplace_back(nullptr);//将一个就地构造的元素添加到向量末尾。
		}
	}

	//注册消息
	void TcpServer::registerCommand(int cmd, void* container)
	{
		if(cmd >= MAX_COMMAND_LEN ) return;
		IContainer* icon = (IContainer*)container;
		if(icon == nullptr) return;
		__Commands[cmd] = icon;
		
	}

	//**********************************************
	//最外层 在业务层更新的 每帧更新的
	void TcpServer::parseCommand()//解析指令
	{
		//更新所有的连接玩家
		for(int i =0; i<Linkers->length; i++)
		{
			//如果不是一个正常的连接状态 就不会继续解析它的数据了
			auto c = Linkers->Value(i);
			if(c->ID == -1) continue;//没有玩家
			if(c->state == func::S_Free)continue;
			if(c->state == func::S_NeedSave) continue;

			checkConnect(c);
			if(c->closeState == func::S_CLOSE_SHUTDOWN) continue;

			parseCommand(c);
			//调用发送数据
			this->postSend(c);
		}
	}

	void TcpServer::parseCommand(S_CLIENT_BASE* c)
	{
		if(c->is_RecvCompleted == false) return;

		while(c->recv_Tail - c->recv_Head > 7)//连八个字节都没有 证明不是一个完整的包
		{
			char head[2];//两个字节的消息头
			head[0] = c->recvBuf[c->recv_Head] ^ c->rCode;
			head[1] = c->recvBuf[c->recv_Head +1] ^ c->rCode;
			if(head[0] != func::__ServerInfo->Head[0] || head[1] != func::__ServerInfo->Head[1])
			{
				shutDown(c->socketfd, 0, c, 2001);
				return;
			}
			//一个数据异或同一个数第二次等于没异或
			u32 length	=   (*(u32*) (c->recvBuf + c->recv_Head + 2 )) ^ c->rCode;//通过u32指针再解除引用还原成整形值 两个字节的消息头 四个字节的消息长度
			u16 cmd		=	(*(u16*) (c->recvBuf + c->recv_Head + 6 )) ^ c->rCode;//两个字节的头指令
			if(c->recv_Tail < c->recv_Head + length) break;//说明至少头指令是残缺的,消息还不完整，需要退出去直到数据到来

			//消息头消息长度头指令解析完了 接下来解析消息体
			c->recv_TempHead = c->recv_Head + 8;
			c->recv_TempTail = c->recv_Head + length;//这条消息最长有多长

			//解析消息体
			parseCommand(c,cmd);
			if(c->state < func::S_Connect)
			{
				return;
			}

			//解析成功
			c->recv_Head += length;//c->recv_Head追赶到c->recv_TempTail
			
		}
		c->is_RecvCompleted = false;
	}

	int times = 1;
	//业务逻辑层
	void TcpServer::parseCommand(S_CLIENT_BASE* c, u16 cmd)
	{
		c->time_Heart = (int)time(NULL);//收到任何消息 都来更新心跳包的时间

		if(cmd <65000)//说明是一个业务线程的头指令
		{
			
			if(cmd == CMD_HEART)
			{
				c->time_Heart = (int)time(NULL);
				u32 value = 0;
				read(c->ID, value);

				//判断是否是客户端和网关之间进行的心跳包
				if(value == 1)//仅玩家客户端发送的心跳包值是1
				{
					LOG_MSG("收到心跳包 %d 发给:%d 这是第%d次\n",value,c->ID,times);
					times++;
					begin(c->ID, CMD_HEART);//服务端返回心跳包
					sss(c->ID, 1);
					end(c->ID);
				}
				
				//通过注册消息的方式 派发到业务逻辑层
				return;
			}

			auto container =__Commands[cmd];
			if(container == nullptr)
			{
				LOG_MSG("command not register... %d \n", cmd);
				return;
			}

			//调用子类的方法 将消息体转接到外部业务层
			container->onServerCommand(this, c, cmd);
			return;
		}

		switch(cmd)
		{
		case CMD_SECURITY://安全连接 符合我们自定义消息体的协议规范
			char a[20];
			sprintf_s(a, "%s_%d", func::__ServerInfo->SafeCode, c->rCode);//安全码和加密码组合生成一个字符串数组
			memset(c->md5, 0, sizeof(c->md5));//先格式化
			if(func::MD5str != NULL) func::MD5str(c->md5, (unsigned char*)a, strlen(a));

			char str5[MAX_MD5_LEN];
			memset(str5, 0, MAX_MD5_LEN);

			//这两个参数将在分布式和负载均衡里有用,服务器和服务器之间通信会有用
			u32 version = 0;
			u32 c_id = 0;
			u8 c_type = 0;
			
			read(c->ID, c_id);//解析
			read(c->ID, c_type);//
			read(c->ID, version);//
			read(c->ID, str5, MAX_MD5_LEN);//

			if(version != func::__ServerInfo->Version)//版本号不相等的话
			{
				//这是在主线程解析的，所以可以封包
				//给客户端发送一条数据回去
				begin(c->ID, CMD_SECURITY);
				sss(c->ID, (u16)1 );//返回客户端1代表版本不一致
				end(c->ID);
				return;
			}

			//经过MD5码验证就可以认为是一个活跃的连接了
			int error = stricmp(c->md5, str5);//字符串匹配，匹配上了返回的是0
			if(error != 0)
			{
				begin(c->ID, CMD_SECURITY);
				sss(c->ID, (u16)2 );//返回2代表MD5安全码验证没通过
				end(c->ID);
				return;
			}

			//证明是一个安全的连接,后面不会再进入检查连接的判断
			c->state = func::S_ConnectSecure;
			begin(c->ID, CMD_SECURITY);
			sss(c->ID, (u16)0 );//返回0代表安全连接成功
			end(c->ID);

			c->ClientID = c_id;
			c->ClientType = c_type;

			this->updateSecurityConnect(true);//更新当前安全连接数量

			if(onSecureEvent != nullptr) this->onSecureEvent(this, c,0);//派发一个安全连接事件给业务层
			break;
		}
		
	}

	void TcpServer::checkConnect(S_CLIENT_BASE* c)
	{
		s32 temp = 0;
		if(c->closeState == func::S_CLOSE_SHUTDOWN)//已经关闭的话
		{
			temp = (int)time(NULL) - c->time_Close;
			if(c->is_RecvCompleted && c->is_SendCompleted)
			{
				closeSocket(c->socketfd, c, 2001);//已经关闭连接的话就释放socket
			}
			else if(temp > 10000)//如果还有数据没有接收或者发送完毕的话 超时
			{
				closeSocket(c->socketfd, c, 2005);//已经关闭连接的话就释放socket
			}
			return;
		}
		
		//检查安全连接 
		temp = (int)time(NULL) - c->time_Connect;
		if(c->state == func::S_Connect )
		{
			if(temp > 100)//如果超过十秒还处于连接状态的话
			{
				if(this->onTimeOutEvent != nullptr ) this->onTimeOutEvent(this, c, 2102);//派发超时事件出去
				shutDown(c->socketfd, 0, c, 2102);
				return;
			}
		}

		//检查心跳包
		temp = (int)time(NULL) - c->time_Heart;
		if(temp > func::__ServerInfo->HeartTime)
		{
			if(this->onTimeOutEvent != nullptr ) this->onTimeOutEvent(this, c, 2003);//派发超时事件出去
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
			//检查socketfd和c里存的socketid是不是一个值
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
		if(id <0 || id >= Linkers->length)return false;//检查ID下标的合法性
		return true;
	}

	bool TcpServer::isSecure_T(const s32 id, s32 secure)
	{
		if(id <0 || id >= Linkers->length)return false;//检查ID下标的合法性
		auto c = Linkers->Value(id);
		if(c->state < secure) return false;//小于安全状态

		return true;
	}

	bool TcpServer::isSecure_F_Close(const s32 id, s32 secure)
	{
		if(id <0 || id >= Linkers->length)return false;//检查ID下标的合法性
		auto c = Linkers->Value(id);
		if(c->state >= secure) return false;//大于安全状态

		//不然就要关闭
		shutDown(c->socketfd, 0, c, 2006);
		return true;
	}



	void TcpServer::getSecurityCount(int& connectnum, int& securitycount)
	{
		connectnum		= m_ConnectCount;
		securitycount	= m_SecurityCount;
		
	}

	//生产发送数据
	void TcpServer::begin(const int id, const u16 cmd)
	{
		auto c = client(id);
		if(c== nullptr)return;
		if(c->send_Head == c->send_Tail)
		{
			//此时没有可以发送的数据了，清零
			c->send_Head = 0;
			c->send_Tail = 0;
		}
		c->send_TempTail = c->send_Tail;
		//满足这四个条件就可以封包了
		if(c->state > func::S_Free && c->is_Sending == false && c->socketfd != INVALID_SOCKET && c->send_TempTail + 8 <= func::__ServerInfo->SendMax )
		{
			c->is_Sending = true;
			c->sendBuf[c->send_Tail + 0] = func::__ServerInfo->Head[0] ^ c->rCode;//异或封装两个字节的消息头
			c->sendBuf[c->send_Tail + 1] = func::__ServerInfo->Head[1] ^ c->rCode;//异或封装两个字节的消息头
			//if (cmd != 65001 && cmd != 65002) printf("\n");
			u16 newcmd = cmd ^ c->rCode;
			char* a = (char*)&newcmd;

			c->sendBuf[c->send_Tail + 6] = a[0];//异或封装消息尾
			c->sendBuf[c->send_Tail + 7] = a[1];//异或封装消息尾

			c->send_TempTail += 8;//偏移八个字节
			return;
		}
		//发生错误的话
		shutDown(c->socketfd, 0, c, 2004);
		
	}

	void TcpServer::end(const int id)
	{
		auto c = client(id);
		if(c== nullptr)return;

		//出问题
		if(c->is_Sending == false || c->send_Tail +8 > func::__ServerInfo->SendMax || c->send_TempTail +8 >func::__ServerInfo->SendMax || c->send_Tail >= c->send_TempTail)
		{
			shutDown(c->socketfd, 0, c, 2005);
			return;
		}

		c->is_Sending = false;//封包成功
		//计算消息长度
		u32 len = (c->send_TempTail - c->send_Tail) ^ c->rCode;//异或防抓包
		char* a = (char*)&len;
		//保存消息长度
		c->sendBuf[c->send_Tail + 2] = a[0];//异或封装消息尾
		c->sendBuf[c->send_Tail + 3] = a[1];//异或封装消息尾
		c->sendBuf[c->send_Tail + 4] = a[2];//异或封装消息尾
		c->sendBuf[c->send_Tail + 5] = a[3];//异或封装消息尾

		c->send_Tail = c->send_TempTail;//最后让尾部偏移消息体的长度指向最后的位置
	}

	void TcpServer::sss(const int id, const s8 v)
	{
		//一个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +1 <= func::__ServerInfo->SendMax)
		{
			c->sendBuf[c->send_TempTail] = v ; // v ^ c->rCode
			c->send_TempTail ++;
			return;
		}
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const u8 v)
	{
		//一个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +1 <= func::__ServerInfo->SendMax)
		{
			c->sendBuf[c->send_TempTail] = v;
			c->send_TempTail ++;
			return;
		}
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const s16 v)
	{
		//两个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +2 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//可以写成for，这样写是为了直观学习
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->send_TempTail +=2;
			return;
		}
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const u16 v)
	{
		//两个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +2 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//可以写成for，这样写是为了直观学习
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->send_TempTail +=2;
			return;
		}
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const s32 v)
	{
		//四个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +4 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//可以写成for，这样写是为了直观学习
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->sendBuf[c->send_TempTail+2] = p[2];
			c->sendBuf[c->send_TempTail+3] = p[3];
			c->send_TempTail +=4;
			return;
		}
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const u32 v)
	{
		//四个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +4 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//可以写成for，这样写是为了直观学习
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->sendBuf[c->send_TempTail+2] = p[2];
			c->sendBuf[c->send_TempTail+3] = p[3];
			c->send_TempTail +=4;
			return;
		}
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const s64 v)
	{
		//八个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +8 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//可以写成for，这样写是为了直观学习
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
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const u64 v)
	{
		//八个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +8 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//可以写成for，这样写是为了直观学习
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
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const bool v)
	{
		//布尔一个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +1 <= func::__ServerInfo->SendMax)
		{
			
			c->sendBuf[c->send_TempTail] = v;//可以写成for，这样写是为了直观学习
			c->send_TempTail +=1;
			return;
		}
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const f32 v)
	{
		//四个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +4 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//可以写成for，这样写是为了直观学习
			c->sendBuf[c->send_TempTail+1] = p[1];
			c->sendBuf[c->send_TempTail+2] = p[2];
			c->sendBuf[c->send_TempTail+3] = p[3];
			c->send_TempTail +=4;
			return;
		}
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, const f64 v)
	{
		//八个字节长
		auto c = client(id);
		if(c== nullptr)return;

		if(c->is_Sending && c->send_TempTail +8 <= func::__ServerInfo->SendMax)
		{
			char* p = (char*)& v;
			c->sendBuf[c->send_TempTail+0] = p[0];//可以写成for，这样写是为了直观学习
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
		c->is_Sending = false;//失败的话
	}

	void TcpServer::sss(const int id, void* v, const u32 len)
	{
		//发送结构体
		auto c = client(id);
		if(c== nullptr)return;
		if(c->is_Sending && c->send_TempTail +len < func::__ServerInfo->SendMax)
		{
			memcpy(&c->sendBuf[c->send_TempTail], v , len);//拷贝数据
			c->send_TempTail += len;
			return;
		}
		c->is_Sending = false;//失败的话
	}

	//***************************************
	//验证客户端有效性
	bool isValidClient(S_CLIENT_BASE* c, int len)
	{
		//非法
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
		if(isValidClient(c, 1) == false)//验证不通过
		{
			v = 0;
			return;
		}
		//验证通过
		v = (*(s8*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead++;
		
	}

	void TcpServer::read(const int id, u8& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 1) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(u8*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead++;
	}

	void TcpServer::read(const int id, s16& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 2) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(s16*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=2;
	}

	void TcpServer::read(const int id, u16& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 2) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(u16*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=2;
	}

	void TcpServer::read(const int id, s32& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 4) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(s32*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=4;
	}

	void TcpServer::read(const int id, u32& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 4) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(u32*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=4;
	}

	void TcpServer::read(const int id, s64& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 8) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(s64*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=8;
	}

	void TcpServer::read(const int id, u64& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 8) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(u64*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=8;
	}

	void TcpServer::read(const int id, f32& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 4) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(f32*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=4;
	}

	void TcpServer::read(const int id, f64& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 8) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(f64*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=8;
	}

	void TcpServer::read(const int id, bool& v)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, 1) == false)//验证不通过
			{
			v = 0;
			return;
			}
		//验证通过
		v = (*(bool*)(c->recvBuf + c->recv_TempHead));
		c->recv_TempHead+=1;
	}

	void TcpServer::read(const int id, void* v, const u32 len)
	{
		auto c= client(id);
		if(c == nullptr) return;
		if(isValidClient(c, len) == false)//验证不通过
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

	//每次有新的连接的时候，获取一个空闲的数据结构
	S_CLIENT_BASE* TcpServer::getFreeLinker()
	{
		std::lock_guard<std::mutex> guard(this->m_findlink_mutex);
		for(int i = 0; i< Linkers->length; i++)
		{
			S_CLIENT_BASE* client = Linkers->Value(i);
			if(client->state == func::S_Free)//寻找到一个空闲状态的结构体
			{
				client->Reset();
				client->ID = i;
				client->state = func::S_Connect;
				return client;
			}
		}
		//没找到空闲索引
		return nullptr;
	}

	//自定义的结构体，用于TCP服务器
	typedef struct tacp_keepalive
	{
		unsigned long onoff;
		unsigned long keepalivetime;
		unsigned long keepaliveinterval;
	}TCP_KEEPALIVE,*PTCP_KEEPALIVE;
	
	//用于检测突然断线、只适用于Windows2000以上平台
	//客户端也需要win2000以上平台
	int TcpServer::setHeartCheck(SOCKET s)
	{
		DWORD dwError = 0L,dwBytes = 0;
		TCP_KEEPALIVE sKA_Settings = {0}, sReturned = {0};
		//sKA_Settings
		sKA_Settings.onoff = 1;
		sKA_Settings.keepalivetime = 5500;//在5.5s内保持存活 微秒
		sKA_Settings.keepaliveinterval = 1000;//如果没有回复重新发送
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
			LOG_MSG("SetHeartCheck->WSAIoctl()发生错误，错误代码: %d \n", dwError);
			return -1;
		}
		
		return 0;
	}
	
	
}

#endif
