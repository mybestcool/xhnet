
#include "qyfh.h"

#ifdef _PLATFORM_WINDOWS_
#ifdef _DEBUG
#pragma comment( lib, "../bins/Debug/xh-net.lib" )
#else
#pragma comment( lib, "../bins/Release/xh-net.lib" )
#endif
#endif

#if defined _PLATFORM_WINDOWS_
#include <windows.h>
#endif

using namespace xhnet_qyfh;

bool listener_can_accept(int userid, unsigned int socketid)
{
	return true;
}

void server_on_status(int userid, unsigned int socketid, unsigned int listener, bool bconnect)
{
	if ( bconnect )
	{
		XH_LOG_INFO(logname_base, "server connected:" << socketid);
	}
	else
	{
		XH_LOG_INFO(logname_base, "server closed:" << socketid);
	}
}

void server_on_message(int userid, unsigned int socketid, unsigned int listener, const CMessageHead_24& head, CIOBuffer* recvmsg)
{
	XH_LOG_INFO(logname_base, "server recvmsg:" << socketid);
}

void server_on_heart(int userid, unsigned int socketid, unsigned int listener)
{
	XH_LOG_INFO(logname_base, "server onheart:" << socketid);
}

void client_on_status(int userid, unsigned int socketid, bool bconnect)
{
	if (bconnect)
	{
		XH_LOG_INFO(logname_base, "client connected:" << socketid);
	}
	else
	{
		XH_LOG_INFO(logname_base, "client closed:" << socketid);
	}
}

void client_on_message(int userid, unsigned int socketid, const CMessageHead_24& head, CIOBuffer* recvmsg)
{
	XH_LOG_INFO(logname_base, "client recvmsg:" << socketid);
}

void client_on_heart(int userid, unsigned int socketid)
{
	XH_LOG_INFO(logname_base, "client onheart:" << socketid);
}


int main(int argv, char** argc)
{
#if defined _PLATFORM_WINDOWS_
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cd = GetLargestConsoleWindowSize(h);
	cd.X = 1024;
	cd.Y = 8096;
	SetConsoleScreenBufferSize(h, cd);
#endif

	XH_OPEN_CONSOLELOGGER();
	XH_ADD_LOGGER(logname_trace, "trace", LOGLEVEL_ALL);
	XH_ADD_LOGGER(logname_base, "net", LOGLEVEL_ALL);

	CThreadIO* mainio = new CThreadIO();

	CNetIO<CMessageHead_24> netio(mainio->GetIOServer());

	netio.SetCallback_Server(listener_can_accept
		, server_on_status
		, server_on_message
		, server_on_heart);

	netio.SetCallback_Client(client_on_status
		, client_on_message
		, client_on_heart
		);

	std::string ip = "10.225.10.39";// "127.0.0.1";
	unsigned int port = 5000;

	unsigned int listener = netio.Listen(ip, port);
	unsigned int connecter = netio.Connect(ip, port);

	netio.Run();
	mainio->Run();

	int cmd = 0;

	do
	{
		cmd = getchar();

		XH_LOG_INFO(logname_base, "--------------------");
		XH_LOG_INFO(logname_base, "listen leftcount:" << sc_listener_socket_count);
		XH_LOG_INFO(logname_base, "accepter leftcount:" << sc_server_socket_count);
		XH_LOG_INFO(logname_base, "connecter leftcount:" << sc_client_socket_count);
		XH_LOG_INFO(logname_base, "--------------------");
	} while (cmd != 'q');

	netio.Close(connecter);
	netio.Close(listener);

	do
	{
		cmd = getchar();

		XH_LOG_INFO(logname_base, "--------------------");
		XH_LOG_INFO(logname_base, "listen leftcount:" << sc_listener_socket_count);
		XH_LOG_INFO(logname_base, "accepter leftcount:" << sc_server_socket_count);
		XH_LOG_INFO(logname_base, "connecter leftcount:" << sc_client_socket_count);
		XH_LOG_INFO(logname_base, "--------------------");
	} while (cmd != 'q');

	netio.Stop();
	mainio->Stop();

	netio.WaitForFinish();
	mainio->WaitForFinish();
	mainio->Release();
	mainio = 0;

	getchar();

	XH_LOG_INFO(logname_base, "--------------------");
	XH_LOG_INFO(logname_base, "listen leftcount:" << sc_listener_socket_count);
	XH_LOG_INFO(logname_base, "accepter leftcount:" << sc_server_socket_count);
	XH_LOG_INFO(logname_base, "connecter leftcount:" << sc_client_socket_count);
	XH_LOG_INFO(logname_base, "--------------------");

	return 0;
}
