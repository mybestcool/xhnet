
#pragma once

#include <queue>
#include <mutex>

#include "xhnet.h"
#include "xhpool.h"
#include "impl_tcpsocket.h"

struct event_base;

namespace xhnet
{
	class CIOServer : public IIOServer, public ICBTimer
	{
	public:
		enum status
		{
			status_null = 0,
			status_common,
		};

	public:
		CIOServer();
		virtual ~CIOServer();

		virtual bool Init(void);
		virtual void Fini(void);

		virtual void Run(void);
		virtual bool IsFinshed(void);

		virtual void Post(postio io);

		virtual unsigned int GetIOServerID(void);
	public:
		virtual void On_Timer(unsigned int timerid, int errid);

	public:
		//event_assign event_new/event_free

		void Reset_Event(::event* ev, evutil_socket_t sock, short flag, event_callback_fn cb, void* p);
		// event_add
		void Add_Event(::event* ev, const struct timeval *timeout);
		// event_del
		void Del_Event(::event* ev);

		//
		void Add_TcpSocket(ITcpSocket* socket);
		void Del_TcpSocket(ITcpSocket* socket);

		void Add_UdpSocket(IUdpSocket* socket);
		void Del_UdpSocket(IUdpSocket* socket);

		void Add_Timer(ITimer* timer);
		void Del_Timer(ITimer* timer);

	protected:
		void real_fini();

		void real_post(bool bfinished);
	protected:
		::event_base*		m_evbase;
		unsigned char		m_status;
		bool				m_binit;

		unsigned int		m_id;

		std::mutex			m_postmutex;
		int					m_curpost;
		std::queue<postio>	m_postdata[2];
		bool				m_haspostdata;
		// √ø∏Ù1∫¡√Î
		ITimer*				m_posttimer;

		std::unordered_map<unsigned int, ITcpSocket*> m_tcpsockets;
		std::unordered_map<unsigned int, IUdpSocket*> m_udpsockets;
		std::unordered_map<unsigned int, ITimer*> m_timers;

	private:
		CIOServer(const CIOServer&) = delete;
		CIOServer& operator=(const CIOServer&) = delete;

	public:
		static COPool<CIOServer, std::mutex> m_pool;
	};
};

