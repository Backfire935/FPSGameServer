
#include "MySqlConnetor.h"

namespace db
{
	MySqlConnetor::MySqlConnetor():mysql(NULL), myres(NULL),m_TotalRows(0), m_TotalFields(0), m_curRowIdx(0), m_IsConnect(false)
	{	
	}

	MySqlConnetor::~MySqlConnetor()
	{
		if (mysql) mysql = NULL;
		if (myres) myres = NULL;
	}
	//清除sql
	void MySqlConnetor::Clear()
	{
		m_IsConnect = false;
		if (mysql)
		{
			mysql_close(mysql);
			mysql = NULL;
		}
		if (myres) myres = NULL;
	}
	//清理结果集
	void MySqlConnetor::ClearRes()
	{
		bool ifNeedQueryNext = (myres != nullptr);
		if (myres)
		{
			mysql_free_result(myres);
		}

		if (ifNeedQueryNext)
		{
			while (mysql_next_result(mysql) == 0)
			{
				myres = mysql_store_result(mysql);
				if (myres)
				{
					mysql_free_result(myres);
				}
			}
		}
		else
		{
			do
			{
				myres = mysql_store_result(mysql);
				if (myres)
				{
					mysql_free_result(myres);
				}
			} while (mysql_next_result(mysql) == 0);
		}

		myres = NULL;
	}
	//*********************************************************************
	//*********************************************************************
	//*********************************************************************
	//0、关闭 清理数据
	void MySqlConnetor::CloseQuery()
	{
		m_curRowIdx = 0;
		m_FieldsValue.clear();
	}
	//1、连接数据库
	bool MySqlConnetor::ConnectMySql(const char * ip, const char * usename, const char * password, const char * dbname, int port)
	{
		strcpy(m_IP, ip);
		strcpy(m_UserName, usename);
		strcpy(m_UserPassword, password);
		strcpy(m_DBName, dbname);
		m_Port = port;
		 
		Clear();
		if(!ReConnect()) return false;
		
		return true;
	}
	//2、重连数据库
	bool MySqlConnetor::ReConnect()
	{
		if (m_IsConnect)
		{
			m_IsGetErrorCode = true;
			int error = mysql_ping(mysql);
			if (error == 0) return true;
			m_IsGetErrorCode = false;
			return false;
		}
		mysql = mysql_init(NULL);
		if (mysql == NULL)
		{
			return false;
		}
		char value = 1;
		int f_outtime = 5;
		mysql_options(mysql, MYSQL_OPT_RECONNECT, &value);
		mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &f_outtime);
		mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");
		bool isconnect = mysql_real_connect(mysql, m_IP, m_UserName, m_UserPassword, m_DBName, m_Port, NULL, CLIENT_MULTI_STATEMENTS);
		if (!isconnect)
		{
			printf("mysql reconnect error:%s \n", GetErrorStr());
			m_IsConnect = false;
			m_IsGetErrorCode = false;
			return false;
		}
		mysql_options(mysql, MYSQL_OPT_RECONNECT, &value);
		m_IsConnect = true;
		m_IsGetErrorCode = true;
		return isconnect;
	}
	//3、执行查询
	int MySqlConnetor::ExecQuery(const std::string & pData)
	{
		if (pData.length() < 1) return -1;
		ClearRes();
		if(!ReConnect()) return -2;
		int  nRet = mysql_real_query(mysql, pData.c_str(), pData.length());
		if (nRet != 0)
		{
			return nRet;
		}

		if ((pData.find("select") == -1) && (pData.find("SELECT") == -1)) return nRet;
		myres = mysql_store_result(mysql);
		if (myres == nullptr)
		{
			LOG_MSG("mysql ExecQuery error...%s\n",pData.c_str());
			return -1;
		}
		
		m_TotalFields = mysql_num_fields(myres);
		m_TotalRows = static_cast<int>(mysql_affected_rows(mysql));

		GetQueryFields();
		return nRet;
	}
	//4、获取到查询到的列数
	void MySqlConnetor::GetQueryFields()
	{
		filedVec.clear();
		m_FieldsValue.clear();
		MYSQL_FIELD* filed = NULL;

		MYSQL_ROW curRow = mysql_fetch_row(myres);
		if (!curRow) return;
		
		int num_fields = mysql_num_fields(myres);
		int i = 0;
		while (filed = mysql_fetch_field(myres))
		{
			auto tempRes = curRow[i];
			if (tempRes)
			{
				std::string filedStr(filed->name, filed->name_length);
				m_FieldsValue[filedStr] = tempRes;
				filedVec.push_back(filedStr);
			}
			++i;
		}
	}
	//5、查询下一行的数据
	int MySqlConnetor::QueryNext()
	{
		m_FieldsValue.clear();
		if (myres == nullptr)
		{
			LOG_MSG("mysql QueryNext error...\n");
			return -1;
		}
		
		MYSQL_ROW curRow = mysql_fetch_row(myres);
		if (!curRow)
		{
			return -1;
		}
		int size = filedVec.size();
		for (int i = 0; i < size; ++i)
		{
			auto tempRes = curRow[i];
			if (tempRes)
			{
				m_FieldsValue[filedVec[i]] = tempRes;
			}
		}
		return 0;
	}
	
	//*****************************************************************
	void	MySqlConnetor::r(const char *fieldname, uint8_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (uint8_t)atoi(iter->second.data());
		}
	}
	void	MySqlConnetor::r(const char *fieldname, int8_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (int8_t)atoi(iter->second.data());
		}
	}
	void	MySqlConnetor::r(const char *fieldname, uint16_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (uint16_t)atoi(iter->second.data());
		}
	}
	void	MySqlConnetor::r(const char *fieldname, int16_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (int16_t)atoi(iter->second.data());
		}
	}
	void	MySqlConnetor::r(const char *fieldname, uint32_t &value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (uint32_t)atoi(iter->second.data());
		}
	}
	void MySqlConnetor::r(const char * fieldname, int32_t & value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = (int32_t)atoi(iter->second.data());
		}
	}
	void MySqlConnetor::r(const char * fieldname, int64_t & value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end()) 
		{
			value = _atoi64(iter->second.data());
		}
	}
	void MySqlConnetor::r(const char * fieldname, uint64_t & value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = _atoi64(iter->second.data());
		}
	}
	void MySqlConnetor::r(const char * fieldname, char * value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end()) 
		{
			strcpy(value, iter->second.data());
		}
	}
	void MySqlConnetor::r(const char * fieldname, std::string & value)
	{
		auto iter = m_FieldsValue.find(fieldname);
		if (iter != m_FieldsValue.end())
		{
			value = iter->second.data();
		}
	}
	


	
}

