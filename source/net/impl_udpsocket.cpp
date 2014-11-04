
#include "impl_udpsocket.h"

#include "impl_ioserver.h"

#include <xhguard.h>

namespace xhnet
{
	IUdpSocket* IUdpSocket::Create(void)
	{
		return new CUdpSocket();
	}

	static unsigned int gen_udp_socketid()
	{
		static std::atomic<unsigned int> gen_socketid( 0 );
		return ++gen_socketid;
	}

	void CUdpSocket::on_recv_callback(evutil_socket_t sock, short flag, void* p)
	{
		CUdpSocket* psocket = (CUdpSocket*)(p);
		if (psocket && (flag&EV_READ))
		{
			psocket->on_inter_recv();
		}
	}

	void CUdpSocket::on_send_callback(evutil_socket_t sock, short flag, void* p)
	{
		CUdpSocket* psocket = (CUdpSocket*)(p);
		if (psocket && (flag&EV_WRITE))
		{
			psocket->on_inter_send();
		}
	}

	CUdpSocket::CUdpSocket()
		: m_status(status_null)
		, m_id(gen_udp_socketid())
		, m_socket(INVALID_SOCKET)
		, m_io(0)
		, m_localport(0)
		, m_cb(0)
	{

	}

	CUdpSocket::~CUdpSocket()
	{

	}

	bool CUdpSocket::Init(IIOServer* io, ICBUdpSocket* cb)
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

	void CUdpSocket::Fini(void)
	{
		if (m_status == status_null) return;

		if (m_io)
		{
			Retain();
			m_io->Post(std::bind(&CUdpSocket::real_close, this, udp_close));
		}
	}

	bool CUdpSocket::Listen(const std::string& ip, unsigned int port)
	{
		if (m_status != status_init) return false;
		if (!m_io || !m_cb) return false;

		Retain();
		m_io->Post(std::bind(&CUdpSocket::real_listen, this, ip, port));

		return true;
	}

	bool CUdpSocket::Async_SendTo(const std::string& ip, unsigned int port, CIOBuffer* buffer)
	{
		if (!buffer) return false;
		if (m_status != status_common) return false;

		buffer->Retain();
		if (m_wait_send.size() == 0)
		{
			m_io->Add_Event(&m_ev_write, 0);
		}
		m_wait_send.push(UdpBuff(ip, port, buffer));

		return true;
	}

	bool CUdpSocket::Async_RecvSomeFrom(CIOBuffer* buffer)
	{
		if (!buffer) return false;
		if (m_status != status_common) return false;

		buffer->Retain();
		if (m_wait_recv.size() == 0)
		{
			m_io->Add_Event(&m_ev_read, 0);
		}
		m_wait_recv.push(UdpBuff(buffer));

		return true;
	}

	std::string CUdpSocket::GetLocalIP(void)
	{
		return m_localip;
	}

	unsigned int CUdpSocket::GetLocalPort(void)
	{
		return m_localport;
	}

	unsigned int CUdpSocket::GetSocketID(void)
	{
		return m_id;
	}

	void CUdpSocket::real_listen(std::string ip, unsigned int port)
	{
		XH_GUARD([&]{Release(); });

		if (m_status != status_init) return;

		m_localip = ip;
		m_localport = port;
		m_status = status_connect;

		socketaddr bindaddr;
		ev_socklen_t addrlen = sizeof(bindaddr);
		if (!ipport2sockaddr(m_localip, m_localport, &bindaddr, addrlen))
		{
			m_status = status_init;
			m_cb->On_Listen(GetSocketID(), udp_listenfail_iperr);
			return;
		}

		struct sockaddr* tempsockaddr = (struct sockaddr*)(&bindaddr);
		evutil_socket_t listener;
		if ((listener = ::socket(tempsockaddr->sa_family, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			m_status = status_init;
			m_cb->On_Listen(GetSocketID(), udp_listenfail_createerr);
			return;
		}

		CScopeGuard guard_close([&]{closetcpsocket(listener); });

		if (::bind(listener, (struct sockaddr *) &bindaddr, addrlen) == SOCKET_ERROR)
		{
			m_status = status_init;
			m_cb->On_Listen(GetSocketID(), udp_listenfail_binderr);
			return;
		}

		setnonblocking(listener);
		setnodelay(listener);
		setreuseport(listener);
		// 默认接收长度
		//setrecvbuffer(listener, recvsize);
		//setsendbuffer(listener, sendsize);

		guard_close.Dismiss();

		m_status = status_common;
		m_socket = listener;
		m_io->Reset_Event(&m_ev_read, m_socket, EV_READ | EV_PERSIST, &CTcpSocket::on_recv_callback, this);
		m_io->Reset_Event(&m_ev_write, m_socket, EV_WRITE | EV_PERSIST, &CTcpSocket::on_send_callback, this);
		m_io->Add_UdpSocket(this);

		m_cb->On_Listen(GetSocketID(), udp_ok);
	}

	void CUdpSocket::real_close(int errid)
	{
		XH_GUARD([&]{Release(); });

		if (m_status == status_null) return;

		XH_GUARD([&]{
			m_status = status_null;

			closetcpsocket(m_socket);
			m_socket = INVALID_SOCKET;

			if (m_cb)
			{
				m_cb->Release();
				m_cb = 0;
			}
				
			if (m_io)
			{
				m_io->Del_Event(&m_ev_read);
				m_io->Del_Event(&m_ev_write);

				m_io->Del_UdpSocket(this);
			}
		});

		while (!m_wait_send.empty())
		{
			UdpBuff tmpbuf = m_wait_send.front();
			XH_GUARD([&]{if (tmpbuf.buff) tmpbuf.buff->Release(); });

			m_cb->On_Send(GetSocketID(), errid, tmpbuf.ip, tmpbuf.port, tmpbuf.buff);

			m_wait_send.pop();
		}

		while (!m_wait_recv.empty())
		{
			UdpBuff tmpbuf = m_wait_recv.front();
			XH_GUARD([&]{if (tmpbuf.buff) tmpbuf.buff->Release(); });

			m_cb->On_Recv(GetSocketID(), errid, tmpbuf.ip, tmpbuf.port, tmpbuf.buff);

			m_wait_recv.pop();
		}
	}

	void CUdpSocket::on_inter_recv()
	{
		if (m_wait_recv.empty())
		{
			m_io->Del_Event(&m_ev_read);
			return;
		}

		UdpBuff buff = m_wait_recv.front();
		CIOBuffer* recvbuff = buff.buff;

		socketaddr addr;
		ev_socklen_t addrlen = sizeof(addr);

		std::string fromip;
		unsigned int fromport;

		int recvlen = ::recvfrom(m_socket, recvbuff->GetCurWrite(), recvbuff->AvailWrite(), 0, (struct sockaddr*)&addr, &addrlen);
		int errid = udp_ok;
		if (recvlen == 0)
		{
			errid = udp_recvfail_closedbypeer;
		}
		else if (recvlen < 0)
		{
			errid = udp_recvfail_recverr;
		}
		else
		{
			recvbuff->SeekWrite(recvlen);

			sockaddr2ipport(&addr, addrlen, fromip, fromport);
		}

		m_cb->On_Recv(GetSocketID(), errid, fromip, fromport, recvbuff);

		m_wait_recv.pop();
		recvbuff->Release();

		// 没有可以接收的缓存，则删除事件
		if (m_wait_recv.empty())
		{
			m_io->Del_Event(&m_ev_read);
		}
	}

	void CUdpSocket::on_inter_send()
	{
		if (!m_wait_send.empty())
		{
			UdpBuff buff = m_wait_send.front();

			CIOBuffer* sendbuff = buff.buff;
			int errid = udp_ok;

			socketaddr sendaddr;
			ev_socklen_t addrlen = sizeof(sendaddr);
			if (!ipport2sockaddr(m_localip, m_localport, &sendaddr, addrlen))
			{
				errid = udp_sendfail_iperr;
			}
			else
			{
				int sendlen = ::sendto(m_socket, sendbuff->GetCurRead(), sendbuff->AvailRead(), 0, (const struct sockaddr*)(&sendaddr), addrlen);
				
				if ( sendlen>0 )
				{
					sendbuff->SeekRead(sendlen);
				}
				else
				{
					errid = udp_sendfail_senderr;
				}
			}

			m_cb->On_Recv(GetSocketID(), errid, buff.ip, buff.port, sendbuff);
			
			m_wait_send.pop();
			sendbuff->Release();
		}

		if (m_wait_send.empty())
		{
			m_io->Del_Event(&m_ev_write);
		}
	}

};
