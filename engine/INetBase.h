#ifndef __INETBASE_H
#define  __INETBASE_H

#include "IDefine.h"

namespace net
{
	class ITcpClient;
#pragma pack(push, packing)
	#pragma pack(1)

	//�ͻ���������������
	struct S_CLIENT_BASE_INDEX
	{
		s32	index;	//��ס����������ݵ��±�
		inline void Reset() { index = -1; }
	};

	//�ͻ�����������
	struct S_CLIENT_BASE
	{
	#ifdef ____WIN32_	//֧�ֿ�ƽ̨
		SOCKET socketfd;
	#else
		int	socketfd;
	#endif

		s8	state; //�з���һ�ֽڣ���ס��ǰ����״̬
		s8	closeState; // ��¼�ر�״̬
		char	ip[MAX_IP_LEN];
		u16	port;
		s32	ID;//��ǰ�����������ID��Ҳ�������±�
		u8	rCode;//�����ݽ�����򣬷�ֹ֪������������ʲô,�𵽻���������


	//��Ϸ��¼����
		s32	ClientID;//�������ܷ�����ID
		u8	ClientType;//0 1-DB 2-Center 3-Game 4-Gate 5-Login
	
		//������--��������
		//������--��������
		bool		is_RecvCompleted;//�����Ƿ�������
		char*		recvBuf;//��Ž��յ������ݣ���ȡ���Ŀͻ��˵����ݣ���Ž������ٵĿռ��С�ǳ�ʼ��ʱConfigXML��ReceMaxֵ
		//char*		recvBuf_Temp;//���һ�����������յ����ݵĴ�С����С��ConfigXML��ReceOne
		s32			recv_Head;//��������ͷ��ƫ���� ������
		s32			recv_Tail;//��������β��ƫ���� ������
		s32			recv_TempHead;//��ʱͷ
		s32			recv_TempTail;//��ʱβ

		//������--���
		//������--��������
		bool		is_Sending;//�Ƿ����ڷ������ݷ��
		bool		is_SendCompleted;//�Ƿ������
		char*		sendBuf;//���ͻ�����,��С��ConfigXML��SendMax
		s32			send_Head;//��������ͷ��ƫ���� ������ ��������
		s32			send_Tail;//��������β��ƫ���� ������
		s32			send_TempTail;//

		//ʱ����
		s32			time_Connect;//�������ʱ��
		s32			time_Heart;//��¼����ʱ��
		s32			time_Close;//�ر�ʱ��
		u8			threadID;//�߳�ID
		s32			shutdown_kind;//��ȫ�رշ���������ӡ�����Ϣ
		char		md5[MAX_MD5_LEN];//MD5�밲ȫ��֤

		void	Init();
		void	Reset();
	#ifdef ____WIN32_
		inline  bool isT(SOCKET _fd)
		{
			if (socketfd == _fd) return true;	
			return false;
		}
	#else // ____WIN32_
		inline bool isT(int fd)
		{
			if (socketfd == fd) return true;
			return false;
		}
	#endif
	};

	//���ӷ��������ݽṹ
	struct S_SERVER_BASE
	{
#ifdef ____WIN32_
		SOCKET	socketfd;
#else
		int			socketfd;
#endif
		s32			ID;
		char			ip[16];
		u16			port;
		s32			serverID;
		u8			serverType;

		u8			state;
		u8			rCode;

		//������--��������
	//������--��������
		char*		recvBuf;//��Ž��յ������ݣ���ȡ���Ŀͻ��˵����ݣ���Ž������ٵĿռ��С�ǳ�ʼ��ʱConfigXML��ReceMaxֵ
		char*		recvBuf_Temp;//���һ�����������յ����ݵĴ�С����С��ConfigXML��ReceOne
		s32			recv_Head;//��������ͷ��ƫ���� ������
		s32			recv_Tail;//��������β��ƫ���� ������
		s32			recv_TempHead;//��ʱͷ
		s32			recv_TempTail;//��ʱβ
		bool			is_Recved;//�����Ƿ�������

		//������--���
	//������--��������
		char*		sendBuf;//���ͻ�����,��С��ConfigXML��SendMax
		s32			send_Head;//��������ͷ��ƫ���� ������ ��������
		s32			send_Tail;//��������β��ƫ���� ������
		s32			send_TempTail;//
		bool			is_Sending;//�Ƿ����ڷ������ݷ��
		bool			is_SendCompleted;//�Ƿ������

		//ʱ����
		s32			time_AutoConnect;//�������ʱ��
		s32			time_Heart;//��¼����ʱ��
		char			md5[MAX_MD5_LEN];//MD5�밲ȫ��֤

		void	Init(int sid);
		void	reset();
	};


#pragma pack(pop, packing)

	class ITcpServer;
	class ITcpClient;
	typedef void(*TCPSERVERNOTIFY_EVENT) (ITcpServer* tcp, S_CLIENT_BASE *c , const s32 code);//������ʧȥ���ӣ��쳣���ӣ���ȫ���ӵ�ʱ�򣬽��¼��ɷ���ҵ���
	typedef void(*TCPSERVERNOTIFYERRO_EVENT) (ITcpServer* tcp, S_CLIENT_BASE* c, const s32 code, const char* err);
	typedef void(*TCPCLIENTNOTIFY_EVENT) (ITcpClient* tcp, const s32 code);

	class ITcpServer
	{
	public:
		virtual ~ITcpServer() {}
		virtual void runServer(int num) = 0;
		virtual void stopServer() =0;
		
	#ifdef ____WIN32_
		virtual	S_CLIENT_BASE * client(SOCKET fd, bool issecurity) = 0;//ͨ��socketID�Ȳ�������Ȼ��ȷ��λ��ǰS_CLIENT_BASE������,ͨ����������������
	#else
		virtual	S_CLIENT_BASE* client(int fd, bool issecurity) = 0;
	#endif
		virtual	S_CLIENT_BASE* client(int index) = 0; //�ڶ��ְ취��ֱ�Ӵ�����ֵ��ȷ��λ

		virtual bool isID_T(const s32 id) = 0;
		virtual bool isSecure_T(const s32 id, s32 secure) = 0;
		virtual bool isSecure_F_Close(const s32 id, s32 secure) = 0;//������ǰ�ȫ���Ӿ�ֱ�ӹص�
		virtual void parseCommand() = 0;
		virtual void getSecurityCount(int& connectnum,int& securitycount) = 0;
		
		virtual void begin(const int id, const u16 cmd) = 0;// ������������±� �� ͷָ��
		virtual void end(const int id) = 0;
		
		virtual void sss(const int id, const s8 v) = 0;
		virtual void sss(const int id, const u8 v) = 0;
		
		virtual void sss(const int id, const s16 v) = 0;
		virtual void sss(const int id, const u16 v) = 0;
		
		virtual void sss(const int id, const s32 v) = 0;
		virtual void sss(const int id, const u32 v) = 0;

		virtual void sss(const int id, const s64 v) = 0;
		virtual void sss(const int id, const u64 v) = 0;

		virtual void sss(const int id, const bool v) = 0;
		virtual void sss(const int id, const f32 v) = 0;
		virtual void sss(const int id, const f64 v) = 0;
		virtual void sss(const int id, void* v, const u32 len) = 0;

		virtual void read(const int id, s8& v) = 0;
		virtual void read(const int id, u8& v) = 0;

		virtual void read(const int id, s16& v) = 0;
		virtual void read(const int id, u16& v) = 0;

		virtual void read(const int id, s32& v) = 0;
		virtual void read(const int id, u32& v) = 0;

		virtual void read(const int id, s64& v) = 0;
		virtual void read(const int id, u64& v) = 0;

		virtual void read(const int id, f32& v) = 0;
		virtual void read(const int id, f64& v) = 0;

		virtual void read(const int id, bool& v) = 0;
		virtual void read(const int id, void* v, const u32 len) = 0;

		virtual void setOnClientAccept(TCPSERVERNOTIFY_EVENT event) =0;
		virtual void setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event) =0;//��ȫ����
		virtual void setOnClientDisConnect(TCPSERVERNOTIFY_EVENT event) =0;//ʧȥ����
		virtual void setOnClientTimeout(TCPSERVERNOTIFY_EVENT event) =0;//��ʱ����
		virtual void setOnClientExcept(TCPSERVERNOTIFY_EVENT event) =0;//�쳣
		virtual void registerCommand(int cmd, void* container) =0;//ע����Ϣ
		
		
	};

	class ITcpClient
	{
	public:
		virtual ~ITcpClient(){}
		virtual S_SERVER_BASE* getData() = 0;
#ifdef ____WIN32_
		virtual SOCKET getSocket() = 0;
#else
		virtual int getSocket() = 0;
#endif

		virtual void runClient(u32 sid, char* ip, int port) = 0;
		virtual bool connectServer() = 0;
		virtual void disconnectServer(const s32 errcode, const char* err) = 0;

		virtual void begin( const u16 cmd) = 0;// ������������±� �� ͷָ��
		virtual void end() = 0;

		virtual void sss( const s8 v) = 0;
		virtual void sss( const u8 v) = 0;

		virtual void sss( const s16 v) = 0;
		virtual void sss( const u16 v) = 0;

		virtual void sss(const s32 v) = 0;
		virtual void sss( const u32 v) = 0;

		virtual void sss(const s64 v) = 0;
		virtual void sss(const u64 v) = 0;

		virtual void sss( const bool v) = 0;
		virtual void sss( const f32 v) = 0;
		virtual void sss(const f64 v) = 0;
		virtual void sss( void* v, const u32 len) = 0;

		virtual void read( s8& v) = 0;
		virtual void read(u8& v) = 0;

		virtual void read(s16& v) = 0;
		virtual void read( u16& v) = 0;

		virtual void read( s32& v) = 0;
		virtual void read(u32& v) = 0;

		virtual void read( s64& v) = 0;
		virtual void read( u64& v) = 0;

		virtual void read( f32& v) = 0;
		virtual void read( f64& v) = 0;

		virtual void read(bool& v) = 0;
		virtual void read(void* v, const u32 len) = 0;

		virtual void parseCommand() = 0;
		virtual void registerCommand(int cmd, void* container) = 0;//ע����Ϣ
		virtual void setOnConnect(TCPCLIENTNOTIFY_EVENT event) = 0;
		virtual void setOnSecure(TCPCLIENTNOTIFY_EVENT event) = 0;//��ȫ����
		virtual void setOnDisConnect(TCPCLIENTNOTIFY_EVENT event) = 0;//ʧȥ����
		virtual void setOnExcept(TCPCLIENTNOTIFY_EVENT event) = 0;//�쳣
	};

	extern ITcpServer* NewTcpServer();//����һ��ITcpServer����
	extern ITcpClient* NewTcpClient();//����һ��ITcpClient����
}

#endif
