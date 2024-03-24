
#include "DBBuffer.h"


namespace db
{
	void  DBBuffer::b(const uint16_t cmd)
	{
		uint16_t newcmd = cmd;
		char* a = (char*)&newcmd;
		m_Buf[0] = a[0];
		m_Buf[1] = a[1];
		m_SendE = 2;
	}
	void  DBBuffer::e()
	{
	}
	void  DBBuffer::s(const int8_t v)
	{
		if (m_SendE+1< m_CurLength)
		{
			m_Buf[m_SendE] = v;
			m_SendE++;
		}
	}
	void  DBBuffer::s(const uint8_t v)
	{
		if (m_SendE+1 < m_CurLength)
		{
			m_Buf[m_SendE] = v;
			m_SendE++;
		}
	}
	void  DBBuffer::s(const int16_t v)
	{
		if (m_SendE+2 < m_CurLength)
		{
			char* p = (char*)&v;
			for (int i = 0; i<2; i++)
				m_Buf[m_SendE + i] = p[i];
			m_SendE += 2;
		}
	}
	void  DBBuffer::s(const uint16_t v)
	{
		if (m_SendE + 2 < m_CurLength)
		{
			char* p = (char*)&v;
			for (int i = 0; i<2; i++)
				m_Buf[m_SendE + i] = p[i];
			m_SendE += 2;
		}
	}
	void  DBBuffer::s(const int v)
	{
		if (m_SendE + 4 < m_CurLength)
		{
			char* p = (char*)&v;
			for (int i = 0; i<4; i++)
				m_Buf[m_SendE + i] = p[i];
			m_SendE += 4;
		}
	}

	void  DBBuffer::s(const int64_t v)
	{
		if (m_SendE + 8 < m_CurLength)
		{
			char* p = (char*)&v;
			for (int i = 0; i<8; i++)
				m_Buf[m_SendE + i] = p[i];
			m_SendE += 8;
		}
	}

	void  DBBuffer::s(const bool v)
	{
		if (m_SendE + 1 < m_CurLength)
		{
			m_Buf[m_SendE] = v;
			m_SendE++;
		}
	}
	
	void  DBBuffer::s(void* v, const int len)
	{
		if (m_SendE + len< m_CurLength)
		{
			memcpy_s(&m_Buf[m_SendE], len, v, len);

			m_SendE += len;
		}
	}
	void* DBBuffer::fs(const int len)
	{
		if (m_SendE + len < m_CurLength)
		{
			void* v = &m_Buf[m_SendE];
			m_SendE += len;
			return v;
		}
		return nullptr;
	}
	//*************************************
	void  DBBuffer::r(int8_t& v)
	{
		if (m_ReceE+1 >= m_CurLength) return;
		v = (*(int8_t *)(m_Buf + m_ReceE));
		m_ReceE++;
	}
	void  DBBuffer::r(uint8_t& v)
	{
		if (m_ReceE+1 >= m_CurLength) return;
		v = (*(uint8_t *)(m_Buf + m_ReceE));
		m_ReceE++;
	}
	void  DBBuffer::r(int16_t& v)
	{
		if (m_ReceE+2 >= m_CurLength) return;
		v = (*(int16_t *)(m_Buf + m_ReceE));
		m_ReceE += 2;
	}
	void  DBBuffer::r(uint16_t& v)
	{
		if (m_ReceE+2 >= m_CurLength) return;
		v = (*(uint16_t *)(m_Buf + m_ReceE));
		m_ReceE += 2;
	}
	void  DBBuffer::r(int& v)
	{
		if (m_ReceE+ 4 >= m_CurLength) return;
		v = (*(int *)(m_Buf + m_ReceE));
		m_ReceE += 4;
	}

	void  DBBuffer::r(int64_t& v)
	{
		if (m_ReceE + 8 >= m_CurLength) return;
		v = (*(int64_t *)(m_Buf + m_ReceE));
		m_ReceE += 8;
	}

	void  DBBuffer::r(bool& v)
	{
		if (m_ReceE + 1 >= m_CurLength) return;
		v = (*(bool *)(m_Buf + m_ReceE));
		m_ReceE++;
	}

	void  DBBuffer::r(void* v, const int len)
	{
		if (m_ReceE + len >= m_CurLength) return;
	
		memcpy_s(v, len, &m_Buf[m_ReceE], len);

		m_ReceE += len;
	}
	void*  DBBuffer::fr(const int len)
	{
		if (m_ReceE + len>= m_CurLength) return nullptr;
		void* v = &m_Buf[m_ReceE];
		
		m_ReceE += len;
		return v;
	}

}