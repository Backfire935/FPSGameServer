#ifndef __ICONTAINER_H
#define __ICONTAINER_H
//定义一个哈希数组模板容器
//一开始就开辟一块连续的内存空间，通过模板的数据类型和类型的大小来决定开辟多大的内存空间，在查找的时候传入一个索引，通过索引精确的定位到位置所在，相当于只用查找一次，时间复杂度恒定O(1)
//在服务端开辟的大小是IDefine.h文件中的MAX_USER_SOCKETFD，乘以INetBase.h中的S_CLEINT_BASE_INDEX大小,SOCKET_ID由系统分配，永远不会重复，作为索引下表

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
	int	length;//总的数据长度
	int	size;//这个数据结构T类型的长度
	void* pointer;//连续的内存数据结构

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
		pointer = malloc(length * size);//开辟内存空间
	}

	virtual ~HashArray()
	{
		if(pointer != nullptr)
		{
			free(pointer);//释放内存
			pointer = nullptr;
		}
	}

	T* Value(const int index)//模板根据索引返回值
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
