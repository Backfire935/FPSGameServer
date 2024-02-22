#include "IOPool.h"

#ifdef ____WIN32_
#include<concurrent_queue.h>//��ȫ����
Concurrency::concurrent_queue<AcceptContext*> __accepts;
Concurrency::concurrent_queue<RecvContext*> __recvs;
Concurrency::concurrent_queue<SendContext*> __sends;
//�ڶ��µڶ���
IOContext::IOContext()
{
	m_Socket = INVALID_SOCKET;
	m_Mode	 = 0;
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

IOContext::~IOContext()
{
	//��Ϊ�Ƕ�����ճأ�û��ʲô��ֵ���Ͳ���Ҫдʲô��
}

//******************************
//�µ�����
AcceptContext::AcceptContext(int mode, SOCKET lisfd, SOCKET sfd)
{
	m_Mode		= mode;
	listenfd	= lisfd;
	m_Socket	= sfd;

	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

AcceptContext::~AcceptContext(void)
{
	listenfd	= NULL;
	m_Socket	= NULL;	
	m_Mode		= 0;
	
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

void AcceptContext::clear()
{
	listenfd	= NULL;
	m_Socket	= NULL;	
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

void AcceptContext::setSocket(SOCKET lisfd, SOCKET sfd)
{
	listenfd	= lisfd;
	m_Socket	= sfd;
}

AcceptContext* AcceptContext::pop()//��ȡһ������صĶ���
{
	AcceptContext* buff = nullptr;
	if(__accepts.empty() == true)//�����û�п��ж���
	{
		//û�ж����newһ������
		buff = new AcceptContext(func::SC_WAIT_ACCEPT, NULL, NULL);
	}
	else
	{
		__accepts.try_pop(buff);//����ж���Ļ��ͳ��Ե���һ������buff��
		if( buff == NULL)
		{
			buff = new AcceptContext(func::SC_WAIT_ACCEPT, NULL, NULL);
		}
	}
	return buff;
}

void AcceptContext::push(AcceptContext* acc)//�Ѷ���Żض����
{
	if(acc == NULL) return;
	if(__accepts.unsafe_size() > POOL_SIZE) //����̫��Ļ�
	{
		delete acc;
		return;
	}
	//С����������Ļ�������������,�ٷ������
	acc->clear();
	__accepts.push(acc);
}

int AcceptContext::getCount()
{
	return 	__accepts.unsafe_size();
}

//*******************************************
RecvContext::RecvContext(int mode)
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= mode;
	m_Buffs = new char[func::__ServerInfo->ReceOne];//�����ڴ�ռ�
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));//�����ص��ṹ
	ZeroMemory(m_Buffs, func::__ServerInfo->ReceOne);//�����ص��ṹ
	m_wsaBuf.buf = m_Buffs;//�����������ݾͻ���䵽������
	m_wsaBuf.len = func::__ServerInfo->ReceOne;//���ó���
	
}

RecvContext::~RecvContext()
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= -1;
	
}

void RecvContext::clear()
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= -1;

	ZeroMemory(m_Buffs, func::__ServerInfo->ReceOne);//�����ص��ṹ
	m_wsaBuf.buf = m_Buffs;//�����������ݾͻ���䵽������
	m_wsaBuf.len = func::__ServerInfo->ReceOne;//���ó���
}

RecvContext* RecvContext::pop()
{
	RecvContext* buff = nullptr;
	if(__recvs.empty() == true)//�����û�п��ж���
		{
			//û�ж����newһ������
			buff = new RecvContext(func::SC_WAIT_RECV);
		}
	else
	{
		__recvs.try_pop(buff);//����ж���Ļ��ͳ��Ե���һ������buff��
		if( buff == NULL)
		{
			buff = new RecvContext(func::SC_WAIT_RECV);
		}
	}
	return buff;
}

void RecvContext::push(RecvContext* acc)
{
	if(acc == NULL) return;
	if(__recvs.unsafe_size() > POOL_SIZE) //����̫��Ļ�
		{
			delete acc;
			return;
		}
	//С����������Ļ�������������,�ٷ������
	acc->clear();
	__recvs.push(acc);
}

int RecvContext::getCount()
{
	return __recvs.unsafe_size(); 
}

//********************************************
SendContext::SendContext(int mode)
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= mode;
	
	m_Buffs = new char[func::__ServerInfo->SendOne];//�����ڴ�ռ�
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));//�����ص��ṹ
	ZeroMemory(m_Buffs, func::__ServerInfo->SendOne);
	m_wsaBuf.buf = m_Buffs;//�����������ݾͻ���䵽������
	m_wsaBuf.len = func::__ServerInfo->SendOne;//���ó���
}

SendContext::~SendContext()
{
	m_Socket	= INVALID_SOCKET;
	m_Mode		= -1;
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));//�����ص��ṹ
}

void SendContext::clear()
{
	m_Socket	= INVALID_SOCKET;
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));//�����ص��ṹ
	ZeroMemory(m_Buffs, func::__ServerInfo->SendOne);
	m_wsaBuf.buf = m_Buffs;//�����������ݾͻ���䵽������
	m_wsaBuf.len = func::__ServerInfo->SendOne;//���ó���
}



SendContext* SendContext::pop()
{
	SendContext* buff = nullptr;
	if(__sends.empty() == true)//�����û�п��ж���
		{
		//û�ж����newһ������
		buff = new SendContext(func::SC_WAIT_SEND);
		}
	else
	{
		__sends.try_pop(buff);//����ж���Ļ��ͳ��Ե���һ������buff��
		if( buff == NULL)
		{
			buff = new SendContext(func::SC_WAIT_SEND);
		}
	}
	return buff;
}

void SendContext::push(SendContext* acc)
{
	if(acc == NULL) return;
	if(__sends.unsafe_size() > POOL_SIZE) //����̫��Ļ�
		{
		delete acc;
		return;
		}
	//С����������Ļ�������������,�ٷ������
	acc->clear();
	__sends.push(acc);
}

int SendContext::getCount()
{
	return __sends.unsafe_size();
}

//������Ϣ�ĺ���
int SendContext::setSend(SOCKET fd, char* data, int sendBytes)
{
	m_Socket = fd;
	if(&m_wsaBuf)
	{
		if(sendBytes != 0 && data != NULL)
		{
			memcpy(m_Buffs, data, sendBytes);//���ʱ��Ͱ����ݸ��ƹ�ȥ��
			m_wsaBuf.buf = m_Buffs;//����ʲôʱ���;Ϳ�����ϵͳ��
			m_wsaBuf.len = sendBytes;
		}
	}

	return sendBytes;
}

#endif
