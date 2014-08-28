
#pragma once

#include "impl_nethead.h"

namespace xhnet
{
	class CIOServer;

	class CTcpSocket : public ITcpSocket, public CInheritOPool<CTcpSocket, std::mutex>
	{
	public:
		enum type
		{
			type_null = 0,
			type_listener,
			type_accepter,
			type_connecter,
		};

		enum status
		{
			status_null = 0,
			status_connect,
			status_common,
		};
	public:
		CTcpSocket();
		virtual ~CTcpSocket();

		// 监听socket初始化，这里设置了io和回调函数
		virtual bool Init_Listener(IIOServer* io, ICBTcpListener* cb);
		virtual bool Init_Accepter(IIOServer* io, ICBTcpAccepter* cb);
		virtual bool Init_Connecter(IIOServer* io, ICBTcpConneter* cb);
		// 异步关闭socket，
		virtual void Fini(void);

		// 异步监听socket，正在监听的时候，再次调用此函数，则忽略本次操作，返回false
		virtual bool Listen(const std::string& ip, unsigned int port);
		// 异步连接，正在连接的时候，再次调用此函数，则忽略本次操作，返回false
		virtual bool Connect(const std::string& ip, unsigned int port);

		//--不线程安全，只能在io线程内调用
		// 异步发送指定长度消息，发送所有可读数据
		// 调用后buffer会立即增加一个计数
		virtual bool Async_Send(CIOBuffer* buffer);
		// 异步接收指定长度消息，读满所有可读空间
		// 调用后buffer会立即增加一个计数
		virtual bool Async_Recv(CIOBuffer* buffer);
		// 异步接收任意长度消息，一旦读到数据就立即返回
		// 调用后buffer会立即增加一个计数
		virtual bool Async_RecvSome(CIOBuffer* buffer);
		//--end

		// 本地ip和端口
		virtual std::string GetLocalIP(void);
		virtual unsigned int GetLocalPort(void);
		// 远端ip和端口
		virtual std::string GetPeerIP(void);
		virtual unsigned int GetPeerPort(void);
		//
		virtual unsigned int GetSocketID(void);

	public:
		static void on_accept_callback(evutil_socket_t sock, short flag, void* p);
		static void on_recv_callback(evutil_socket_t sock, short flag, void* p);
		static void on_send_callback(evutil_socket_t sock, short flag, void* p);

	protected:
		void real_listen(std::string ip, unsigned int port);
		void real_connect(std::string ip, unsigned int port);
		void real_fini();

		void on_inter_listen_accept(evutil_socket_t listenfd);
		void on_inter_accept_accept(evutil_socket_t fd, socketaddr* peeraddr, int addrlen);
		void on_inter_recv(short flag);
		void on_inter_send(short flag);
		void on_inter_close(int errid, bool bconnecting);
	protected:
		unsigned char	m_type;
		bool			m_binit;
		unsigned char	m_status;
		unsigned int	m_id;
		evutil_socket_t	m_socket;

		CIOServer*		m_io;

		std::string		m_localip;
		unsigned int	m_localport;

		std::string		m_peerip;
		unsigned int	m_peerport;

		// 匿名联合
		union
		{
			ICBTcpListener*	m_lcb;
			ICBTcpAccepter*	m_acb;
			ICBTcpConneter*	m_ccb;
		};

		unsigned char	m_bassign;	// 0x01 表示已经对m_ev_readassign过 0x02对m_ev_write assign过
		::event			m_ev_read;
		::event			m_ev_write;

		std::queue<ITcpSocket*>	m_wait_accept;
		std::queue<CIOBuffer*>	m_wait_send;
		std::queue<CIOBuffer*>	m_wait_recv;

	private:
		CTcpSocket(const CTcpSocket&) = delete;
		CTcpSocket& operator=(const CTcpSocket&) = delete;
	};
};

