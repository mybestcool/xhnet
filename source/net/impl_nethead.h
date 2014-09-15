
#pragma once

#include <queue>
#include <unordered_map>
#include <xhhead.h>
#include <xhnet.h>
#include <xhutility.h>
#include <xhpool.h>

#if defined _PLATFORM_WINDOWS_
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <windows.h>
#elif defined _PLATFORM_LINUX_
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#define INVALID_SOCKET	int(~0)
#define SOCKET_ERROR	-1
#endif

#include <libevent/include/event.h>

namespace xhnet
{
	typedef ifclass<struct sockaddr_in6, struct sockaddr_in, (sizeof(struct sockaddr_in6)>sizeof(struct sockaddr_in)) > socketaddr;

	inline bool ipport2sockaddr(const std::string& inip, unsigned int inport, socketaddr* outaddr, int& outaddrlen)
	{
		int ret = -1;
		// ipv4
		if (SplitString(inip, ".").size() == 4)
		{
			char tempaddr[1024] = { 0 };
			evutil_snprintf(tempaddr, sizeof(tempaddr) - 1, "%s:%d", inip.c_str(), inport);

			int v4addrlen = sizeof(struct sockaddr_in);
			ret = evutil_parse_sockaddr_port(tempaddr, (struct sockaddr*)outaddr, &v4addrlen);
			outaddrlen = v4addrlen;
		}
		else if (SplitString(inip, ":").size() == 6)
		{
			char tempaddr[1024] = { 0 };
			evutil_snprintf(tempaddr, sizeof(tempaddr) - 1, "%s:%d", inip.c_str(), inport);

			int v6addrlen = sizeof(struct sockaddr_in6);
			ret = evutil_parse_sockaddr_port(tempaddr, (struct sockaddr*)outaddr, &v6addrlen);
			outaddrlen = v6addrlen;
		}

		return ret == 0;
	}

	inline bool sockaddr2ipport(socketaddr* inaddr, int inaddrlen, std::string& outip, unsigned int& outport)
	{
		struct sockaddr* tempaddr = (struct sockaddr*)(inaddr);
		if (tempaddr->sa_family == AF_INET)
		{
			struct sockaddr_in* v4addr = (struct sockaddr_in*)(tempaddr);

			char tempip[256] = { 0 };
			evutil_inet_ntop(v4addr->sin_family, &v4addr->sin_addr, tempip, sizeof(tempip));
			outip = tempip;

			outport = ntohs(v4addr->sin_port);

			return true;
		}
		else if (tempaddr->sa_family == AF_INET6)
		{
			struct sockaddr_in6* v6addr = (struct sockaddr_in6*)(tempaddr);

			char tempip[256] = { 0 };
			evutil_inet_ntop(v6addr->sin6_family, &v6addr->sin6_addr, tempip, sizeof(tempip));
			outip = "[";
			outip += tempip;
			outip += "]";

			outport = ntohs(v6addr->sin6_port);

			return true;
		}

		return false;
	}

#if defined _PLATFORM_WINDOWS_
	inline bool setnonblocking(evutil_socket_t socket)
	{
		return evutil_make_socket_nonblocking(socket) == 0;

		//u_long iMode = 1;
		//return ::ioctlsocket(socket, FIONBIO, &iMode) == NO_ERROR;
	}

	inline bool isnoblockerr()
	{
		unsigned long err = GetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			return true;
		}

		return false;
	}

	inline bool setreuseport(evutil_socket_t socket)
	{
		return evutil_make_listen_socket_reuseable(socket) == 0;

		//BOOL on = 1;
		//return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == NO_ERROR;
	}

	inline bool setlingerclose(evutil_socket_t socket)
	{
		LINGER optval;
		optval.l_onoff = 1;
		optval.l_linger = 0;
		return ::setsockopt(socket, SOL_SOCKET, SO_LINGER, (const char*)&optval, sizeof(LINGER)) == NO_ERROR;
	}

	inline bool setnodelay(evutil_socket_t socket)
	{
		BOOL on = 1;
		return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on)) == NO_ERROR;
	}

	inline bool setrecvbuffer(evutil_socket_t socket, int size)
	{
		return ::setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size)) == NO_ERROR;
	}

	inline bool setsendbuffer(evutil_socket_t socket, int size)
	{
		return ::setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size)) == NO_ERROR;
	}

	inline void closetcpsocket(evutil_socket_t socket)
	{
		evutil_closesocket(socket);
		//::shutdown(socket, SD_BOTH);
		//::closesocket(socket);
	}

#elif defined _PLATFORM_LINUX_
	inline bool setnonblocking(evutil_socket_t socket)
	{
		return evutil_make_socket_nonblocking(socket) == 0;

		//int flags = ::fcntl(socket, F_GETFL, 0);
		//if (::fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
		//{
		//	return false;
		//}

		//return true;
	}

	inline bool isnoblockerr()
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			return true;
		}

		return false;
	}

	inline bool setreuseport(evutil_socket_t socket)
	{
		return evutil_make_listen_socket_reuseable(socket) == 0;

		//int on = 1;
		//if (::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(int)) == -1)
		//{
		//	return false;
		//}

		//return true;
	}

	inline bool setlingerclose(evutil_socket_t socket)
	{
		struct linger optval;
		optval.l_onoff = 1;
		optval.l_linger = 0;
		if (::setsockopt(socket, SOL_SOCKET, SO_LINGER, &optval, sizeof(struct linger)) == -1)
		{
			return false;
		}

		return true;
	}

	inline bool setnodelay(evutil_socket_t socket)
	{
		int on = 1;
		if (::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on)) == -1)
		{
			return false;
		}

		return true;
	}

	inline bool setrecvbuffer(evutil_socket_t socket, int size)
	{
		if (::setsockopt(socket, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)
		{
			return false;
		}

		return true;
	}

	inline bool setsendbuffer(evutil_socket_t socket, int size)
	{
		if (::setsockopt(socket, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) == -1)
		{
			return false;
		}

		return true;
	}

	inline void closetcpsocket(evutil_socket_t socket)
	{
		evutil_closesocket(socket);
		//::shutdown(socket, SHUT_RDWR);
		//::close(m_socket);
	}
#endif
};


