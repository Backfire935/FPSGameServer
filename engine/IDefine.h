#ifndef __IDEFINE_H
#define  __IDEFINE_H

#include <vector>

#ifdef  ____WIN32_
#include<WinSock2.h>
#pragma  comment (lib,"ws2_32")
#endif


#define MAX_USER_SOCKETFD	100000 //�����������
#define MAX_EXE_LEN			200 //exe·����󳤶�
#define MAX_FILENAME_LEN	250 //�ļ�����󳤶�
#define SIO_KEEPALIVE_VALS IOC_IN | IOC_VENDOR | 4 //Windows���������ĺ궨��
#define MAX_MD5_LEN	35 //����˺Ϳͻ��˽���ͨѶʱ����MD5����֤����󳤶�
#define MAX_IP_LEN	20 //IP�ĳ���
#define MAX_COMMAND_LEN 65535//���ָ���	

#define LOG_MSG printf //��־��ӡ�ĺ�

#define CMD_HEART		60000 //������
#define CMD_RCODE		65001 //������
#define CMD_SECURITY	65002 //��ȫ��֤

#ifdef ____WIN32_
//����һЩ�ͷ�ָ����handle socket�ĺ�
#define RELEASE_POINTER(p) { if(p != NULL) {delete p; p = NULL;} }
#define RELEASE_HANDLE(h) { if(h != NULL && h != INVALID_HANDLE_VALUE) {CloseHandle(h);} h = INVALID_HANDLE_VALUE; }
#define RELEASE_SOCKET(s) {if( s != INVALID_SOCKET) { closesocket(s); s = INVALID_SOCKET; }}
#endif

typedef signed char         s8;
typedef signed short		s16;
typedef signed int			s32;
typedef signed long long	s64;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long	u64;
typedef float				f32;
typedef double				f64;

namespace  func
{
	enum SOCKET_CLOSE
	{
		S_CLOSE_FREE = 0,
		S_CLOSE_ACCEPT = 1,//���ӳ���ر�
		S_CLOSE_SHUTDOWN = 2 ,//�ر�����
		S_CLOSE_CLOSE = 3, //��ʽ�ر�
	};

	//IOCPͶ����Ϣ״̬
	enum SOCKET_CONTEXT_STATE
	{
		SC_WAIT_ACCEPT = 0,
		SC_WAIT_RECV = 1,
		SC_WAIT_SEND = 2,
		SC_WAIT_RESET = 3,
	};

	//�����������socket״̬ �Ƿ���Ҫ���棬�Ƿ�ȫ��������....
	enum S_SOCKET_STATE
	{
		S_Free = 0,
		S_Connect = 1,
		S_ConnectSecure = 2,
		S_Login = 3,
		S_NeedSave = 4,
		S_Saving = 5,
	};

	enum C_SOCKET_STATE
	{
		C_Free = 0,
		C_ConnectTry = 1,
		C_Connect = 2,
		C_ConnectSecure = 3,
		C_Login = 4,
	};

	enum S_SERVER_TYPE
	{
		S_TYPE_USER = 0x00,
		S_TYPE_DB ,
		S_TYPE_CENTER ,
		S_TYPE_GAME ,
		S_TYPE_GATE ,
		S_TYPE_LOGIN ,
	};

	//������������� IP �˿� �汾����֤ ɶ��
	struct ConfigXML
	{
		s32	ID; //��ǰӦ�ó���������ID���Ƕ���
		u8	Type;//��ǰ���еķ������Ǹ�ʲô���͵ķ�����,ds�����������ķ���������ͼ������
		u16	Port; //�˿ں�
		s32	MaxPlayer; //������
		s32	MaxConnect; //��ǰ����������������������
		u8	RCode; //����ͻ��˷�������Ϣ���ݣ���һЩ���ܵ�����ɶ��
		s32	Version; //�汾�ţ����������ӵĹ�������֤
		s32	ReceOne;	//����˽������ݵ�ʱ��һ�������ն�������
		s32	ReceMax; //�ͻ��������������ٻ���ռ�Ĵ�С
		s32	SendOne; //һ����෢�Ͷ����ֽڵ�����
		s32	SendMax; //����ͻ���������һ�η������ʱ�򣬾Ͳ���������һ���ٷ�
		s32	HeartTime; //ͨ���������жϷ���˺Ϳͻ��������Ƿ���Ч���Ƿ�Ҫ�ͷ�
		s32	AutoTime; //�Զ�����ʱ��
		s32	MaxAccept; //IOCP���� 
		s32	MaxRece; //IO������ճ�
		s32	MaxSend;
		char	SafeCode[20];//��ȫ�� ���20�ֽ�,����������ʱ�ж��Ƿ��ǺϷ�����
		char	Head[3];//��֤��Ϣͷ
		char	IP[MAX_IP_LEN];
	};

	//���ӷ������
	struct ServerListXML
	{
		s32	ID; //��Ҫ���ӵ�ǰ����˵�ID
		u16	Port; //���ӷ������Ķ˿ں�
		char	IP[MAX_IP_LEN]; //���ӷ�������IP��ַ
	};

	//����ȫ�ֱ���
	extern char FileExePath[MAX_EXE_LEN];//����ļ�·��
	extern ConfigXML* __ServerInfo; //�����XMLָ�����
	extern ConfigXML* __ClientInfo; //�ͻ���XMLָ�����
	extern std::vector<ServerListXML*> __ServerListInfo; //��Ҫ���ӵ���ͬ��������ʱ���ŵ�����������һ�����ݾ�ֻ����һ�������������������������
	extern void(*MD5str) (char* output, unsigned char* input, int Len); //MD5������֤��
	extern bool InitData(); //��ʼ�����ݺ���

	extern u8 GetServerType(s32 sid);
	extern const char* getShutDownError(const s32 id);
	extern const char* getCloseSocketError(const s32 id);
	extern void SetConsoleColor(u16 index);

}

#endif


