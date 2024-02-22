#ifndef __TCPSERVER_H
#define __TCPSERVER_H

#ifdef ____WIN32_

#include "INetBase.h"
#include "IContainer.h"
#include <mutex>
#include <thread>
#include <MSWSock.h>
#pragma comment(lib, "mswsock")


namespace net
{
	class TcpServer : public ITcpServer
	{
	public:
		TcpServer();
		virtual ~TcpServer();

	private:
		//�����������
		s32	m_ConnectCount;//��ǰ������
		s32	m_SecurityCount;//��ȫ������
		bool	m_IsRunning;		//�������Ƿ���������
		s32	m_ThreadNum;	//�߳�����
		SOCKET	listenfd;		//�����׽��־��
		//����IOCP����
		HANDLE	m_CompletePort;	//��ɶ˿ھ��
		LPFN_ACCEPTEX	m_AcceptEx;//AcceptEx������ַ
		LPFN_GETACCEPTEXSOCKADDRS	m_GetAcceptEx;//��ȡ�ͻ�����Ϣ������ַ
		//���幤���߳�
		std::shared_ptr<std::thread> m_workthread[10];//����ָ�빤���߳�
		//���廥��
		std::mutex	m_findlink_mutex;
		std::mutex	m_ConnectMutex;
		std::mutex	m_SecurityMutex;
		//�����������
		HashArray<S_CLIENT_BASE>* Linkers;//���ӵ����
		HashArray<S_CLIENT_BASE_INDEX>* LinkerIndexs;//���������������
		//����ָ���й�
		TCPSERVERNOTIFY_EVENT	onAcceptEvent;
		TCPSERVERNOTIFY_EVENT	onSecureEvent;
		TCPSERVERNOTIFY_EVENT	onTimeOutEvent;
		TCPSERVERNOTIFY_EVENT	onDisconnectEvent;
		TCPSERVERNOTIFY_EVENT	onExceptEvent;

	private:

		s32 postAccept();
		s32 onAccept(void* context);

		s32 postRecv(SOCKET s);
		s32 onRecv(void * context, s32 recvBytes, u32 tid);//tid��ʾ���ĸ��߳̽�����
		s32 onRecv_SavaData(S_CLIENT_BASE* c, char* buf, s32 recvBytes);

		s32 postSend(S_CLIENT_BASE* c);
		s32 onSend(void *context, s32 sendBytes);
		
		s32	closeSocket(SOCKET sfd, S_CLIENT_BASE* c, int kind);//�ر��ͷ�socket
		void	shutDown(SOCKET sfd, s32 mode, S_CLIENT_BASE* c, int kind); //�ر�socket��д�˵ķ��������������ݣ���������
		int setHeartCheck(SOCKET s);
		S_CLIENT_BASE* getFreeLinker();
		
		//����һЩ˽��api
		inline HANDLE getCompletePort() { return m_CompletePort; }
		inline void	updateSecurityConnect(	bool isadd)
		{
			//��������
			std::lock_guard<std::mutex>	guard(this->m_SecurityMutex);
			if (isadd) m_SecurityCount++;
			else m_SecurityCount--;
		}

		inline void	updateConnect(bool isadd)
		{
			//��������
			std::lock_guard<std::mutex>	guard(this->m_ConnectMutex);
			if (isadd) m_ConnectCount++;
			else m_ConnectCount--;
		}

		inline S_CLIENT_BASE_INDEX* getClientIndex(int sfd)
		{
			if (sfd < 0 || sfd > MAX_USER_SOCKETFD) return nullptr;
			auto c = LinkerIndexs->Value(sfd);
			return c;
		}

	private:
		s32 initSocket();
		void initPost();
		//��ʼ����Ϣ
		void initCommands();
		
		void runThread(int num);
		void parseCommand(S_CLIENT_BASE* c);
		void parseCommand(S_CLIENT_BASE* c, u16 cmd);
		void checkConnect(S_CLIENT_BASE* c);
		
		static void run(TcpServer* tcp, int id);//�Զ�����߳�ID

	public:
		virtual	void runServer(int num);
		virtual void stopServer();
		virtual	S_CLIENT_BASE* client(SOCKET socketfd, bool issecurity);//ͨ��socketID�Ȳ�������Ȼ��ȷ��λ��ǰS_CLIENT_BASE������,ͨ����������������
		virtual	S_CLIENT_BASE* client(const int id); //�ڶ��ְ취��ֱ�Ӵ�����ֵ��ȷ��λ
		virtual bool isID_T(const s32 id);
		virtual bool isSecure_T(const s32 id, s32 secure);
		virtual bool isSecure_F_Close(const s32 id, s32 secure);//������ǰ�ȫ���Ӿ�ֱ�ӹص�
		virtual void parseCommand();
		virtual void getSecurityCount(int& connectnum,int& securitycount);
		
		virtual void begin(const int id, const u16 cmd);// ������������±� �� ͷָ��
		virtual void end(const int id);

		//���õĻ���Ϣ�岿��Ҳ��Ҫ�����
		virtual void sss(const int id, const s8 v) ;
		virtual void sss(const int id, const u8 v);
		
		virtual void sss(const int id, const s16 v);
		virtual void sss(const int id, const u16 v) ;
		
		virtual void sss(const int id, const s32 v) ;
		virtual void sss(const int id, const u32 v) ;

		virtual void sss(const int id, const s64 v) ;
		virtual void sss(const int id, const u64 v) ;

		virtual void sss(const int id, const bool v) ;
		virtual void sss(const int id, const f32 v) ;
		virtual void sss(const int id, const f64 v) ;
		virtual void sss(const int id, void* v, const u32 len) ;

		virtual void read(const int id, s8& v);
		virtual void read(const int id, u8& v);

		virtual void read(const int id, s16& v);
		virtual void read(const int id, u16& v);

		virtual void read(const int id, s32& v);
		virtual void read(const int id, u32& v);

		virtual void read(const int id, s64& v);
		virtual void read(const int id, u64& v);

		virtual void read(const int id, f32& v);
		virtual void read(const int id, f64& v);

		virtual void read(const int id, bool& v);
		virtual void read(const int id, void* v, const u32 len);

		virtual void setOnClientAccept(TCPSERVERNOTIFY_EVENT event);
		virtual void setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event);//��ȫ����
		virtual void setOnClientDisConnect(TCPSERVERNOTIFY_EVENT event);//ʧȥ����
		virtual void setOnClientTimeout(TCPSERVERNOTIFY_EVENT event);//��ʱ����
		virtual void setOnClientExcept(TCPSERVERNOTIFY_EVENT event);//�쳣
		virtual void registerCommand(int cmd, void* container);//ע����Ϣ
		
	};



}

#endif


#endif
