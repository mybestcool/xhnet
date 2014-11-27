
#pragma once

#include "qyfhhead.h"

namespace xhnet_qyfh
{
	class CMessageHead_24
	{
	public:
		enum 
		{
			HEAD_SIZE = 6,
		};
		CMessageHead_24()	{ }
		~CMessageHead_24()	{ }

		unsigned short size()		{ return HEAD_SIZE; }

		unsigned short msglen()		{ return m_msglen; }
		unsigned short msgid()		{ return m_msgid; }

		bool decode_header(CIOBuffer& iobuf)
		{
			try
			{
				iobuf >> m_msglen >> m_msgid;
			}
			catch (...)
			{
				return false;
			}
			
			return true;
		}

		bool encode_header(CIOBuffer& iobuf)
		{
			try
			{
				iobuf << m_msglen << m_msgid;
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		void Reset()
		{
			m_msglen = 0;
			m_msgid = 0;
		}

	public:
		unsigned short	m_msglen;
		unsigned int	m_msgid;
	};
};
