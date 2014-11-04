
#include "impl_timer.h"

#include "impl_ioserver.h"

#include <xhguard.h>
#include <xhlog.h>


namespace xhnet
{
	ITimer* ITimer::Create(void)
	{
		return new CTimer();
	}


	static unsigned int gen_timertid()
	{
		static std::atomic<unsigned int> gen_timerid(0);
		return ++gen_timerid;
	}

	void CTimer::on_timer_callback(evutil_socket_t sock, short flag, void* p)
	{
		CTimer* ptimer = (CTimer*)(p);
		if ( ptimer )
		{
			ptimer->timer_clock();
		}
	}

	CTimer::CTimer(void)
		: m_status(status_null)
		, m_id(gen_timertid())
		, m_io(0)
		, m_cb(0)
	{

	}

	CTimer::~CTimer(void)
	{

	}

	bool CTimer::Init(IIOServer* io, ICBTimer* cb)
	{
		if (!io || !cb) return false;
		if (m_io || m_cb) return false;
		if (m_status != status_null) return false;

		m_io = dynamic_cast<CIOServer*>(io);
		if (!m_io) return false;

		cb->Retain();
		m_cb = cb;
		m_status = status_init;
		
		return true;
	}

	void CTimer::Fini(void)
	{
		if (m_status == status_null) return;

		if (m_io)
		{
			Retain();
			m_io->Post(std::bind(&CTimer::real_fini, this));
		}
	}

	bool CTimer::Async_Wait(unsigned int ms)
	{
		if (m_status != status_init) return false;

		if ( m_io )
		{
			Retain();
			m_io->Post(std::bind(&CTimer::real_wait, this, ms));
		}

		return true;
	}

	unsigned int CTimer::GetTimerID(void)
	{
		return m_id;
	}

	void CTimer::real_fini()
	{
		XH_GUARD([&]{Release(); });

		if (m_status == status_null) return;
		m_status = status_end;
	}

	void CTimer::real_wait(unsigned int ms)
	{
		XH_GUARD([&]{Release(); });

		if (m_status != status_init)
		{
			XH_LOG_ERROR(logname_base, "Timer: " << m_id << " start failed, has start");
			return;
		}

		m_status = status_begin;

		m_io->Reset_Event(&m_ev_timer, INVALID_SOCKET, 0, &(CTimer::on_timer_callback), this);
		m_io->Add_Timer(this);

		timeval passtime;
		passtime.tv_sec = m_ms / 1000;
		passtime.tv_usec = (m_ms % 1000) * 1000;
		m_io->Add_Event(&m_ev_timer, &passtime);
	}

	void CTimer::timer_clock()
	{
		if (m_status == status_begin)
		{
			m_cb->On_Timer(GetTimerID(), timer_begin);

			m_status = status_common;

			timeval passtime;
			passtime.tv_sec = m_ms / 1000;
			passtime.tv_usec = (m_ms % 1000) * 1000;
			m_io->Add_Event(&m_ev_timer, &passtime);
		}
		else if (m_status == status_end)
		{
			//XH_GUARD([&]{
			//	m_status = status_null;

			//	if ( m_cb )
			//	{
			//		m_cb->Release();
			//		m_cb = 0;
			//	}
			//	if (m_io)
			//	{
			//		m_io->Del_Event(&m_ev_timer);
			//		m_io->Del_Timer(this);
			//	}
			//});

			m_cb->On_Timer(GetTimerID(), timer_end);

			m_status = status_null;

			if (m_cb)
			{
				m_cb->Release();
				m_cb = 0;
			}
			if (m_io)
			{
				m_io->Del_Event(&m_ev_timer);
				m_io->Del_Timer(this);
			}
		}
		else
		{
			m_cb->On_Timer(GetTimerID(), timer_ok);

			timeval passtime;
			passtime.tv_sec = m_ms / 1000;
			passtime.tv_usec = (m_ms % 1000) * 1000;
			m_io->Add_Event(&m_ev_timer, &passtime);
		}
	}
};


