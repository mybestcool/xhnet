
#include "impl_tcpsocket.h"

#include "impl_ioserver.h"

#include <xhguard.h>
#include <xhlog.h>

namespace xhnet
{
	static const unsigned int sf_connect_timeout = 5;	// 单位秒

	ITcpSocket* ITcpSocket::Create(void)
	{
		return new CTcpSocket();
	}

	static unsigned int gen_tcp_socketid()
	{
		static std::atomic<unsigned int> gen_socketid = 0;
		return ++gen_socketid;
	}

	CTcpSocket::CTcpSocket()
		: m_type(type_null)
		, m_binit(false)
		, m_status(status_null)
		, m_id(gen_tcp_socketid())
		, m_socket( INVALID_SOCKET )
		, m_io( 0 )
		, m_localport( 0 )
		, m_peerport( 0 )
		, m_lcb(0)
		, m_bassign( 0 )
	{
#ifdef _DEBUG
		static unsigned int tcpsocket_createcnt = 0;
		++tcpsocket_createcnt;
		XH_LOG_INFO(logname_base, "tcpsocket create count:" << tcpsocket_createcnt);
#endif
	}

	CTcpSocket::~CTcpSocket()
	{
#ifdef _DEBUG
		static unsigned int tcpsocket_destorycnt = 0;
		++tcpsocket_destorycnt;
		XH_LOG_INFO(logname_base, "tcpsocket destory count:" << tcpsocket_destorycnt);
#endif
	}

	bool CTcpSocket::Init_Listener(IIOServer* io, ICBTcpListener* cb)
	{
		if (!io || !cb) return false;
		if (m_io || m_lcb) return false;
		if (m_status != status_null) return false;

		m_io = dynamic_cast<CIOServer*>(io);
		if (!m_io) return false;

		cb->Retain();
		m_lcb = cb;
		m_type = type_listener;
		m_binit = true;

		return true;
	}

	bool CTcpSocket::Init_Accepter(IIOServer* io, ICBTcpAccepter* cb)
	{
		if (!io || !cb) return false;
		if (m_io || m_acb) return false;
		if (m_status != status_null) return false;

		m_io = dynamic_cast<CIOServer*>(io);
		if (!m_io) return false;

		cb->Retain();
		m_acb = cb;
		m_type = type_accepter;
		m_binit = true;

		return true;
	}

	bool CTcpSocket::Init_Connecter(IIOServer* io, ICBTcpConneter* cb)
	{
		if (!io || !cb) return false;
		if (m_io || m_ccb) return false;
		if (m_status != status_null) return false;

		m_io = dynamic_cast<CIOServer*>(io);
		if (!m_io) return false;

		cb->Retain();
		m_ccb = cb;
		m_type = type_connecter;
		m_binit = true;

		return true;
	}

	void CTcpSocket::Fini(void)
	{
		if (!m_binit) return;

		if ( m_io )
		{
			Retain();
			m_io->Post(std::bind(&CTcpSocket::real_fini, this));
		}
	}

	bool CTcpSocket::Listen(const std::string& ip, unsigned int port)
	{
		if (m_status != status_null) return false;
		if (!m_binit) return false;
		if (m_type != type_listener) return false;
		//if (!m_io || !m_lcb) return false;

		Retain();
		m_io->Post(std::bind(&CTcpSocket::real_listen, this, ip, port));

		return true;
	}

	bool CTcpSocket::Connect(const std::string& ip, unsigned int port)
	{
		if (m_status != status_null) return false;
		if (!m_binit) return false;
		if (m_type != type_connecter) return false;
		//if (!m_io || !m_ccb) return false;

		Retain();
		m_io->Post(std::bind(&CTcpSocket::real_connect, this, ip, port));

		return true;
	}

	bool CTcpSocket::Async_Send(CIOBuffer* buffer)
	{
		if (!buffer) return false;
		if (m_status != status_common) return false;
		if (m_type != type_accepter && m_type != type_connecter) return false;

		buffer->Retain();
		if (m_wait_send.size() == 0)
		{
			m_io->Add_Event(&m_ev_write, 0);
		}
		m_wait_send.push(buffer);

		return true;
	}

	bool CTcpSocket::Async_Recv(CIOBuffer* buffer)
	{
		if (!buffer) return false;
		if (m_status != status_common) return false;
		if (m_type != type_accepter && m_type != type_connecter) return false;

		buffer->SetTag(0);
		buffer->Retain();
		if (m_wait_recv.size()==0)
		{
			m_io->Add_Event(&m_ev_read, 0);
		}
		m_wait_recv.push(buffer);

		return true;
	}

	bool CTcpSocket::Async_RecvSome(CIOBuffer* buffer)
	{
		if (!buffer) return false;
		if (m_status != status_common) return false;
		if (m_type != type_accepter && m_type != type_connecter) return false;

		buffer->SetTag(1);
		buffer->Retain();
		if (m_wait_recv.size() == 0)
		{
			m_io->Add_Event(&m_ev_read, 0);
		}
		m_wait_recv.push(buffer);

		return true;
	}

	std::string CTcpSocket::GetLocalIP(void)
	{
		return m_localip;
	}

	unsigned int CTcpSocket::GetLocalPort(void)
	{
		return m_localport;
	}

	std::string CTcpSocket::GetPeerIP(void)
	{
		return m_peerip;
	}

	unsigned int CTcpSocket::GetPeerPort(void)
	{
		return m_peerport;
	}

	unsigned int CTcpSocket::GetSocketID(void)
	{
		return m_id;
	}




	void CTcpSocket::on_accept_callback(evutil_socket_t sock, short flag, void* p)
	{
		CTcpSocket* psocket = (CTcpSocket*)(p);
		if (psocket && (flag&EV_READ))
		{
			psocket->on_inter_listen_accept(sock);
		}
	}

	void CTcpSocket::on_recv_callback(evutil_socket_t sock, short flag, void* p)
	{
		CTcpSocket* psocket = (CTcpSocket*)(p);
		if (psocket)
		{
			psocket->on_inter_recv(flag);
		}
	}

	void CTcpSocket::on_send_callback(evutil_socket_t sock, short flag, void* p)
	{
		CTcpSocket* psocket = (CTcpSocket*)(p);
		if (psocket)
		{
			psocket->on_inter_send(flag);
		}
	}


	void CTcpSocket::real_listen(std::string ip, unsigned int port)
	{
		XH_GUARD([&]{Release(); });

		if (!m_binit) return;

		m_localip = ip;
		m_localport = port;
		m_status = status_connect;

		socketaddr bindaddr;
		int addrlen = sizeof(bindaddr);
		if (!ipport2sockaddr(m_localip, m_localport, &bindaddr, addrlen))
		{
			XH_LOG_ERROR(logname_base, "tcpsocket listen failed iperr, ip:" << ip << ", port:" << port);
			on_inter_close(tcp_listenfail_iperr, true);
			
			return;
		}

		struct sockaddr* tempsockaddr = (struct sockaddr*)(&bindaddr);
		evutil_socket_t listener;
		if ((listener = ::socket(tempsockaddr->sa_family, SOCK_STREAM, 0)) == INVALID_SOCKET)
		{
			XH_LOG_ERROR(logname_base, "tcpsocket listen failed ::socket err, ip:" << ip << ", port:" << port);
			on_inter_close(tcp_listenfail_createerr, true);
			
			return;
		}

		CScopeGuard guard_close([&]{closetcpsocket(listener); });

		if (::bind(listener, (struct sockaddr *) &bindaddr, addrlen) == SOCKET_ERROR)
		{
			XH_LOG_ERROR(logname_base, "tcpsocket listen failed ::bind err, ip:" << ip << ", port:" << port);
			on_inter_close(tcp_listenfail_binderr, true);
			
			return;
		}

		if (::listen(listener, SOMAXCONN) == SOCKET_ERROR)
		{
			XH_LOG_ERROR(logname_base, "tcpsocket listen failed ::listen err, ip:" << ip << ", port:" << port);
			on_inter_close(tcp_listenfail_listenerr, true);
			
			return;
		}

		setnonblocking(listener);
		setnodelay(listener);
		setreuseport(listener);
		//setlingerclose(listener);
		// 默认接收长度
		//setrecvbuffer(listener, recvsize);
		//setsendbuffer(listener, sendsize);

		guard_close.Dismiss();

		m_status = status_common;
		m_socket = listener;
		m_io->Reset_Event(&m_ev_read, m_socket, EV_READ|EV_PERSIST, &(CTcpSocket::on_accept_callback), this);
		m_bassign |= 0x01;
		m_io->Add_TcpSocket(this);
		m_io->Add_Event(&m_ev_read, 0);

		m_lcb->On_Listen(GetSocketID(), tcp_ok);

		XH_LOG_INFO(logname_base, "tcpsocket listen succeed, ip:" << ip << ", port:" << port);
	}

	void CTcpSocket::real_connect(std::string ip, unsigned int port)
	{
		XH_GUARD([&]{Release(); });

		if (!m_binit) return;

		m_peerip = ip;
		m_peerport = port;
		m_status = status_connect;

		socketaddr connectaddr;
		int addrlen = sizeof(connectaddr);
		if (!ipport2sockaddr(m_peerip, m_peerport, &connectaddr, addrlen))
		{
			XH_LOG_ERROR(logname_base, "tcpsocket connect failed errip, ip:" << ip << ", port:" << port);
			on_inter_close(tcp_connectfail_iperr, true);
			
			return;
		}

		struct sockaddr* tempsockaddr = (struct sockaddr*)(&connectaddr);
		evutil_socket_t connecter;
		if ((connecter = ::socket(tempsockaddr->sa_family, SOCK_STREAM, 0)) == INVALID_SOCKET)
		{
			XH_LOG_ERROR(logname_base, "tcpsocket connect failed ::socket err, ip:" << ip << ", port:" << port);
			on_inter_close(tcp_connectfail_createerr, true);
			
			return;
		}

		setnonblocking(connecter);
		setnodelay(connecter);
		//setlingerclose(connecter);

		if (::connect(connecter, (struct sockaddr *) &connectaddr, addrlen) == SOCKET_ERROR)
		{
			if ( !isnoblockerr() )
			{
				closetcpsocket(connecter);
				XH_LOG_ERROR(logname_base, "tcpsocket connect failed ::connect err, ip:" << ip << ", port:" << port);
				on_inter_close(tcp_connectfail_connecterr, true);
				
				return;
			}
		}

		m_socket = connecter;

		socketaddr localaddr;
		int locallen = sizeof(localaddr);
		if (0 == getsockname(m_socket, (struct sockaddr*)&localaddr, &locallen))
		{
			sockaddr2ipport(&localaddr, locallen, m_localip, m_localport);
		}

		m_io->Reset_Event(&m_ev_read, m_socket, EV_READ | EV_PERSIST, &CTcpSocket::on_recv_callback, this);
		m_io->Reset_Event(&m_ev_write, m_socket, EV_WRITE | EV_PERSIST, &CTcpSocket::on_send_callback, this);
		m_bassign |= 0x01;
		m_bassign |= 0x02;
		m_io->Add_TcpSocket(this);

		// 可以设置一个超时时间，
		struct timeval writeval;
		writeval.tv_sec = sf_connect_timeout;
		writeval.tv_usec = 0;
		m_io->Add_Event(&m_ev_write, &writeval);

		XH_LOG_INFO(logname_base, "tcpsocket connect succeed, ip:" << ip << ", port:" << port);
	}

	void CTcpSocket::real_fini()
	{
		XH_GUARD([&]{Release(); });

		if (!m_binit) 
			return;

		//XH_GUARD([&]{
		//	switch (m_type)
		//	{
		//	case type_listener:
		//		if (m_lcb)
		//		{
		//			m_lcb->Release();
		//			m_lcb = 0;
		//		}
		//		break;
		//	case type_accepter:
		//		if (m_acb)
		//		{
		//			m_acb->Release();
		//			m_acb = 0;
		//		}
		//		break;
		//	case type_connecter:
		//		if (m_ccb)
		//		{
		//			m_ccb->Release();
		//			m_ccb = 0;
		//		}
		//		break;;
		//	default:
		//		break;
		//	}

		//	m_status = status_null;
		//	m_type = type_null;
		//});
		
		m_binit = false;

		on_inter_close(tcp_close_bylocal, false);
		XH_LOG_ERROR(logname_base, "tcpsocket fini");

		{
			switch (m_type)
			{
			case type_listener:
				if (m_lcb)
				{
					m_lcb->Release();
					m_lcb = 0;
				}
				break;
			case type_accepter:
				if (m_acb)
				{
					m_acb->Release();
					m_acb = 0;
				}
				break;
			case type_connecter:
				if (m_ccb)
				{
					m_ccb->Release();
					m_ccb = 0;
				}
				break;;
			default:
				break;
			}

			m_status = status_null;
			m_type = type_null;
		}
	}

	void CTcpSocket::on_inter_listen_accept(evutil_socket_t listenfd)
	{
		socketaddr acceptaddr;
		int addrlen = sizeof(acceptaddr);

		evutil_socket_t newfd = ::accept(listenfd, (struct sockaddr*)&acceptaddr, &addrlen);
		if (newfd == INVALID_SOCKET)
		{
			XH_LOG_ERROR(logname_base, "tcpsocket listener ::accept failed, ip:" << m_localip << ", port:" << m_localport);
			return;
		}

		ITcpSocket* newsocket = ITcpSocket::Create();
		if (!newsocket)
		{
			closetcpsocket(newfd);
			return;
		}

		m_lcb->On_Accept(GetSocketID(), newsocket);

		CTcpSocket* tcpsocket = (CTcpSocket*)newsocket;
		if (tcpsocket)
			tcpsocket->on_inter_accept_accept(newfd, &acceptaddr, addrlen);
		else
		{
			XH_SAFE_RELEASE(newsocket);
			XH_LOG_ERROR(logname_base, "tcpsocket listener cast<ITcpSocket->CTcpSocket> failed, ip:" << m_localip << ", port:" << m_localport);
		}
	}

	void CTcpSocket::on_inter_accept_accept(evutil_socket_t fd, socketaddr* peeraddr, int addrlen)
	{
		CScopeGuard releaseguard(
			[&]{
			XH_LOG_ERROR(logname_base, "tcpsocket accept failed because of ICBTcpListener::On_Accept, user has not ITcpSocket::Init_Accepter");
			Release();
		});

		if (m_status != status_null) return;
		if (!m_binit) return;
		if (m_type != type_accepter) return;

		releaseguard.Dismiss();
		m_status = status_connect;
		m_socket = fd;

		setnonblocking(m_socket);
		setnodelay(m_socket);
		//setlingerclose(m_socket);
		
		if ( peeraddr )
		{
			sockaddr2ipport(peeraddr, addrlen, m_peerip, m_peerport);
		}

		socketaddr localaddr;
		int locallen = sizeof(localaddr);
		if (0 == getsockname(m_socket, (struct sockaddr*)&localaddr, &locallen))
		{
			sockaddr2ipport(&localaddr, locallen, m_localip, m_localport);
		}

		m_io->Reset_Event(&m_ev_read, m_socket, EV_READ | EV_PERSIST, &CTcpSocket::on_recv_callback, this);
		m_io->Reset_Event(&m_ev_write, m_socket, EV_WRITE | EV_PERSIST, &CTcpSocket::on_send_callback, this);
		m_bassign |= 0x01;
		m_bassign |= 0x02;
		m_io->Add_TcpSocket(this);

		// 可以设置一个超时时间，
		struct timeval writeval;
		writeval.tv_sec = sf_connect_timeout;
		writeval.tv_usec = 0;
		m_io->Add_Event(&m_ev_write, &writeval);

		XH_LOG_INFO(logname_base, "tcpsocket accept succeed, peerip:" << m_peerip << ", peerport:" << m_peerport);
	}

	void CTcpSocket::on_inter_recv(short flag)
	{
		if (m_wait_recv.empty())
		{
			m_io->Del_Event(&m_ev_read);
			return;
		}

		CIOBuffer* recvbuff = m_wait_recv.front();

		int recvlen = ::recv(m_socket, recvbuff->GetCurWrite(), recvbuff->AvailWrite(), 0);
		if (recvlen == 0)
		{
			XH_LOG_ERROR(logname_base, "tcpsocket ::recv failed, closed by peer");
			on_inter_close(tcp_recvfail_closedbypeer, false);
			return;
		}
		else if (recvlen < 0)
		{
			if (isnoblockerr())
			{
				;
			}
			else
			{
				XH_LOG_ERROR(logname_base, "tcpsocket ::recv failed, recv -1");
				on_inter_close(tcp_recvfail_recverr, false);
			}
			return;
		}

		bool bneedcb = false;
		recvbuff->SeekWrite(recvlen);
		// 如果是readsome
		if ( recvbuff->GetTag()==1 )
		{
			bneedcb = true;
		}
		// 如果是读固定长度
		else
		{
			// 如果读全了
			if (recvbuff->AvailWrite()==0)
			{
				bneedcb = true;
			}
		}

		if ( !bneedcb )
		{
			return;
		}

		switch (m_type)
		{
		case type_accepter:
			if (m_acb)
			{
				m_acb->On_Recv(GetSocketID(), tcp_ok, recvbuff);
			}
			break;
		case type_connecter:
			if (m_ccb)
			{
				m_ccb->On_Recv(GetSocketID(), tcp_ok, recvbuff);
			}
			break;
		default:
			break;
		}

		m_wait_recv.pop();
		recvbuff->Release();

		// 没有可以接收的缓存，则删除事件
		if (m_wait_recv.empty())
		{
			m_io->Del_Event(&m_ev_read);
		}
	}

	void CTcpSocket::on_inter_send(short flag)
	{
		if (m_status == status_connect)
		{
			if ((flag&EV_WRITE) == EV_WRITE)
			{
				m_status = status_common;

				switch (m_type)
				{
				case type_accepter:
					if (m_acb)
					{
						m_acb->On_Connect(GetSocketID(), tcp_ok);
					}
					break;
				case type_connecter:
					if (m_ccb)
					{
						m_ccb->On_Connect(GetSocketID(), tcp_ok);
					}
					break;
				default:
					break;
				}
			}
			else
			{
				XH_LOG_ERROR(logname_base, "tcpsocket connect timeout");
				on_inter_close(tcp_connectfail_timeout, true);

				return;
			}
		}
		else
		{
			while (!m_wait_send.empty())
			{
				CIOBuffer* buff = m_wait_send.front();

				bool bneedbreak = false;
				int sendlen = ::send(m_socket, buff->GetCurRead(), buff->AvailRead(), 0);

				if ( sendlen>0 )
				{
					buff->SeekRead(sendlen);
				}
				else if (sendlen<0)
				{
					if (isnoblockerr())
					{
						bneedbreak = true;
					}
					else
					{
						XH_LOG_ERROR(logname_base, "tcpsocket ::send failed send -1");
						on_inter_close(tcp_sendfail_senderr, false);
						return;
					}
				}
				else
				{
					bneedbreak = true;
				}

				//已经写完了，则回调通知
				if (buff->AvailRead()==0)
				{
					switch (m_type)
					{
					case type_accepter:
						if (m_acb)
						{
							m_acb->On_Send(GetSocketID(), tcp_ok, buff);
						}
						break;
					case type_connecter:
						if (m_ccb)
						{
							m_ccb->On_Send(GetSocketID(), tcp_ok, buff);
						}
						break;
					default:
						break;
					}

					m_wait_send.pop();
					buff->Release();
				}

				if (bneedbreak) break;
			}
		}

		if (m_wait_send.empty())
		{
			m_io->Del_Event(&m_ev_write);
		}
	}

	void CTcpSocket::on_inter_close(int errid, bool bconnecting)
	{
		if (m_status != status_connect && m_status != status_common) return;

		XH_GUARD([&]{
			if (m_io)
			{
				if ((m_bassign & 0x01) == 0x01)
					m_io->Del_Event(&m_ev_read);
				if ((m_bassign & 0x02) == 0x02)
					m_io->Del_Event(&m_ev_write);

				m_bassign = 0;
				m_io->Del_TcpSocket(this);
			}
		});

		m_status = status_null;

		closetcpsocket(m_socket);
		m_socket = INVALID_SOCKET;

		switch (m_type)
		{
		case type_listener:
			if (m_lcb)
			{
				if (bconnecting)
				{
					m_lcb->On_Listen(GetSocketID(), errid);
				}

				m_lcb->On_Close(GetSocketID(), errid);
			}
			break;
		case type_accepter:
			if (m_acb)
			{
				if (bconnecting)
				{
					m_acb->On_Connect(GetSocketID(), errid);
				}

				while (!m_wait_send.empty())
				{
					CIOBuffer* tmpbuf = m_wait_send.front();
					XH_GUARD([&]{tmpbuf->Release(); });

					m_acb->On_Send(GetSocketID(), errid, tmpbuf);

					m_wait_send.pop();
				}

				while (!m_wait_recv.empty())
				{
					CIOBuffer* tmpbuf = m_wait_recv.front();
					XH_GUARD([&]{tmpbuf->Release(); });

					m_acb->On_Recv(GetSocketID(), errid, tmpbuf);

					m_wait_recv.pop();
				}

				m_acb->On_Close(GetSocketID(), errid);

				// 异常调度行为检查
				while (!m_wait_send.empty())
				{
					CIOBuffer* tmpbuf = m_wait_send.front();
					XH_GUARD([&]{tmpbuf->Release(); });

					m_wait_send.pop();
				}
				while (!m_wait_recv.empty())
				{
					CIOBuffer* tmpbuf = m_wait_recv.front();
					XH_GUARD([&]{tmpbuf->Release(); });

					m_wait_recv.pop();
				}
			}
			break;
		case type_connecter:
			if (m_ccb)
			{
				if (bconnecting)
				{
					m_ccb->On_Connect(GetSocketID(), errid);
				}

				while (!m_wait_send.empty())
				{
					CIOBuffer* tmpbuf = m_wait_send.front();
					XH_GUARD([&]{tmpbuf->Release(); });

					m_ccb->On_Send(GetSocketID(), errid, tmpbuf);

					m_wait_send.pop();
				}

				while (!m_wait_recv.empty())
				{
					CIOBuffer* tmpbuf = m_wait_recv.front();
					XH_GUARD([&]{tmpbuf->Release(); });

					m_ccb->On_Recv(GetSocketID(), errid, tmpbuf);

					m_wait_recv.pop();
				}

				m_ccb->On_Close(GetSocketID(), errid);

				// 异常调度行为检查
				while (!m_wait_send.empty())
				{
					CIOBuffer* tmpbuf = m_wait_send.front();
					XH_GUARD([&]{tmpbuf->Release(); });

					m_wait_send.pop();
				}
				while (!m_wait_recv.empty())
				{
					CIOBuffer* tmpbuf = m_wait_recv.front();
					XH_GUARD([&]{tmpbuf->Release(); });

					m_wait_recv.pop();
				}
			}
			break;
		default:
			break;
		}

		XH_LOG_ERROR(logname_base, "tcpsocket closed by errid:"<<errid);
	}
};


