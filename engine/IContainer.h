#ifndef __ICONTAINER_H
#define __ICONTAINER_H
//����һ����ϣ����ģ������
//һ��ʼ�Ϳ���һ���������ڴ�ռ䣬ͨ��ģ����������ͺ����͵Ĵ�С���������ٶ����ڴ�ռ䣬�ڲ��ҵ�ʱ����һ��������ͨ��������ȷ�Ķ�λ��λ�����ڣ��൱��ֻ�ò���һ�Σ�ʱ�临�ӶȺ㶨O(1)
//�ڷ���˿��ٵĴ�С��IDefine.h�ļ��е�MAX_USER_SOCKETFD������INetBase.h�е�S_CLEINT_BASE_INDEX��С,SOCKET_ID��ϵͳ���䣬��Զ�����ظ�����Ϊ�����±�

#include"INetBase.h"
#include "malloc.h"

//namespace net
//{
//	struct S_CLIENT_BASE;
//	class ITcpServer;
//}

template<class T>
class HashArray
{
public:
	int	length;//�ܵ����ݳ���
	int	size;//������ݽṹT���͵ĳ���
	void* pointer;//�������ڴ����ݽṹ

public:
	HashArray()
	{
		size = sizeof(T);
		length = 0;
		pointer = nullptr;
	}

	HashArray(int counter)
	{
		if (counter < 1) return;
		size = sizeof(T);
		if (size == 0) return;
		length = counter;
		pointer = malloc(length * size);//�����ڴ�ռ�
	}

	virtual ~HashArray()
	{
		if(pointer != nullptr)
		{
			free(pointer);//�ͷ��ڴ�
			pointer = nullptr;
		}
	}

	T* Value(const int index)//ģ�������������ֵ
	{
		T* tmp = (T*)pointer;
		return &tmp[index];
	}

};

class IContainer
{
public:
	IContainer(){}
	virtual ~IContainer(){}
	virtual void onInit(){}
	virtual void onUpdate(){}
	virtual bool onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd) {return false;}
	virtual bool onClientCommand(net::ITcpClient* ctc, const u16 cmd) {return false;}
	virtual bool  OnDBCommand(void* buff) { return false; }
};

#endif 
