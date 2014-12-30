
#pragma once

#include "qyfhhead.h"
#include "msgstream.h"

namespace xhnet_qyfh
{
	static unsigned int sc_client_socket_count = 0;

	template<class User, class CMessageHead>
	class CTcpSocketClient : public ICBTcpConneter, public ICBTimer
	{
	public:
		enum SCStatus
		{
			SCS_CLOSED = 0,
			SCS_CONNETING,
			SCS_CONNETED,
		};

		typedef CTcpSocketClient<User, CMessageHead>	ImplSocket;
		typedef COPool<CIOBuffer, mutex>				MessageObjPool;
		typedef CMPool<mutex>							MessageMemPool;

		// socket连接上来的通知
		typedef std::function< void(User, unsigned int) >					tcp_connect_callback;
		// socket关闭通知，
		typedef std::function< void(User, unsigned int) >					tcp_close_callback;
		// socket接收消息的通知 socketid,msglen,msgid,msg
		typedef std::function< void(User, unsigned int, const CMessageHead&, CIOBuffer*) >	tcp_message_callback;
		// socket 定时操作，比如心跳等
		typedef std::function< void(User, unsigned int) >					tcp_timeout_callback;


		enum TimerClick
		{
			RECONNECT_TIME_MS = 10000,	// 10秒检查一次重连
		};

		CTcpSocketClient(IIOServer* io
			, MessageObjPool* msgobjpool
			, MessageMemPool* msgmempool
			, tcp_connect_callback cbconnect
			, tcp_close_callback cbclose
			, tcp_message_callback cbmessage
			, tcp_timeout_callback cbtimeout, unsigned int timeout_ms)
			: m_io(io), m_msgobjpool(msgobjpool), m_msgmempool(msgmempool)
			, m_reconnect_timer(0), m_timeout_timer(0)
			, m_socket(0), m_socketid(0), m_status(SCS_CLOSED), m_busing(false), m_brecvhead(true)
			, m_connect_cb(cbconnect)
			, m_close_cb(cbclose)
			, m_message_cb(cbmessage)
			, m_timeout_cb(cbtimeout), m_timeout_ms(timeout_ms)
		{
			if (m_timeout_ms < 100) 
				m_timeout_ms = 100;

			m_socket			= ITcpSocket::Create();
			m_reconnect_timer	= ITimer::Create();
			m_timeout_timer		= ITimer::Create();

			XH_ASSERT(m_io&&m_msgobjpool&&m_msgmempool&&m_socket&&m_reconnect_timer&&m_timeout_timer)
				(m_io)(m_msgobjpool)(m_msgmempool)(m_socket)(m_reconnect_timer)(m_timeout_timer);

			m_io->Retain();

			++sc_client_socket_count;
		}

		virtual ~CTcpSocketClient()
		{
			if (m_socket)
			{
				m_socket->Release();
				m_socket = 0;
			}
			if (m_reconnect_timer)
			{
				m_reconnect_timer->Release();
				m_reconnect_timer = 0;
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

			--sc_client_socket_count;
		}

		bool Connect(const string& host, unsigned int port)
		{
			if (host.empty() || port <= 0)
			{
				return false;
			}

			if (m_busing || m_status != SCS_CLOSED || !m_io)
			{
				return false;
			}

			if (!m_socket || !m_reconnect_timer || !m_timeout_timer)
			{
				return false;
			}

			Retain();
			m_io->Post( std::bind(&ImplSocket::postin_connect, this, host, port) );

			return true;
		}

		void Close()
		{
			if (!m_busing)
				return;

			Retain();
			m_io->Post( std::bind(&ImplSocket::postin_close, this) );
		}

		bool Send(CMessageHead& head, CIOBuffer* msg)
		{
			if ( !msg || msg->LengthWrite()<=0)
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
			return m_busing && (m_status == SCS_CONNETED);
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

	public:
		virtual void On_Connect(unsigned int socketid, int errid)
		{
			if (tcp_ok == errid)
			{
				m_status = SCS_CONNETED;

				if ( m_connect_cb )
				{
					m_connect_cb( m_user, GetSocketID());
				}

				m_brecvhead = true;
				async_recv();
			}
			else
			{
				XH_LOG_WARN(logname_base, "client connect failed, socketid:" <<socketid << " errid:"<<errid);
			}
		}

		virtual void On_Send(unsigned int socketid, int errid, CIOBuffer* buffer)
		{
			if (errid)
			{
				XH_LOG_WARN(logname_base, "client send failed, socketid:" << socketid << " errid:" << errid);
			}
		}

		virtual void On_Recv(unsigned int socketid, int errid, CIOBuffer* buffer)
		{
			if (errid)
			{
				XH_LOG_WARN(logname_base, "client recv failed, socketid:" << socketid << " errid:" << errid);
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
					reset_socket();
				}
			}
			else
			{
				if (m_message_cb)
				{
					m_message_cb(m_user, GetSocketID(), m_recvhead, buffer);
				}

				m_brecvhead = true;
				async_recv();
			}
		}

		virtual void On_Close(unsigned int socketid, int errid)
		{
			m_status = SCS_CLOSED;

			if (m_close_cb)
			{
				m_close_cb(m_user, GetSocketID());
			}
		}

		virtual void On_Timer(unsigned int timerid, int errid)
		{
			if (m_reconnect_timer && m_reconnect_timer->GetTimerID()==timerid)
			{
				reconnect();
			}
			else if (m_timeout_timer && m_timeout_timer->GetTimerID()==timerid)
			{
				timeout();
			}
		}

	private:
		CIOBuffer* new_iobuffer(unsigned int bufflen)
		{
			if (m_msgmempool && m_msgobjpool)
			{
				char* allocated_buf = (char*)m_msgmempool->Allocate(bufflen);
				if (!allocated_buf)
				{
					XH_LOG_ERROR(logname_base, "client new io buffer failed, mempool allocate size:" << bufflen);
					return 0;
				}

				CIOBuffer* io_buf = m_msgobjpool->New_Object(allocated_buf, bufflen);
				if (!io_buf)
				{
					m_msgmempool->Free(allocated_buf);

					XH_LOG_ERROR(logname_base, "client new io buffer failed, objpool allocate size:" << bufflen);
					return 0;
				}

				// 由于
				// 1.CTcpSocket保存了本对象的计数，当CTcpSocket计数release前，都会处理掉接收和发送的CIOBuffer
				// 2.CNetIO保存了本对象的计数，调用了onclose后，会析构本对象，即处理完接收消息
				// 从而保证了m_msgmempool，m_msgobjpool的必定存在。
				// 所以可以这样使用
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

		void postin_connect(string host, unsigned int port)
		{
			XH_GUARD([&]{Release(); });

			if (m_busing || m_status != SCS_CLOSED || !m_io)
			{
				return;
			}

			vector<string> ips = IIOServer::Resolve_DNS(host);
			if (ips.empty())
			{
				return;
			}

			m_host = host;
			m_port = port;

			m_socket->Init_Connecter(m_io, this);
			m_socket->Connect(ips[0], port);

			// timer
			m_reconnect_timer->Init(m_io, this);
			m_reconnect_timer->Async_Wait(RECONNECT_TIME_MS);

			m_timeout_timer->Init(m_io, this);
			m_timeout_timer->Async_Wait(m_timeout_ms);

			m_busing = true;
			m_status = SCS_CONNETING;
		}

		void postin_close(void)
		{
			XH_GUARD([&]{Release(); });

			m_busing = false;

			if ( m_socket )
			{
				m_socket->Fini();
			}

			if (m_timeout_timer)
			{
				m_timeout_timer->Fini();
			}

			if (m_reconnect_timer)
			{
				m_reconnect_timer->Fini();
			}
		}

		void reconnect(void)
		{
			if (m_busing 
				&& m_status == SCS_CLOSED
				&& m_socket)
			{
				vector<string> ips = IIOServer::Resolve_DNS(m_host);
				if (ips.empty())
				{
					return;
				}

				m_socket->Connect(ips[0], m_port);
			}
		}

		void timeout(void)
		{
			if (m_timeout_cb && IsOK())
			{
				m_timeout_cb(m_user, GetSocketID());
			}
		}

		void async_recv( )
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

				m_socket->Async_Recv(buff);
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
			if ( IsOK() )
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
					XH_LOG_ERROR(logname_base, "client send buffer failed");
				}

				sendbuff->Release();
			}
		}

		void reset_socket()
		{
			if (m_socket && m_msgmempool && m_msgobjpool)
			{
				CIOBuffer* recvbuf = m_msgobjpool->New_Object((char*)0, 0);
				if (!recvbuf)
				{
					return;
				}

				recvbuf->Set_DestoryCB([&](CPPRef* ref){ m_msgobjpool->Delete_Object(dynamic_cast<CIOBuffer*>(ref)); });

				m_socket->Async_Send(recvbuf);

				recvbuf->Release();
			}
		}

	private:
		IIOServer*			m_io;				// 
		MessageObjPool*		m_msgobjpool;
		MessageMemPool*		m_msgmempool;

		ITimer*				m_reconnect_timer;
		ITimer*				m_timeout_timer;

		string				m_host;
		unsigned int		m_port;
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

