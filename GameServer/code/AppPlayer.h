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
		virtual bool onClientCommand(net::ITcpClient* tc, const u16 cmd);

	
		void onSendMove(net::ITcpServer* ts, net::S_CLIENT_BASE* c);
		void onSendGetPlayerData(net::ITcpServer* ts, net::S_CLIENT_BASE* c);

		void OnRecvGetPlayerData(net::ITcpClient* tc);
	};

	extern IContainer* __AppPlayer;
	extern std::map<int, S_PLAYER_BASE*>  __Onlines;

}

#endif