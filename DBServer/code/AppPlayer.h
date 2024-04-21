#ifndef  ____APPPLAYER_H
#define  ____APPPLAYER_H


#include "AppGlobal.h"
#include "GameData.h"
#include <map>

namespace app
{

	class AppPlayer :public IContainer
	{
	public:
		AppPlayer();
		~AppPlayer();
		virtual void  onInit();
		virtual void  onUpdate();
		virtual bool  onServerCommand(net::ITcpServer* ts, net::S_CLIENT_BASE* c, const u16 cmd);
		virtual bool  OnDBCommand(void* buff);

		void onReigster(net::ITcpServer* ts, net::S_CLIENT_BASE* c);
		void onLogin(net::ITcpServer* ts, net::S_CLIENT_BASE* c);
		void onMove(net::ITcpServer* ts, net::S_CLIENT_BASE* c);
		void onGetPlayerData(net::ITcpServer* ts, net::S_CLIENT_BASE* c);
		void OnLeave(net::ITcpServer* ts, net::S_CLIENT_BASE* c);
	};

	extern IContainer* __AppPlayer;
	extern std::map<int, S_PLAYER_BASE*>  __Onlines;

}

#endif