#ifndef __IOPOOL_H
#define  __IOPOOL_H

//�ж��ǲ���win32ƽ̨��
#ifdef  ____WIN32_


#include "IDefine.h"
#define ACCEPT_BUF_LENGTH (sizeof(struct sockaddr_in) + 16 ) *2 //������Զ�˵�ַ�ͱ��ص�ַ����Ϣ��������*2  +16��WindowsϵͳԤ����16�ֽ�
#define POOL_SIZE 1000 //����ص�������С
class  IOContext
{
public:
	IOContext();
	~IOContext();

public:
	WSAOVERLAPPED	m_OverLapped;
	SOCKET					m_Socket;
	int							m_Mode; //��ʾ��ǰͶ�ݵ���ʲôģʽ
};

class AcceptContext : public IOContext
{
public:
	AcceptContext(int mode, SOCKET lisfd, SOCKET sfd);
	~AcceptContext(void);

public:
	SOCKET listenfd;
	unsigned char m_buf[ACCEPT_BUF_LENGTH];//��ű��ص�ַ��Զ�˵�ַ

public:
	void clear();//��������
	void setSocket(SOCKET lisfd, SOCKET sfd);//������socket��ҪͶ�ݵ�socket
	static AcceptContext* pop(); //ͨ������ֱ�ӻ�ȡһ������ʵ��
	static void push(AcceptContext* acc); //ʹ����Żض�����ճ�
	static int getCount();

};

class RecvContext : public IOContext
{
public:
	RecvContext(int mode);
	~RecvContext(void);

public:
	char* m_Buffs;//���ٺõĿռ䣬��xml���õģ��������ݵ�buff������
	WSABUF m_wsaBuf;//�յ����ݵ�ʱ����䵽m_wsaBuf��֮��֪ͨ�������������ȡ����

public:
	void clear();//��������
	static RecvContext* pop(); //ͨ������ֱ�ӻ�ȡһ������ʵ��
	static void push(RecvContext* acc); //ʹ����Żض�����ճ�
	static int getCount();

};

class SendContext : public IOContext
{
public:
	SendContext(int mode);
	~SendContext(void);

public:
	char* m_Buffs;//���ٺõĿռ䣬��xml���õģ��������ݵ�buff������
	WSABUF m_wsaBuf;//�յ����ݵ�ʱ����䵽m_wsaBuf��֮��֪ͨ�������������ȡ����

	public:
	void clear();//��������
	int setSend(SOCKET fd, char* data, int sendBytes);
	static SendContext* pop(); //ͨ������ֱ�ӻ�ȡһ������ʵ��
	static void push(SendContext* acc); //ʹ����Żض�����ճ�
	static int getCount();

};

#endif

#endif

