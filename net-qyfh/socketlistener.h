
#pragma once

#include "qyfhhead.h"
#include "msgstream.h"


namespace xhnet_qyfh
{
	static unsigned int sc_listener_socket_count = 0;

	template<class User>
	class CTcpSocketListener : public ICBTcpListener
	{
	public:
		enum SLStatus
		{
			SLS_CLOSED = 0,
			SLS_LISTENING,
			SLS_LISTENED,
		};

		typedef CTcpSocketListener<User>									ImplSocket;
		// 监听成功或者失败
		typedef std::function< void(User, unsigned int, bool) >				tcp_listen_callback;
		// 接受到一个链接
		typedef std::function< void(User, unsigned int, ITcpSocket*) >		tcp_accept_callback;
		// socket关闭通知，
		typedef std::function< void(User, unsigned int) >					tcp_close_callback;

		CTcpSocketListener(IIOServer* io
			, tcp_listen_callback cblisten
			, tcp_accept_callback cbaccept
			, tcp_close_callback cbclose)
			: m_io(io)
			, m_socketid(0)
			, m_status(SLS_CLOSED)
			, m_listen_cb( cblisten )
			, m_accept_cb( cbaccept )
			, m_close_cb( cbclose )
		{
			m_socket = ITcpSocket::Create();

			XH_ASSERT(m_io&&m_socket)(m_io)(m_socket);

			m_io->Retain();

			++sc_listener_socket_count;
		}

		~CTcpSocketListener(void)
		{
			if (m_socket)
			{
				m_socket->Release();
				m_socket = 0;
			}

			if (m_io)
			{
				m_io->Release();
				m_io = 0;
			}

			--sc_listener_socket_count;
		}

		bool Listen(const std::string& ip, unsigned int port)
		{
			if ( ip.empty() || port<=0 )
			{
				return false;
			}

			if ( m_status != SLS_CLOSED || !m_io)
			{
				return false;
			}

			if (!m_socket)
			{
				return false;
			}

			Retain();

			m_io->Post(std::bind(&ImplSocket::postin_listen, this, ip, port));

			return true;
		}

		void Close(void)
		{
			m_io->Post(std::bind(&ImplSocket::postin_close, this));
		}

		bool IsOK()
		{
			return (m_status == SLS_LISTENED);
		}

		void SetUser(const User& user)
		{
			m_user = user;
		}

		User& GetUser()
		{
			return m_user;
		}

		unsigned int GetSocketID(void)
		{
			if (m_socketid == 0)
			{
				if (m_socket)
				{
					m_socketid = m_socket->GetSocketID();
				}
			}

			return m_socketid;
		}

	public:
		virtual void On_Listen(unsigned int socketid, int errid)
		{
			if ( errid )
			{
				if (m_listen_cb)
					m_listen_cb(m_user, GetSocketID(), false);
			}
			else
			{ 
				if (m_listen_cb)
					m_listen_cb(m_user, GetSocketID(), true);

				m_status = SLS_LISTENED;
			}
		}

		virtual void On_Accept(unsigned int socketid, ITcpSocket* socket)
		{
			if ( m_accept_cb )
			{
				m_accept_cb(m_user, socketid, socket);
			}
		}

		virtual void On_Close(unsigned int socketid, int errid)
		{
			m_status = SLS_CLOSED;

			if (m_close_cb)
				m_close_cb(m_user, GetSocketID());
		}

	private:
		void postin_listen(string ip, unsigned int port)
		{
			XH_GUARD([&]{Release(); });

			CScopeGuard guard_onfailed([&]
			{
				if (m_listen_cb) 
					m_listen_cb(m_user, GetSocketID(), false);
				if (m_close_cb)
					m_close_cb(m_user, GetSocketID());

				XH_LOG_ERROR(logname_base, "post listen failed, listen ip:" << ip << ", port:" << port);
			});

			if (m_status != SLS_CLOSED || !m_io)
			{
				return;
			}

			if (!m_socket->Init_Listener(m_io, this))
				return;

			if (!m_socket->Listen(ip, port))
				return;

			guard_onfailed.Dismiss();
			m_status = SLS_LISTENING;
		}

		void postin_close(void)
		{
			XH_GUARD([&]{Release(); });

			if (m_socket)
			{
				m_socket->Fini();
			}
		}

	private:
		IIOServer*					m_io;				//

		ITcpSocket*					m_socket;
		unsigned int				m_socketid;
		unsigned char				m_status;			// 状态

		tcp_listen_callback			m_listen_cb;
		tcp_accept_callback			m_accept_cb;
		tcp_close_callback			m_close_cb;

		User						m_user;
	};
};
