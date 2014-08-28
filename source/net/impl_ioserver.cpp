
#include "impl_ioserver.h"

#include <xhlog.h>

#include <event.h>

#if defined _PLATFORM_WINDOWS_
#pragma comment( lib, "libevent_core.lib" )
#pragma comment( lib, "libevent_extras.lib" )
#pragma comment( lib,"ws2_32.lib" )
#elif defined _PLATFORM_LINUX_
#pragma comment( lib, "libevent_core" )
#pragma comment( lib, "libevent_extras" )
#endif


namespace xhnet
{

	IIOServer* IIOServer::Create()
	{
		return new CIOServer();
	}

	std::vector<std::string> IIOServer::Resolve_DNS(const std::string& hostname)
	{
		std::vector<std::string> out;


		// 这方式获取 ip会重复
		//evutil_addrinfo* result = 0;
		//int ret = evutil_getaddrinfo(hostname.c_str(), "0", NULL, &result);
		//for (evutil_addrinfo* it = result; it != 0; it = it->ai_next)
		//{
		//	std::string ip;
		//	unsigned int port = 0;

		//	if (sockaddr2ipport((socketaddr*)(it->ai_addr), it->ai_addrlen, ip, port))
		//	{
		//		out.push_back(ip);
		//	}
		//}

		struct addrinfo* result = 0;
		int ret = getaddrinfo(hostname.c_str(), NULL, NULL, &result);
		for (struct addrinfo* it = result; it != 0; it = it->ai_next)
		{
			std::string ip;
			unsigned int port = 0;

			if (sockaddr2ipport((socketaddr*)(it->ai_addr), it->ai_addrlen, ip, port))
			{
				out.push_back(ip);
			}
		}

		return out;
	}

	static void write_event_log(int severity, const char *msg)
	{
		switch (severity)
		{
		case _EVENT_LOG_DEBUG:
			XH_LOG_DEBUG(::xhnet::logname_base, msg);
			break;
		case _EVENT_LOG_MSG:
			XH_LOG_INFO(::xhnet::logname_base, msg);
			break;
		case _EVENT_LOG_WARN:
			XH_LOG_WARN(::xhnet::logname_base, msg);
			break;
		case _EVENT_LOG_ERR:
			XH_LOG_ERROR(::xhnet::logname_base, msg);
			break;
		default:
			break;
		}
	}

	static void write_event_fatal(int err)
	{
		XH_LOG_FATAL(::xhnet::logname_base, "libevent fatalerr:"<<err);
	}

	class CNetInit
	{
	public:
		CNetInit()
		{
#if defined _PLATFORM_WINDOWS_
			WSADATA WSAData;
			WSAStartup(0x101, &WSAData);
#elif defined _PLATFORM_LINUX_
			signal(SIGPIPE, SIG_IGN);
#endif
			event_set_log_callback(write_event_log);
			event_set_fatal_callback(write_event_fatal);

// 内存处理
//#ifdef EVENT_SET_MEM_FUNCTIONS_IMPLEMENTED
//			event_set_mem_functions
//#endif // EVENT_SET_MEM_FUNCTIONS_IMPLEMENTED

			// 2.1.1 版本后才有
			//event_enable_debug_logging();

			// 检查支持哪些后端方式
			//event_get_supported_methods
		}

		~CNetInit()
		{
			// 2.1.1
			//libevent_global_shutdown();
		}
	};

	static CNetInit sf_netinit;

	CIOServer::CIOServer()
		: m_evbase(event_base_new())
		, m_status(status_null)
		, m_binit(false)
		, m_curpost( 0 )
		, m_haspostdata( false )
	{
		//static CNetInit netinit;
		if ( !m_evbase )
		{
			XH_LOG_ERROR(::xhnet::logname_base, "Create EventBase Failed, Check OS config");
		}
		else
		{
			XH_LOG_DEBUG(::xhnet::logname_base, "Using LibEvent with backend method " << event_base_get_method(m_evbase));

			// 设置m_evbase的优先级范围0-10
			//event_base_priority_init(m_evbase, 10);
			//event_priority_set 设置event的优先级
			//event_base_get_npriorities // 2.1.1 
#ifdef _DEBUG
			int f = event_base_get_features(m_evbase);
			if ((f & EV_FEATURE_ET))
				XH_LOG_DEBUG(::xhnet::logname_base, "  Edge-triggered events are supported.");
			if ((f & EV_FEATURE_O1))
				XH_LOG_DEBUG(::xhnet::logname_base, "  O(1) event notification is supported.");
			if ((f & EV_FEATURE_FDS))
				XH_LOG_DEBUG(::xhnet::logname_base, "  All FD types are supported.");
#endif
		}
	}

	CIOServer::~CIOServer()
	{
		if (m_evbase)
		{
			event_base_free(m_evbase);
			m_evbase = 0;
		}
	}

	bool CIOServer::Init(void)
	{
		if (m_status != status_null) return false;
		if (!m_evbase) return false;

		m_posttimer = ITimer::Create();
		if (!m_posttimer) 
			return false;

		if (!m_posttimer->Init(this, this))
			return false;

		if (!m_posttimer->Async_Wait(1))
		{
			m_posttimer->Fini();
			return false;
		}

		m_status = status_common;
		m_binit = true;
		m_curpost = 0;

		return true;
	}

	void CIOServer::Fini(void)
	{
		if (!m_binit || m_status == status_null)
			return;

		Post(std::bind(&CIOServer::real_fini, this));
	}

	void CIOServer::Run(void)
	{
		if (!m_binit || m_status == status_null)
			return;

		real_post(false);

		while (m_status==status_common)
		{
			event_base_loop(m_evbase, EVLOOP_NONBLOCK);
		}

		//event_base_dispatch(m_evbase);
	}

	// print event dump
	// event_base_dump_events()

	// 获取当前时间
	// event_base_gettimeofday_cached()

	bool CIOServer::IsFinshed(void)
	{
		return (m_status == status_null);
	}

	void CIOServer::Post(postio io)
	{
		std::lock_guard<std::mutex> guard(m_postmutex);
		m_postdata[m_curpost].push(io);
		m_haspostdata = true;
	}

	void CIOServer::On_Timer(unsigned int timerid, int errid)
	{
		real_post(errid == timer_end);
	}

	void CIOServer::Reset_Event(::event* ev, evutil_socket_t sock, short flag, event_callback_fn cb, void* p)
	{
		int err = event_assign(ev, m_evbase, sock, flag, cb, p);
		if ( err!=0 )
		{
			int test = 9;
		}
	}

	// event_add
	void CIOServer::Add_Event(::event* ev, const struct timeval *timeout)
	{
		int err = event_add(ev, timeout);
		if ( err!=0 )
		{
			int test=10;
		}
	}

	// event_del
	void CIOServer::Del_Event(::event* ev)
	{
		int err = event_del(ev);
		if ( err!=0 )
		{
			int test = 11;
		}
	}

	//
	void CIOServer::Add_TcpSocket(ITcpSocket* socket)
	{
		socket->Retain();
		m_tcpsockets[socket->GetSocketID()] = socket;
	}
	//
	void CIOServer::Del_TcpSocket(ITcpSocket* socket)
	{
		auto it = m_tcpsockets.find(socket->GetSocketID());
		if (it != m_tcpsockets.end())
		{
			m_tcpsockets.erase(it);
			socket->Release();
		}
	}

	void CIOServer::Add_UdpSocket(IUdpSocket* socket)
	{
		socket->Retain();
		m_udpsockets[socket->GetSocketID()] = socket;
	}

	void CIOServer::Del_UdpSocket(IUdpSocket* socket)
	{
		auto it = m_udpsockets.find(socket->GetSocketID());
		if (it != m_udpsockets.end())
		{
			m_udpsockets.erase(it);
			socket->Release();
		}
	}

	void CIOServer::Add_Timer(ITimer* timer)
	{
		if ( timer==m_posttimer )
			return;

		timer->Retain();
		m_timers[timer->GetTimerID()] = timer;
	}

	void CIOServer::Del_Timer(ITimer* timer)
	{
		if ( timer==m_posttimer )
		{
			XH_SAFE_RELEASE(m_posttimer);
			return;
		}

		auto it = m_timers.find(timer->GetTimerID());
		if (it != m_timers.end())
		{
			m_timers.erase(it);
			timer->Release();
		}
	}

	void CIOServer::real_fini()
	{
		if (!m_binit || m_status == status_null)
			return;

		m_binit = false;
	}

	void CIOServer::real_post(bool bfinished)
	{
		if (bfinished)
		{
			m_status = status_null;
			return;
		}

		bool bret = (m_binit
			|| m_tcpsockets.size() > 0
			|| m_udpsockets.size() > 0
			|| m_timers.size() >0
			|| m_haspostdata);

		if (!bret)
		{
			m_posttimer->Fini();
			return;
		}

		if (!m_binit)
		{
			for (auto it = m_tcpsockets.begin(); it != m_tcpsockets.end(); ++it)
			{
				it->second->Fini();
			}

			for (auto it = m_udpsockets.begin(); it != m_udpsockets.end(); ++it)
			{
				it->second->Fini();
			}

			for (auto it = m_timers.begin(); it != m_timers.end(); ++it)
			{
				it->second->Fini();
			}
		}

		std::queue<postio>* pcur = 0;
		{
			std::lock_guard<std::mutex> guard(m_postmutex);
			pcur = &(m_postdata[m_curpost]);
			m_curpost = (++m_curpost) % 2;
			m_haspostdata = false;
		}

		while (!pcur->empty())
		{
			pcur->front()();
			pcur->pop();
		}
	}
};

