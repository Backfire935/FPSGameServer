#ifndef ____EPOLLSERVER_H
#define ____EPOLLSERVER_H

#ifndef ____WIN32_

#include "INetBase.h"
#include "IContainer.h"
#include<condition_variable>
#include<thread>
#include<list>
#include<netinet/in.h>


namespace net
{
	class EpollServer : public ITcpServer
	{
	public:
		EpollServer();
		virtual ~EpollServer() {};

	private:
		u32	m_ConnectNum;//������
		u32	m_SecurityCount;//��ȫ������
		u32	m_ThreadNum;
		u32	m_LinkNum;
		bool	m_IsRunning;

		//��������
		std::condition_variable m_AcceptCond;
		std::condition_variable m_RecvCond;

		//������
		std::mutex m_ConnectMutex;
		std::mutex m_SecurityMutex;
		std::mutex m_AcceptMutex;
		std::mutex m_RecvMutex;
		std::mutex m_FindLinkMutex;

		//��Ҫ�������߳�
		std::shared_ptr<std::thread> m_ManagerThread;//�������߳�
		std::shared_ptr<std::thread> m_AcceptThread;//�������߳�
		std::shared_ptr<std::thread> m_RecvThread;//���������߳�
		//��Ϸ����������������ķ��������ӽ��٣����Է������ݵĲ��ֿ���д�����̻߳�ҵ���߳������������ǵ�����һ�����������߳�

		std::list<int>	m_Socketfds;//���µ��������ӵ�ʱ�򣬾Ͱ�socketfd�Ž������������
		int					listenfd;
		int					epollfd;
		char* recvBuf[10];//��ʱ��ת�������洢�ӽ������ݵ��߳��л�õ�����
		HashArray<S_CLIENT_BASE>* Linkers;
		HashArray<S_CLIENT_BASE_INDEX>* LinkersIndex;

		TCPSERVERNOTIFY_EVENT onAcceptEvent;
		TCPSERVERNOTIFY_EVENT onSecurityEvent;
		TCPSERVERNOTIFY_EVENT onTimeoutEvent;
		TCPSERVERNOTIFY_EVENT onDisconnectEvent;
		TCPSERVERNOTIFY_EVENT onExceptEvent;

	private:
		int initSocket();
		int add_event(int epollfd, int socketfd, int events);
		int delete_event(int epollfd, int socketfd, int events);


	public:
		inline  void	updateSecurityCount(bool isadd)
		{
			std::lock_guard<std::mutex> guard(m_SecurityMutex);
			if (isadd)m_SecurityCount++;
			else m_SecurityCount--;
		}

		inline  void	updateConnectCount(bool isadd)
		{
			std::lock_guard<std::mutex> guard(m_ConnectMutex);
			if (isadd)m_ConnectNum++;
			else m_ConnectNum--;
		}

		inline S_CLIENT_BASE_INDEX* GetClientIndex(const int socketfd)
		{
			if (socketfd < 0 || socketfd >= MAX_USER_SOCKETFD)return NULL;
			S_CLIENT_BASE_INDEX* c = LinkersIndex->Value(socketfd);
			return c;
		}

	public:
		virtual void runServer(int num) ;//�����Ǵ�����߳�����
		virtual void stopServer() ;

		virtual	S_CLIENT_BASE* client(int fd, bool issecurity) ;//ͨ��socketID�Ȳ�������Ȼ��ȷ��λ��ǰS_CLIENT_BASE������,ͨ����������������
		virtual	S_CLIENT_BASE* client(int index) ; //�ڶ��ְ취��ֱ�Ӵ�����ֵ��ȷ��λ

		virtual bool isID_T(const s32 id) ;
		virtual bool isSecure_T(const s32 id, s32 secure) ;
		virtual bool isSecure_F_Close(const s32 id, s32 secure) ;//������ǰ�ȫ���Ӿ�ֱ�ӹص�
		virtual void parseCommand() ;
		virtual void getSecurityCount(int& connectnum, int& securitycount) ;

		virtual void begin(const int id, const u16 cmd) ;// ������������±� �� ͷָ��
		virtual void end(const int id) ;

		virtual void sss(const int id, const s8 v) ;
		virtual void sss(const int id, const u8 v) ;

		virtual void sss(const int id, const s16 v) ;
		virtual void sss(const int id, const u16 v) ;

		virtual void sss(const int id, const s32 v) ;
		virtual void sss(const int id, const u32 v) ;

		virtual void sss(const int id, const s64 v) ;
		virtual void sss(const int id, const u64 v) ;

		virtual void sss(const int id, const bool v) ;
		virtual void sss(const int id, const f32 v) ;
		virtual void sss(const int id, const f64 v) ;
		virtual void sss(const int id, void* v, const u32 len) ;

		virtual void read(const int id, s8& v) ;
		virtual void read(const int id, u8& v) ;

		virtual void read(const int id, s16& v) ;
		virtual void read(const int id, u16& v) ;

		virtual void read(const int id, s32& v) ;
		virtual void read(const int id, u32& v) ;

		virtual void read(const int id, s64& v) ;
		virtual void read(const int id, u64& v) ;

		virtual void read(const int id, f32& v) ;
		virtual void read(const int id, f64& v) ;

		virtual void read(const int id, bool& v) ;
		virtual void read(const int id, void* v, const u32 len) ;

		virtual void setOnClientAccept(TCPSERVERNOTIFY_EVENT event) ;
		virtual void setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event) ;//��ȫ����
		virtual void setOnClientDisConnect(TCPSERVERNOTIFY_EVENT event) ;//ʧȥ����
		virtual void setOnClientTimeout(TCPSERVERNOTIFY_EVENT event) ;//��ʱ����
		virtual void setOnClientExcept(TCPSERVERNOTIFY_EVENT event) ;//�쳣
		virtual void registerCommand(int cmd, void* container) ;//ע����Ϣ


	};

	
}

#endif


#endif

