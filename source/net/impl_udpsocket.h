
#pragma once

#include "impl_nethead.h"

namespace xhnet
{
	class CIOServer;

	class CUdpSocket : public IUdpSocket, public CInheritOPool<CUdpSocket, std::mutex>
	{
	public:
		enum status
		{
			status_null = 0,
			status_init,
			status_connect,
			status_common,
		};

		struct UdpBuff
		{
			CIOBuffer*	buff;
			unsigned int port;
			std::string ip;

			UdpBuff()
				: buff(0)
				, port(0)
				, ip("0.0.0.0")
			{
			}

			UdpBuff(CIOBuffer* b)
				: buff(b)
				, port(0)
				, ip("0.0.0.0")
			{

			}

			UdpBuff(const std::string& i, unsigned int p, CIOBuffer* b)
				: buff(b)
				, port(p)
				, ip(i)
			{
			}
		};

	public:
		CUdpSocket();
		virtual ~CUdpSocket();

		virtual bool Init(IIOServer* io, ICBUdpSocket* cb);
		virtual void Fini(void);

		virtual bool Listen(const std::string& ip, unsigned int port);

		virtual bool Async_SendTo(const std::string& ip, unsigned int port, CIOBuffer* buffer);
		virtual bool Async_RecvSomeFrom(CIOBuffer* buffer);

		virtual std::string GetLocalIP(void);
		virtual unsigned int GetLocalPort(void);

		virtual unsigned int GetSocketID(void);

	public:
		static void on_recv_callback(evutil_socket_t sock, short flag, void* p);
		static void on_send_callback(evutil_socket_t sock, short flag, void* p);

	protected:
		void real_listen(std::string ip, unsigned int port);
		void real_close(int errid);

		void on_inter_recv();
		void on_inter_send();

	protected:
		unsigned char	m_status;
		unsigned int	m_id;
		evutil_socket_t	m_socket;

		CIOServer*		m_io;

		std::string		m_localip;
		unsigned int	m_localport;

		ICBUdpSocket*	m_cb;

		::event			m_ev_read;
		::event			m_ev_write;

		std::queue<UdpBuff>	m_wait_send;
		std::queue<UdpBuff>	m_wait_recv;

	private:
		CUdpSocket(const CUdpSocket&) = delete;
		CUdpSocket& operator=(const CUdpSocket&) = delete;
	};
};
