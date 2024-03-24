#ifndef  ____MYSQLCONNECTOR_H
#define  ____MYSQLCONNECTOR_H

#include <map> 
#include <vector> 
#include <string>
#include "IDefine.h"
#include "mysql\mysql.h"
#pragma comment(lib, "libmysql.lib")

namespace db
{
	const int	datalen = 32;
	//MySQL连接器
	class MySqlConnetor
	{
	public:
		MySqlConnetor();
		~MySqlConnetor();
	
		int		ExecQuery(const std::string& pData);
		void	GetQueryFields();
		int 	QueryNext();
		void	CloseQuery();
		void	r(const char *fieldname, uint8_t &value);
		void	r(const char *fieldname, int8_t &value);
		void	r(const char *fieldname, uint16_t &value);
		void	r(const char *fieldname, int16_t &value);
		void	r(const char *fieldname, uint32_t &value);
		void	r(const char *fieldname, int32_t &value);
		void	r(const char *fieldname, int64_t &value);
		void	r(const char *fieldname, uint64_t &value);
		void	r(const char *fieldname, char *  value);
		void	r(const char *fieldname, std::string&  value);
	public:
		bool	ConnectMySql(const char *ip, const char	*usename, const char *password, const char *dbname, int  port = 3306);
		void	Clear();
		bool	ReConnect();
		void	ClearRes();


		int				GetErrorCode() { if (m_IsGetErrorCode) return mysql_errno(mysql);  return -1000; }
		const char*		GetErrorStr() { return mysql_error(mysql); }

		int				GetQueryRowNum()   { return m_TotalRows; }
		int				GetQueryFieldNum() { return m_TotalFields; }

		MYSQL *			GetMySql() { return mysql; }
		MYSQL_RES *	    GetMyRes() { return myres; }
	public:
		MYSQL       *mysql;
		MYSQL_RES	*myres;
		int		    m_TotalRows;
		int         m_TotalFields;
		int		    m_curRowIdx;
		char m_IP[datalen];     //数据库IP
		char m_UserName[datalen];//数据库用户名
		char m_UserPassword[datalen];//数据库密码
		char m_DBName[datalen];//数据库名
		uint16_t  m_Port;//数据库端口号
		bool m_IsConnect;//连接状态
		bool m_IsGetErrorCode;
		std::vector<std::string> filedVec;//字段列表数据
		std::map<std::string, std::string>  m_FieldsValue;//字段数据
	};
}

#endif