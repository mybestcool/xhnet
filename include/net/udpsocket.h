
#pragma once

#include "iobuffer.h"

namespace xhnet
{
	class IIOServer;

	enum udpsocket_err
	{
		udp_ok = 0,

		udp_listenfail_iperr,
		udp_listenfail_createerr,
		udp_listenfail_binderr,

		udp_recvfail_closedbypeer,
		udp_recvfail_recverr,

		udp_sendfail_iperr,
		udp_sendfail_senderr,

		udp_close,
	};

	// 所有回调都会在io线程内
	class ICBUdpSocket : virtual public CPPRef
	{
	public:
		virtual ~ICBUdpSocket() { }

		virtual void On_Listen(unsigned int socketid, int errid) = 0;
		virtual void On_Send(unsigned int socketid, int errid, const std::string& ip, unsigned int port, CIOBuffer* buffer) = 0;
		virtual void On_Recv(unsigned int socketid, int errid, const std::string& ip, unsigned int port, CIOBuffer* buffer) = 0;
	};

	class IUdpSocket : virtual public CPPRef
	{
	public:
		// 对socket初始化进行二段式初始化
		// 创建socket，只创建了一个对象，不对原始socket做任何处理
		static IUdpSocket* Create(void);

	public:
		virtual ~IUdpSocket(void)						{  }

		// 监听socket初始化，这里设置了io和回调函数
		virtual bool Init(IIOServer* io, ICBUdpSocket* cb) = 0;
		// 异步关闭socket，异步计数减-，当计数减到0的时候，自动销毁
		virtual void Fini(void) = 0;

		// 异步监听socket，正在监听的时候，再次调用此函数，则忽略本次操作，返回false
		virtual bool Listen(const std::string& ip, unsigned int port) = 0;

		//--不线程安全，只能在io线程内调用
		// 异步发送指定长度消息，
		virtual bool Async_SendTo(const std::string& ip, unsigned int port, CIOBuffer* buffer) = 0;
		// 异步接收任意长度消息
		virtual bool Async_RecvSomeFrom(CIOBuffer* buffer) = 0;
		//--end

		// 本地ip和端口
		virtual std::string GetLocalIP(void) = 0;
		virtual unsigned int GetLocalPort(void) = 0;

		//
		virtual unsigned int GetSocketID(void) = 0;
	};

};

