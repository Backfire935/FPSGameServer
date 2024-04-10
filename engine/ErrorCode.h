#pragma once
#ifndef  _ERRORCODE_H

namespace app
{
	
		enum ErrorCode_Account
		{
			//成功
			EC_SUCCESS = 20000,
			//失败
			EC_FAILED = 20001,
			//账号已存在
			EC_ACCOUNT_EXIST = 20002,
			//账号不存在
			EC_ACCOUNT_NOT_EXIST = 20003,
			//密码错误
			EC_PASSWORD_ERROR = 20004,
			//账号被封
			EC_ACCOUNT_BAN = 20005,
			//账号未激活
			EC_ACCOUNT_NOT_ACTIVE = 20006,
			//账号已激活
			EC_ACCOUNT_ACTIVE = 20007,
			//账号已登录
			EC_ACCOUNT_LOGINED = 20008,
			//账号未登录
			EC_ACCOUNT_NOT_LOGIN = 20009,
			//账号已在线
			EC_ACCOUNT_ONLINE = 20010,
			//账号不在线
			EC_ACCOUNT_OFFLINE = 20011,
			//账号已注册
			EC_ACCOUNT_REGISTERED = 20012,
			//账号未注册
			EC_ACCOUNT_NOT_REGISTERED = 20013,
			//账号已删除
			EC_ACCOUNT_DELETED = 20014,
			//账号未删除
			EC_ACCOUNT_NOT_DELETED = 20015,
			//账号已禁用
			EC_ACCOUNT_DISABLED = 20016,
			//账号未禁用
			EC_ACCOUNT_NOT_DISABLED = 20017,
			//账号已激活
			EC_ACCOUNT_ACTIVATED = 20018,
			//账号未激活
			EC_ACCOUNT_NOT_ACTIVATED = 20019,
			//账号已锁定
			EC_ACCOUNT_LOCKED = 20020,
			//账号未锁定
			EC_ACCOUNT_NOT_LOCKED = 20021,
			//账号已解锁
			EC_ACCOUNT_UNLOCKED = 20022,
			//账号未解锁
			EC_ACCOUNT_NOT_UNLOCKED = 20023,
			//账号已过期
			EC_ACCOUNT_EXPIRED = 20024,
			//账号未过期
			EC_ACCOUNT_NOT_EXPIRED = 20025,
			//账号在其他地方登录
			EC_ACCOUNT_LOGIN_OTHER = 20026,
		};
	;
}


#define _ERRORCODE_H
#endif

