
#pragma once

#include "qyfhhead.h"
#include "msgstream.h"


namespace xhnet_qyfh
{
	unsigned int sc_server_socket_count = 0;

	template<class User, class CMessageHead>
	class CTcpSocketServer : public ICBTcpAccepter, public ICBTimer
	{
	public:
		enum SSStatus
		{
			SSS_CLOSED = 0,
			SSS_ESTABLISHING,
			SSS_ESTABLISHED,
		};

		typedef CTcpSocketServer<User, CMessageHead>	ImplSocket;
		typedef COPool<CIOBuffer, mutex>				MessageObjPool;
		typedef CMPool<mutex>							MessageMemPool;

		// socket连接上来的通知, socketid, listensocketid
		typedef std::function< void(User, unsigned int, unsigned int) >					tcp_connect_callback;
		// socket关闭通知，
		typedef std::function< void(User, unsigned int, unsigned int) >					tcp_close_callback;
		// socket接收消息的通知 socketid,msglen,msgid,msg
		typedef std::function< void(User, unsigned int, unsigned int, const CMessageHead&, CIOBuffer*) >	tcp_message_callback;
		// socket 定时操作，比如心跳等
		typedef std::function< void(User, unsigned int, unsigned int) >					tcp_timeout_callback;

		CTcpSocketServer(unsigned int listener
			, IIOServer* io
			, MessageObjPool* msgobjpool
			, MessageMemPool* msgmempool
			, tcp_connect_callback cbconnect
			, tcp_close_callback cbclose
			, tcp_message_callback cbmessage
			, tcp_timeout_callback cbtimeout, unsigned int timeout_ms)
			: m_listener(listener)
			, m_io(io), m_msgobjpool(msgobjpool), m_msgmempool(msgmempool)
			, m_timeout_timer(0)
			, m_socket(0), m_socketid(0), m_status(SSS_CLOSED), m_busing(false), m_brecvhead( true )
			, m_connect_cb(cbconnect)
			, m_close_cb(cbclose)
			, m_message_cb(cbmessage)
			, m_timeout_cb(cbtimeout), m_timeout_ms(timeout_ms)
		{
			if (m_timeout_ms < 100)
				m_timeout_ms = 100;

			m_timeout_timer = ITimer::Create();

			XH_ASSERT(m_io&&m_msgobjpool&&m_msgmempool&&m_timeout_timer)(m_io)(m_msgobjpool)(m_msgmempool)(m_timeout_timer);
			m_io->Retain();

			++sc_server_socket_count;
		}

		virtual ~CTcpSocketServer()
		{
			if (m_socket)
			{
				m_socket->Release();
				m_socket = 0;
			}
			if (m_timeout_timer)
			{
				m_timeout_timer->Release();
				m_timeout_timer = 0;
			}
			if ( m_io )
			{
				m_io->Release();
				m_io = 0;
			}

			--sc_server_socket_count;
		}

		bool BeConnected(ITcpSocket* socket)
		{
			if ( !socket )
			{
				return false;
			}

			if (m_busing || m_status != SSS_CLOSED || !m_io)
			{
				return false;
			}

			if (m_socket || !m_timeout_timer)
			{
				return false;
			}

			socket->Init_Accepter(m_io, this);

			Retain();
			socket->Retain();
			m_io->Post(std::bind(&ImplSocket::postin_beconnected, this, socket));

			return true;
		}

		void Close()
		{
			if (!m_busing)
				return;

			Retain();
			m_io->Post(std::bind(&ImplSocket::postin_close, this));
		}

		bool Send(CMessageHead& head, CIOBuffer* msg)
		{
			if (!msg || msg->LengthWrite() <= 0)
			{
				return false;
			}

			if (!IsOK())
			{
				return false;
			}

			Retain();
			msg->Retain();
			m_io->Post(std::bind(&ImplSocket::postin_send, this, head, msg));

			return true;
		}

		bool IsOK()
		{
			return m_busing && (m_status == SSS_ESTABLISHED);
		}

		void SetUser(const User& user)
		{
			m_user = user;
		}

		User& GetUser()
		{
			return m_user;
		}

		// 本地ip和端口
		std::string GetLocalIP(void)
		{
			return m_socket ? m_socket->GetLocalIP() : "";
		}

		unsigned int GetLocalPort(void)
		{
			return m_socket ? m_socket->GetLocalPort() : 0;
		}
		// 远端ip和端口
		std::string GetPeerIP(void)
		{
			return m_socket ? m_socket->GetPeerIP() : "";
		}

		unsigned int GetPeerPort(void)
		{
			return m_socket ? m_socket->GetPeerPort() : 0;
		}
		//
		unsigned int GetSocketID(void)
		{
			if (m_socketid==0 && m_socket)
			{
				m_socketid = m_socket->GetSocketID();
			}

			return m_socketid;
		}

		//
		unsigned int GetListener()
		{
			return m_listener;
		}

	public:
		virtual void On_Connect(unsigned int socketid, int errid)
		{
			if (tcp_ok == errid)
			{
				m_status = SSS_ESTABLISHED;

				if (m_connect_cb)
				{
					m_connect_cb(m_user, GetSocketID(), GetListener());
				}

				m_brecvhead = true;
				async_recv();
			}
			else
			{
				XH_LOG_WARN(logname_base, "server be connected failed, socketid:" << socketid << " errid:" << errid);
			}
		}

		virtual void On_Send(unsigned int socketid, int errid, CIOBuffer* buffer)
		{
			if (errid)
			{
				XH_LOG_WARN(logname_base, "server send failed, socketid:" << socketid << " errid:" << errid);
			}
		}

		virtual void On_Recv(unsigned int socketid, int errid, CIOBuffer* buffer)
		{
			if (errid)
			{
				XH_LOG_WARN(logname_base, "server recv failed, socketid:" << socketid << " errid:" << errid);
				return;
			}

			if (m_brecvhead)
			{
				if (m_recvhead.decode_header(*buffer))
				{
					m_brecvhead = false;
					async_recv();
				}
				else
				{
					m_socket->Fini();
				}
			}
			else
			{
				if (m_message_cb)
				{
					m_message_cb(m_user, GetSocketID(), GetListener(), m_recvhead, buffer);
				}

				m_brecvhead = true;
				async_recv();
			}
		}

		virtual void On_Close(unsigned int socketid, int errid)
		{
			m_status = SSS_CLOSED;

			if (m_close_cb)
			{
				m_close_cb(m_user, GetSocketID(), GetListener());
			}

			m_socket->Fini();
			if (m_timeout_timer)
			{
				m_timeout_timer->Fini();
			}
		}

		virtual void On_Timer(unsigned int timerid, int errid)
		{
			timeout();
		}

	private:
		CIOBuffer* new_iobuffer(unsigned int bufflen)
		{
			if (m_msgmempool && m_msgobjpool)
			{
				char* allocated_buf = (char*)m_msgmempool->Allocate(bufflen);
				if (!allocated_buf)
				{
					XH_LOG_ERROR(logname_base, "server new io buffer failed, mempool allocate size:" << bufflen);
					return 0;
				}

				CIOBuffer* io_buf = m_msgobjpool->New_Object(allocated_buf, bufflen);
				if (!io_buf)
				{
					m_msgmempool->Free(allocated_buf);

					XH_LOG_ERROR(logname_base, "server new io buffer failed, objpool allocate size:" << bufflen);
					return 0;
				}

				io_buf->Set_DestoryCB([&](CPPRef* ref)
				{
					CIOBuffer* temptbuf = dynamic_cast<CIOBuffer*>(ref);
					m_msgmempool->Free(temptbuf->GetBuffer());
					m_msgobjpool->Delete_Object(temptbuf);
				});

				return io_buf;
			}

			return 0;
		}

		void postin_beconnected(ITcpSocket* socket)
		{
			XH_GUARD([&]{Release(); });

			if (m_busing || m_status != SSS_CLOSED || !m_io)
			{
				socket->Release();
				return;
			}

			m_socket = socket;

			m_timeout_timer->Init(m_io, this);
			m_timeout_timer->Async_Wait(m_timeout_ms);

			m_busing = true;
			m_status = SSS_ESTABLISHING;
		}

		void postin_close(void)
		{
			XH_GUARD([&]{Release(); });

			m_busing = false;

			if (m_socket)
			{
				m_socket->Fini();
			}

			if (m_timeout_timer)
			{
				m_timeout_timer->Fini();
			}
		}

		void timeout(void)
		{
			if (m_timeout_cb && IsOK())
			{
				m_timeout_cb(m_user, GetSocketID(), GetListener());
			}
		}

		void async_recv()
		{
			if (IsOK())
			{
				// 读取body
				unsigned short recvlen = 0;
				if (m_brecvhead)
				{
					// 读取头
					recvlen = m_recvhead.size();
				}
				else
				{
					recvlen = m_recvhead.msglen();
				}


				CIOBuffer* buff = new_iobuffer(recvlen);
				if (!buff)
					return;

				m_socket->Async_RecvSome(buff);
				buff->Release();
			}
		}

		void postin_send(CMessageHead head, CIOBuffer* buffer)
		{
			XH_GUARD([&]{if (buffer) buffer->Release(); Release(); });

			async_send(head, buffer);
		}

		void async_send(CMessageHead& head, CIOBuffer* buffer)
		{
			if (IsOK())
			{
				unsigned int sendlen = msg->LengthWrite() + head.size();

				CIOBuffer* sendbuff = new_iobuffer(sendlen);
				if (!sendbuff)
					return;

				if (head.encode_header(*sendbuff))
				{
					sendbuff->Write(buffer->GetCurRead(), buff->AvailRead());

					m_socket->Async_Send(sendbuff);
				}
				else
				{
					XH_LOG_ERROR(logname_base, "server send buffer failed");
				}

				sendbuff->Release();
			}
		}

	private:
		unsigned int		m_listener;

		IIOServer*			m_io;				// 
		MessageObjPool*		m_msgobjpool;
		MessageMemPool*		m_msgmempool;

		ITimer*				m_timeout_timer;

		ITcpSocket*			m_socket;
		unsigned int		m_socketid;
		unsigned char		m_status;			// 状态
		volatile bool		m_busing;			// 是否使用
		CMessageHead		m_recvhead;
		bool				m_brecvhead;

		tcp_connect_callback	m_connect_cb;
		tcp_close_callback		m_close_cb;
		tcp_message_callback	m_message_cb;
		tcp_timeout_callback	m_timeout_cb;
		unsigned int			m_timeout_ms;

		User				m_user;
	};
};
