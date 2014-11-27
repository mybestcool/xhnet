
#pragma once

#include "iobuffer.h"

namespace xhnet
{
	class IIOServer;
	class ITcpSocket;

	enum tcpsocket_err
	{
		tcp_ok	= 0,

		tcp_listenfail_iperr,
		tcp_listenfail_createerr,
		tcp_listenfail_binderr,
		tcp_listenfail_listenerr,

		tcp_connectfail_iperr,
		tcp_connectfail_createerr,
		tcp_connectfail_connecterr,
		tcp_connectfail_timeout,

		tcp_recvfail_closedbypeer,
		tcp_recvfail_recverr,
		tcp_sendfail_senderr,

		tcp_close_bylocal,
	};

	//
	//
	// socket状态如下
	//
	//       Init             Listen/Connect               On_Listen/On_Connect/On_Connect
	// null-------->init--------------------------->connect------------------------------>common
	//   ^           ^									|									|
	//   |           |									|									|
	//	 |		     ------------------------------------------------------------------------
	//   |										On_Close/On_Close/On_Close
	//   |                                                 |
	//   |--------------------------------------------------
	//			Fini
	//   (init状态下调用Fini是不会调用On_Close的)
	//	 (On_Accept的socket的状态是null，需要在On_Accept中Init_Accepter，否则socket自动释放)
	//	 (init以后必须调用Fini)
	//
	//
	// 1、任何socket Init以后，都需要调用Fini来结束，
	// 2、socket调用Fini主动关闭（tcp_close_bylocal），还是socket本身的原因关闭，都会调用On_Close
	// 3、Fini可以多次调用，Fini以后需要重用额话则需再次Init
	// 4、socket在回调On_Close的时候，未被调用过Fini，则可以再次Listen、Connect
	// 5、Async_Send、Async_Recv、Async_RecvSome只有在回调On_Listen、On_Connect、On_Connect成功的时候才有效果，即socket进入common状态
	//
	//
	//
	class ICBTcpListener : virtual public CPPRef
	{
	public:
		virtual ~ICBTcpListener() { }

		virtual void On_Listen(unsigned int socketid, int errid) = 0;
		virtual void On_Accept(unsigned int socketid, ITcpSocket* socket) = 0;
		virtual void On_Close(unsigned int socketid, int errid) = 0;
	};

	class ICBTcpAccepter : virtual public CPPRef
	{
	public:
		virtual ~ICBTcpAccepter() { }

		virtual void On_Connect(unsigned int socketid, int errid) = 0;
		virtual void On_Send(unsigned int socketid, int errid, CIOBuffer* buffer) = 0;
		virtual void On_Recv(unsigned int socketid, int errid, CIOBuffer* buffer) = 0;
		virtual void On_Close(unsigned int socketid, int errid) = 0;
	};

	class ICBTcpConneter : virtual public CPPRef
	{
	public:
		virtual ~ICBTcpConneter() { }

		virtual void On_Connect(unsigned int socketid, int errid) = 0;
		virtual void On_Send(unsigned int socketid, int errid, CIOBuffer* buffer) = 0;
		virtual void On_Recv(unsigned int socketid, int errid, CIOBuffer* buffer) = 0;
		virtual void On_Close(unsigned int socketid, int errid) = 0;
	};

	class ITcpSocket : virtual public CPPRef
	{
	public:
		// 对socket初始化进行二段式初始化
		// 创建socket，只创建了一个对象，不对原始socket做任何处理
		// listener 指的是监听socket
		// accepter 指的是监听socket接收的socket
		// connecter指的是连接出去的socket
		// listener、accepter、connecter都是外部创建的
		static ITcpSocket* Create(void);

	public:
		virtual ~ITcpSocket(void) {  }

		// 监听socket初始化，这里设置了io和回调函数
		virtual bool Init_Listener(IIOServer* io, ICBTcpListener* cb) = 0;
		virtual bool Init_Accepter(IIOServer* io, ICBTcpAccepter* cb) = 0;
		virtual bool Init_Connecter(IIOServer* io, ICBTcpConneter* cb) = 0;
		// 异步关闭socket，
		virtual void Fini(void) = 0;

		// 异步监听socket，正在监听的时候，再次调用此函数，则忽略本次操作，返回false
		virtual bool Listen(const std::string& ip, unsigned int port) = 0;
		// 异步连接，正在连接的时候，再次调用此函数，则忽略本次操作，返回false
		virtual bool Connect(const std::string& ip, unsigned int port) = 0;

		//--不线程安全，只能在io线程内调用
		// 异步发送指定长度消息，发送所有可读数据
		// 调用后buffer会立即增加一个计数
		virtual bool Async_Send(CIOBuffer* buffer) = 0;
		// 异步接收指定长度消息，读满所有可读空间
		// 调用后buffer会立即增加一个计数
		virtual bool Async_Recv(CIOBuffer* buffer) = 0;
		// 异步接收任意长度消息，一旦读到数据就立即返回
		// 调用后buffer会立即增加一个计数
		virtual bool Async_RecvSome(CIOBuffer* buffer) = 0;
		//--end

		// 本地ip和端口
		virtual std::string GetLocalIP(void) = 0;
		virtual unsigned int GetLocalPort(void) = 0;
		// 远端ip和端口
		virtual std::string GetPeerIP(void) = 0;
		virtual unsigned int GetPeerPort(void) = 0;
		//
		virtual unsigned int GetSocketID(void) = 0;
	};
};


