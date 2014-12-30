
#pragma once

#include "threadio.h"
#include "socketclient.h"
#include "socketlistener.h"
#include "socketserver.h"

namespace xhnet_qyfh
{
	// 对socket
	template<class CMessageHead
		, class UserServer = int
		, class UserListener = int
		, class UserClient = int
		, unsigned int ServerThreadCnt = 1
		, unsigned int ListenerThreadCnt = 1
		, unsigned int ClientThreadCnt = 1>
	class CNetIO
	{
	public:
		typedef CNetIO<CMessageHead, UserServer, UserListener, UserClient, ServerThreadCnt, ListenerThreadCnt, ClientThreadCnt>	NetImpl;

		typedef CTcpSocketClient<UserClient, CMessageHead>			TCPClient;
		typedef CTcpSocketListener<UserListener>					TCPListener;
		typedef CTcpSocketServer<UserServer, CMessageHead>			TCPServer;

		typedef typename unordered_map<unsigned int, TCPClient*>		TCPClientMap;
		typedef typename unordered_map<unsigned int, TCPListener*>		TCPListenerMap;
		typedef typename unordered_map<unsigned int, TCPServer*>		TCPServerMap;

		typedef function<bool(UserListener, unsigned int)>												listener_can_accept_callback;
		// socketid, socket状态，true表示连接上 false表示断开
		typedef function<void(UserServer, unsigned int, unsigned int, bool)>							server_socket_status_callback;
		//socketid, listener, 
		typedef function<void(UserServer, unsigned int, unsigned int, const CMessageHead&, CIOBuffer*)>	server_recv_message_callback;
		//
		typedef function<void(UserServer, unsigned int, unsigned int)>									server_heartbeat_callback;

		// socketid, socket状态，true表示连接上 false表示断开
		typedef function<void(UserClient, unsigned int, bool)>											client_socket_status_callback;
		//socketid, listener, 
		typedef function<void(UserClient, unsigned int, const CMessageHead&, CIOBuffer*)>				client_recv_message_callback;
		//
		typedef function<void(UserClient, unsigned int)>												client_heartbeat_callback;

		CNetIO(IIOServer* mainio)
			: m_mainio( mainio )
		{
			XH_ASSERT(m_mainio)(m_mainio);
			m_mainio->Retain();

			for (unsigned int i = 0; i < ListenerThreadCnt; ++i)
			{
				m_listener_threads.push_back(new CThreadIO());
			}

			for (unsigned int i = 0; i < ServerThreadCnt; ++i)
			{
				m_server_threads.push_back(new CThreadIO());
			}

			for (unsigned int i = 0; i < ClientThreadCnt; ++i)
			{
				m_client_threads.push_back(new CThreadIO());
			}
		}

		virtual ~CNetIO()
		{
			for (vector<CThreadIO*>::iterator it = m_listener_threads.begin(); it != m_listener_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if ( io )
				{
					io->Release();
				}
			}

			for (vector<CThreadIO*>::iterator it = m_server_threads.begin(); it != m_server_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->Release();
				}
			}

			for (vector<CThreadIO*>::iterator it = m_client_threads.begin(); it != m_client_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->Release();
				}
			}

			if (m_mainio)
			{
				m_mainio->Release();
			}
		}

		// 所有接口 都需要在 main thread io 中调用
		void SetCallback_Server(listener_can_accept_callback cbcan
			, server_socket_status_callback cbstatus
			, server_recv_message_callback cbmessage
			, server_heartbeat_callback cbheart)
		{
			m_can_accept_cb = cbcan;
			m_server_status_cb = cbstatus;
			m_server_message_cb = cbmessage;
			m_server_heartbeat_cb = cbheart;
		}

		void SetCallback_Client(client_socket_status_callback cbstatus
			, client_recv_message_callback cbmessage
			, client_heartbeat_callback cbheart)
		{
			m_client_status_cb = cbstatus;
			m_client_message_cb = cbmessage;
			m_client_heartbeat_cb = cbheart;
		}

		// 返回socketid，失败返回0
		unsigned int Listen(const std::string& ip, unsigned int port)
		{
			CThreadIO* io = RandGetTcpListenIO();

			if ( !io )
			{
				return 0;
			}

			TCPListener* listener = m_listen_objpool.New_Object(io->GetIOServer()
				, std::bind(&NetImpl::on_listen_succeed, this, placeholders::_1, placeholders::_2, placeholders::_3)
				, std::bind(&NetImpl::on_listen_accept, this, placeholders::_1, placeholders::_2, placeholders::_3)
				, std::bind(&NetImpl::on_listen_close, this, placeholders::_1, placeholders::_2));

			if ( !listener )
			{
				return 0;
			}

			listener->Set_DestoryCB([&](CPPRef* ref){ m_listen_objpool.Delete_Object(dynamic_cast<TCPListener*>(ref)); });

			if (!listener->Listen(ip, port))
			{
				listener->Close();
				listener->Release();
				return 0;
			}

			m_listeners.insert(std::make_pair(listener->GetSocketID(), listener));

			return listener->GetSocketID();
		}

		unsigned int Connect(const string& host, unsigned int port)
		{
			CThreadIO* io = RandTcpServerIO();

			if (!io)
			{
				return 0;
			}

			TCPClient* client = m_client_objpool.New_Object(io->GetIOServer()
				, &m_client_msg_objpool, &m_client_msg_mempool
				, std::bind(&NetImpl::on_client_connect, this, placeholders::_1, placeholders::_2)
				, std::bind(&NetImpl::on_client_close, this, placeholders::_1, placeholders::_2)
				, std::bind(&NetImpl::on_client_message, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4)
				, std::bind(&NetImpl::on_client_timeout, this, placeholders::_1, placeholders::_2), 10000);

			if (!client)
			{
				return 0;
			}

			client->Set_DestoryCB([&](CPPRef* ref){ m_client_objpool.Delete_Object(dynamic_cast<TCPClient*>(ref)); });

			if ( !client->Connect( host, port ) )
			{
				client->Close();
				client->Release();
				return 0;
			}

			m_clients.insert(::make_pair(client->GetSocketID(), client));

			return client->GetSocketID();
		}

		void Close(unsigned int socketid)
		{
			{
				TCPClientMap::iterator it = m_clients.find(socketid);
				if (it != m_clients.end())
				{
					it->second->Close();
					it->second->Release();
					m_clients.erase(it);
					return;
				}
			}

			{
				TCPListenerMap::iterator it = m_listeners.find(socketid);
				if (it != m_listeners.end())
				{
					it->second->Close();
					return;
				}
			}

			{
				TCPServerMap::iterator it = m_servers.find(socketid);
				if (it != m_servers.end())
				{
					it->second->Close();
					return;
				}
			}
		}

		void Run()
		{
			for (vector<CThreadIO*>::iterator it = m_listener_threads.begin(); it != m_listener_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->Run();
				}
			}

			for (vector<CThreadIO*>::iterator it = m_server_threads.begin(); it != m_server_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->Run();
				}
			}

			for (vector<CThreadIO*>::iterator it = m_client_threads.begin(); it != m_client_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->Run();
				}
			}
		}

		void Stop()
		{
			for (vector<CThreadIO*>::iterator it = m_listener_threads.begin(); it != m_listener_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->Stop();
				}
			}

			for (vector<CThreadIO*>::iterator it = m_server_threads.begin(); it != m_server_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->Stop();
				}
			}

			for (vector<CThreadIO*>::iterator it = m_client_threads.begin(); it != m_client_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->Stop();
				}
			}
		}

		void WaitForFinish()
		{
			for (vector<CThreadIO*>::iterator it = m_listener_threads.begin(); it != m_listener_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->WaitForFinish();
				}
			}

			for (vector<CThreadIO*>::iterator it = m_server_threads.begin(); it != m_server_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->WaitForFinish();
				}
			}

			for (vector<CThreadIO*>::iterator it = m_client_threads.begin(); it != m_client_threads.end(); ++it)
			{
				CThreadIO* io = *it;
				if (io)
				{
					io->WaitForFinish();
				}
			}
		}

		void SetListenUser(unsigned int socketid, const UserListener& user)
		{
			TCPListenerMap::iterator it = m_listeners.find(socketid);
			if (it != m_listeners.end())
			{
				it->second->SetUser(user);
				return;
			}
		}

		void SetServerUser(unsigned int socketid, const UserServer& user)
		{
			TCPServerMap::iterator it = m_servers.find(socketid);
			if (it != m_servers.end())
			{
				it->second->SetUser(user);
				return;
			}
		}

		void SetClientUser(unsigned int socketid, const UserClient& user)
		{
			TCPClientMap::iterator it = m_clients.find(socketid);
			if (it != m_clients.end())
			{
				it->second->SetUser(user);
				return;
			}
		}

		bool Send(unsigned int socketid, CMessageHead& head, CIOBuffer* msg)
		{
			{
				TCPClientMap::iterator it = m_clients.find(socketid);
				if (it != m_clients.end())
				{
					return it->second->Send(head, msg);
				}
			}

			{
				TCPServerMap::iterator it = m_servers.find(socketid);
				if (it != m_servers.end())
				{
					return it->second->Send(head, msg);
				}
			}

			return false;
		}

		bool IsOK(unsigned int socketid)
		{
			{
				TCPClientMap::iterator it = m_clients.find(socketid);
				if (it != m_clients.end())
				{
					return it->second->IsOK();
				}
			}

			{
				TCPListenerMap::iterator it = m_listeners.find(socketid);
				if (it != m_listeners.end())
				{
					return it->second->IsOK();
				}
			}

			{
				TCPServerMap::iterator it = m_servers.find(socketid);
				if (it != m_servers.end())
				{
					return it->second->IsOK();
				}
			}

			return false;
		}

		// 本地ip和端口
		std::string GetLocalIP(unsigned int socketid)
		{
			{
				TCPClientMap::iterator it = m_clients.find(socketid);
				if (it != m_clients.end())
				{
					return it->second->GetLocalIP();
				}
			}

			{
				TCPServerMap::iterator it = m_servers.find(socketid);
				if (it != m_servers.end())
				{
					return it->second->GetLocalIP();
				}
			}

			return "";
		}

		unsigned int GetLocalPort(unsigned int socketid)
		{
			{
				TCPClientMap::iterator it = m_clients.find(socketid);
				if (it != m_clients.end())
				{
					return it->second->GetLocalPort();
				}
			}

			{
				TCPServerMap::iterator it = m_servers.find(socketid);
				if (it != m_servers.end())
				{
					return it->second->GetLocalPort();
				}
			}

			return 0;
		}
		// 远端ip和端口
		std::string GetPeerIP(unsigned int socketid)
		{
			{
				TCPClientMap::iterator it = m_clients.find(socketid);
				if (it != m_clients.end())
				{
					return it->second->GetPeerIP();
				}
			}

			{
				TCPServerMap::iterator it = m_servers.find(socketid);
				if (it != m_servers.end())
				{
					return it->second->GetPeerIP();
				}
			}

			return "";
		}

		unsigned int GetPeerPort(unsigned int socketid)
		{
			{
				TCPClientMap::iterator it = m_clients.find(socketid);
				if (it != m_clients.end())
				{
					return it->second->GetPeerPort();
				}
			}

			{
				TCPServerMap::iterator it = m_servers.find(socketid);
				if (it != m_servers.end())
				{
					return it->second->GetPeerPort();
				}
			}

			return 0;
		}

	public:
		// listener
		void on_listen_succeed(UserListener user, unsigned int socketid, bool bsucceed)
		{
			if ( !bsucceed )
			{
				XH_LOG_FATAL(logname_base, "listener listen failed, socketid:" << socketid);
			}
			else
			{
				XH_LOG_INFO(logname_base, "listener listen succeed, socketid:" << socketid);
			}
		}

		void on_listen_accept(UserListener user, unsigned int socketid, ITcpSocket* newsocket)
		{
			CThreadIO* io = RandTcpServerIO();

			if ( !io )
			{
				newsocket->Fini();
				return;
			}

			//
			TCPServer* server = m_server_objpool.New_Object( socketid, io->GetIOServer()
				, &m_server_msg_objpool, &m_server_msg_mempool
				, std::bind(&NetImpl::on_server_connect, this, placeholders::_1, placeholders::_2, placeholders::_3)
				, std::bind(&NetImpl::on_server_close, this, placeholders::_1, placeholders::_2, placeholders::_3)
				, std::bind(&NetImpl::on_server_message, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5)
				, std::bind(&NetImpl::on_server_timeout, this, placeholders::_1, placeholders::_2, placeholders::_3)
				, 10000);

			if (!server)
			{
				newsocket->Fini();
				return;
			}

			server->Set_DestoryCB([&](CPPRef* ref){ m_server_objpool.Delete_Object(dynamic_cast<TCPServer*>(ref)); });

			if (!server->BeConnected(newsocket))
			{
				newsocket->Fini();
				newsocket->Release();
				return;
			}

			m_mainio->Post(std::bind(&NetImpl::mainio_listener_accept, this, server, user, socketid));
		}

		void on_listen_close(UserListener user, unsigned int socketid)
		{
			// do nothing
			XH_LOG_INFO(logname_base, "listener listen closed, socketid:" << socketid);
			
			m_mainio->Post(std::bind(&NetImpl::mainio_listener_close, this, user, socketid));
		}
		//

		// server
		void on_server_connect(UserServer user, unsigned int socketid, unsigned int listener)
		{
			m_mainio->Post(std::bind(&NetImpl::mainio_server_on_status, this, user, socketid, listener, true));
		}

		void on_server_message(UserServer user, unsigned int socketid, unsigned int listener, const CMessageHead& msghead, CIOBuffer* msgbody)
		{
			if (msgbody)
				msgbody->Retain();

			m_mainio->Post(std::bind(&NetImpl::mainio_server_on_message, this, user, socketid, listener, msghead, msgbody));
		}

		void on_server_close(UserServer user, unsigned int socketid, unsigned int listener)
		{
			m_mainio->Post(std::bind(&NetImpl::mainio_server_on_status, this, user, socketid, listener, false));
		}

		void on_server_timeout(UserServer user, unsigned int socketid, unsigned int listener)
		{
			m_mainio->Post(std::bind(&NetImpl::mainio_server_on_heart, this, user, socketid, listener));
		}
		//

		// client
		void on_client_connect(UserClient user, unsigned int socketid)
		{
			m_mainio->Post(std::bind(&NetImpl::mainio_client_on_status, this, user, socketid, true));
		}

		void on_client_message(UserClient user, unsigned int socketid, const CMessageHead& msghead, CIOBuffer* msgbody)
		{
			if (msgbody)
				msgbody->Retain();

			m_mainio->Post(std::bind(&NetImpl::mainio_client_on_message, this, user, socketid, msghead, msgbody));
		}

		void on_client_close(UserClient user, unsigned int socketid)
		{
			m_mainio->Post(std::bind(&NetImpl::mainio_client_on_status, this, user, socketid, false));
		}

		void on_client_timeout(UserClient user, unsigned int socketid)
		{
			m_mainio->Post(std::bind(&NetImpl::mainio_client_on_heart, this, user, socketid));
		}
		//

		CThreadIO* RandGetTcpListenIO()
		{
			unsigned int pos = GenRandUINT(0, m_listener_threads.size());
			return m_listener_threads[pos];
		}

		CThreadIO* RandTcpServerIO()
		{
			unsigned int pos = GenRandUINT(0, m_server_threads.size());
			return m_server_threads[pos];
		}

		CThreadIO* RandTcpClientIO()
		{
			unsigned int pos = GenRandUINT(0, m_client_threads.size());
			return m_client_threads[pos];
		}

		// in main thread io
		// listener
		void mainio_listener_accept(TCPServer* server, UserListener user, unsigned int socketid)
		{
			if (!m_can_accept_cb || !m_can_accept_cb(user, socketid))
			{
				server->Close();
				server->Release();

				return;
			}

			m_servers.insert(::make_pair(server->GetSocketID(), server));
		}

		void mainio_listener_close(UserListener user, unsigned int socketid)
		{
			TCPListenerMap::iterator it = m_listeners.find(socketid);
			if (it != m_listeners.end())
			{
				it->second->Close();
				it->second->Release();
				m_listeners.erase(it);
			}
		}

		// server
		void mainio_server_on_status(UserServer user, unsigned int socketid, unsigned int listener, bool bconnect)
		{
			if (m_server_status_cb)
			{
				m_server_status_cb(user, socketid, listener, bconnect);
			}

			if (!bconnect)
			{
				TCPServerMap::iterator it = m_servers.find(socketid);
				if (it != m_servers.end())
				{
					it->second->Close();
					it->second->Release();
					m_servers.erase(it);
				}
			}
		}

		void mainio_server_on_message(UserServer user, unsigned int socketid, unsigned int listener, CMessageHead_24 msghead, CIOBuffer* msgbody)
		{
			if (m_server_message_cb)
			{
				m_server_message_cb(user, socketid, listener, msghead, msgbody);
			}

			if (msgbody)
			{
				msgbody->Release();
			}
		}

		void mainio_server_on_heart(UserServer user, unsigned int socketid, unsigned int listener)
		{
			if (m_server_heartbeat_cb)
			{
				m_server_heartbeat_cb(user, socketid, listener);
			}
		}

		// client
		void mainio_client_on_status(UserClient user, unsigned int socketid, bool bconnect)
		{
			if (m_client_status_cb)
			{
				m_client_status_cb(user, socketid, bconnect);
			}
		}

		void mainio_client_on_message(UserClient user, unsigned int socketid, CMessageHead_24 msghead, CIOBuffer* msgbody)
		{
			if (m_client_message_cb)
			{
				m_client_message_cb(user, socketid, msghead, msgbody);
			}

			if (msgbody)
			{
				msgbody->Release();
			}
		}

		void mainio_client_on_heart(UserClient user, unsigned int socketid)
		{
			if (m_client_heartbeat_cb)
			{
				m_client_heartbeat_cb(user, socketid);
			}
		}
		//
	protected:
		IIOServer*					m_mainio;

		COPool<TCPListener, mutex>	m_listen_objpool;
		COPool<TCPServer, mutex>	m_server_objpool;
		COPool<TCPClient, mutex>	m_client_objpool;

		CMPool<mutex>				m_server_msg_mempool;
		COPool<CIOBuffer, mutex>	m_server_msg_objpool;

		CMPool<mutex>				m_client_msg_mempool;
		COPool<CIOBuffer, mutex>	m_client_msg_objpool;

		vector<CThreadIO*>		m_listener_threads;
		vector<CThreadIO*>		m_server_threads;
		vector<CThreadIO*>		m_client_threads;

		TCPListenerMap			m_listeners;
		TCPServerMap			m_servers;
		TCPClientMap			m_clients;


		listener_can_accept_callback	m_can_accept_cb;
		server_socket_status_callback	m_server_status_cb;
		server_recv_message_callback	m_server_message_cb;
		server_heartbeat_callback		m_server_heartbeat_cb;

		client_socket_status_callback	m_client_status_cb;
		client_recv_message_callback	m_client_message_cb;
		client_heartbeat_callback		m_client_heartbeat_cb;
	};
};
