
#pragma once

#include "impl_nethead.h"

namespace xhnet
{
	class CIOServer;

	class CTimer : public ITimer
	{
	public:
		enum status
		{
			status_null = 0,
			status_init,
			status_begin,
			status_common,
			status_end,
		};

	public:
		CTimer(void);
		virtual ~CTimer(void);

		virtual bool Init(IIOServer* io, ICBTimer* cb);
		virtual void Fini(void);

		virtual bool Async_Wait(unsigned int ms);

		virtual unsigned int GetTimerID(void);

	public:
		static void on_timer_callback(evutil_socket_t sock, short flag, void* p);

	protected:
		void real_fini();
		void real_wait(unsigned int ms);

		void timer_clock();

	protected:
		unsigned char	m_status;
		unsigned int	m_id;

		CIOServer*		m_io;
		ICBTimer*		m_cb;

		unsigned int	m_ms;
		::event			m_ev_timer;
	private:
		CTimer(const CTimer&) = delete;
		CTimer& operator=(const CTimer&) = delete;

	public:
		static COPool<CTimer, std::mutex> m_pool;
	};

};
