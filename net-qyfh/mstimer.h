
#pragma once

#include "qyfhhead.h"

namespace xhnet_qyfh
{
	// ºÁÃë¼ÆÊ±Æ÷
	class CMsTimer : public ICBTimer
	{
	public:
		typedef std::function< void(unsigned int) >	tcp_timeout_callback;

		CMsTimer(IIOServer* io, tcp_timeout_callback cb)
			: m_io( io )
			, m_cb( cb )
		{
			m_timer = ITimer::Create();

			XH_ASSERT(m_io&&m_timer)(m_io)(m_timer);

			m_io->Retain();
		}

		virtual ~CMsTimer()
		{
			if ( m_timer )
			{
				m_timer->Release();
				m_timer = 0;
			}

			if ( m_io )
			{
				m_io->Release();
				m_io = 0;
			}
		}

		bool Start(unsigned int ms)
		{
			if ( !m_io || !m_timer || ms<=0 )
			{
				return false;
			}

			if (m_timer)
			{
				m_timer->Init(m_io, this);

				m_timer->Async_Wait(ms);
			}
		}

		void Stop()
		{
			if (m_timer)
				m_timer->Fini();
		}

		unsigned int GetTimerID()
		{
			return m_timer ? m_timer->GetTimerID() : 0;
		}

	public:
		virtual void On_Timer(unsigned int timerid, int errid)
		{
			if ( m_cb )
			{
				m_cb(timerid);
			}
		}

	private:
		IIOServer*				m_io;
		ITimer*					m_timer;
		tcp_timeout_callback	m_cb;
	};

};

