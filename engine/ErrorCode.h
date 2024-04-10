#pragma once
#ifndef  _ERRORCODE_H

namespace app
{
	
		enum ErrorCode_Account
		{
			//�ɹ�
			EC_SUCCESS = 20000,
			//ʧ��
			EC_FAILED = 20001,
			//�˺��Ѵ���
			EC_ACCOUNT_EXIST = 20002,
			//�˺Ų�����
			EC_ACCOUNT_NOT_EXIST = 20003,
			//�������
			EC_PASSWORD_ERROR = 20004,
			//�˺ű���
			EC_ACCOUNT_BAN = 20005,
			//�˺�δ����
			EC_ACCOUNT_NOT_ACTIVE = 20006,
			//�˺��Ѽ���
			EC_ACCOUNT_ACTIVE = 20007,
			//�˺��ѵ�¼
			EC_ACCOUNT_LOGINED = 20008,
			//�˺�δ��¼
			EC_ACCOUNT_NOT_LOGIN = 20009,
			//�˺�������
			EC_ACCOUNT_ONLINE = 20010,
			//�˺Ų�����
			EC_ACCOUNT_OFFLINE = 20011,
			//�˺���ע��
			EC_ACCOUNT_REGISTERED = 20012,
			//�˺�δע��
			EC_ACCOUNT_NOT_REGISTERED = 20013,
			//�˺���ɾ��
			EC_ACCOUNT_DELETED = 20014,
			//�˺�δɾ��
			EC_ACCOUNT_NOT_DELETED = 20015,
			//�˺��ѽ���
			EC_ACCOUNT_DISABLED = 20016,
			//�˺�δ����
			EC_ACCOUNT_NOT_DISABLED = 20017,
			//�˺��Ѽ���
			EC_ACCOUNT_ACTIVATED = 20018,
			//�˺�δ����
			EC_ACCOUNT_NOT_ACTIVATED = 20019,
			//�˺�������
			EC_ACCOUNT_LOCKED = 20020,
			//�˺�δ����
			EC_ACCOUNT_NOT_LOCKED = 20021,
			//�˺��ѽ���
			EC_ACCOUNT_UNLOCKED = 20022,
			//�˺�δ����
			EC_ACCOUNT_NOT_UNLOCKED = 20023,
			//�˺��ѹ���
			EC_ACCOUNT_EXPIRED = 20024,
			//�˺�δ����
			EC_ACCOUNT_NOT_EXPIRED = 20025,
			//�˺��������ط���¼
			EC_ACCOUNT_LOGIN_OTHER = 20026,
		};
	;
}


#define _ERRORCODE_H
#endif

