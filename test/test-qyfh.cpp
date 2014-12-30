
#include "test-qyfh.h"

#ifdef _PLATFORM_WINDOWS_
#ifdef _DEBUG
#pragma comment( lib, "../bins/Debug/xh-net.lib" )
#pragma comment( lib, "luajit/lib/lua51.lib" )
#else
#pragma comment( lib, "../bins/Release/xh-net.lib" )
#pragma comment( lib, "luajit/lib/lua51.lib" )
#endif
#endif

#if defined _PLATFORM_WINDOWS_
#include <windows.h>
#endif


CTestNetIO* CTestNetIO::GetNetIO()
{
	static CTestNetIO netio;

	return &netio;
}

CTestNetIO::CTestNetIO()
	:m_netio(m_mainio.GetIOServer())
{
	m_netio.SetCallback_Server(
		bind(&CTestNetIO::listener_can_accept, this, placeholders::_1, placeholders::_2)
		, bind(&CTestNetIO::server_on_status, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4)
		, bind(&CTestNetIO::server_on_message, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5)
		, bind(&CTestNetIO::server_on_heart, this, placeholders::_1, placeholders::_2, placeholders::_3)
		);

	m_netio.SetCallback_Client(
		bind(&CTestNetIO::client_on_status, this, placeholders::_1, placeholders::_2, placeholders::_3)
		, bind(&CTestNetIO::client_on_message, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4)
		, bind(&CTestNetIO::client_on_heart, this, placeholders::_1, placeholders::_2)
		);

	m_luaengine.init(0);
}

CTestNetIO::~CTestNetIO()
{
	m_luaengine.clean();
}

std::string CTestNetIO::GetConnectIP()
{
	return m_netio.GetPeerIP(m_connecter);
}

unsigned int CTestNetIO::GetConnectPort()
{
	return m_netio.GetPeerPort(m_connecter);
}

void CTestNetIO::Start(const std::string& listenip, unsigned int listenport, const std::string& connectip, unsigned int connectport)
{
	m_listener = m_netio.Listen(listenip, listenport);
	m_connecter = m_netio.Connect(connectip, connectport);

	m_netio.Run();
	m_mainio.Run();

	bserver_open = true;
}

void CTestNetIO::Close()
{
	m_netio.Close(m_connecter);
	m_netio.Close(m_listener);
}

void CTestNetIO::Stop()
{
	m_netio.Stop();
	m_mainio.Stop();

	m_netio.WaitForFinish();
	m_mainio.WaitForFinish();

	bserver_open = false;
}

bool CTestNetIO::listener_can_accept(int userid, unsigned int socketid)
{
	return true;
}

void CTestNetIO::server_on_status(int userid, unsigned int socketid, unsigned int listener, bool bconnect)
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

void CTestNetIO::server_on_message(int userid, unsigned int socketid, unsigned int listener, const CMessageHead_24& head, CIOBuffer* recvmsg)
{
	XH_LOG_INFO(logname_base, "server recvmsg:" << socketid);
}

void CTestNetIO::server_on_heart(int userid, unsigned int socketid, unsigned int listener)
{
	XH_LOG_INFO(logname_base, "server onheart:" << socketid);
}

void CTestNetIO::client_on_status(int userid, unsigned int socketid, bool bconnect)
{
	if (bconnect)
	{
		XH_LOG_INFO(logname_base, "client connected:" << socketid);

		std::string luas = "local netio=CTestNetIO:GetNetIO();print(netio:GetConnectIP());";
		CTestNetIO::GetNetIO()->m_luaengine.executeScriptString(luas.c_str());
	}
	else
	{
		XH_LOG_INFO(logname_base, "client closed:" << socketid);
	}
}

void CTestNetIO::client_on_message(int userid, unsigned int socketid, const CMessageHead_24& head, CIOBuffer* recvmsg)
{
	XH_LOG_INFO(logname_base, "client recvmsg:" << socketid);
}

void CTestNetIO::client_on_heart(int userid, unsigned int socketid)
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

	CTestNetIO* netio = CTestNetIO::GetNetIO();;

	std::string ip = "10.225.10.39";// "127.0.0.1";
	unsigned int port = 5000;

	netio->Start(ip, port, ip, port);

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

	netio->Close();

	do
	{
		cmd = getchar();

		XH_LOG_INFO(logname_base, "--------------------");
		XH_LOG_INFO(logname_base, "listen leftcount:" << sc_listener_socket_count);
		XH_LOG_INFO(logname_base, "accepter leftcount:" << sc_server_socket_count);
		XH_LOG_INFO(logname_base, "connecter leftcount:" << sc_client_socket_count);
		XH_LOG_INFO(logname_base, "--------------------");
	} while (cmd != 'q');

	netio->Stop();

	getchar();

	XH_LOG_INFO(logname_base, "--------------------");
	XH_LOG_INFO(logname_base, "listen leftcount:" << sc_listener_socket_count);
	XH_LOG_INFO(logname_base, "accepter leftcount:" << sc_server_socket_count);
	XH_LOG_INFO(logname_base, "connecter leftcount:" << sc_client_socket_count);
	XH_LOG_INFO(logname_base, "--------------------");

	return 0;
}
