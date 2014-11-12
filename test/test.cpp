

#include <xh.h>

#if defined _PLATFORM_WINDOWS_
#include <windows.h>
#endif

#include <thread>
#include <mutex>
using namespace std;
using namespace xhnet;

#define _RANDOM_TEST_

class CAccepter;
class CConnecter;

IIOServer* server = 0;
ITcpSocket* ls = 0;

// socketid -> CAccepter
map<unsigned int, CAccepter*> accepters;

// socket -> CConnecter
mutex	mutex_connectors;
map<unsigned int, CConnecter*> connectors;

int listen_opentimes = 0;
int listen_closetimes = 0;

int accepter_opentimes = 0;
int accepter_closetimes = 0;

int connecter_opentimes = 0;
int connecter_closetimes = 0;

class CAccepter : public ICBTcpAccepter
{
public:
	ITcpSocket* m_socket;

public:
	CAccepter(ITcpSocket* a)
		: m_socket( a )
	{
		accepter_opentimes++;
	}
	virtual ~CAccepter() 
	{ 
		if (m_socket)
		{
			m_socket->Release();
		}
		accepter_closetimes++;
	}

	virtual void On_Connect(unsigned int socketid, int errid)
	{
		if ( errid )
		{
			XH_LOG_ERROR(logname_base, "accept failed err:" << errid);
		}
		else
		{
			XH_LOG_INFO(logname_base, "accept succeed socketid:"<<socketid);

			{
				char* cbuff = new char[1024];
				CIOBuffer* buff = new CIOBuffer(cbuff, 1024);
				buff->Set_Recycle_Function([](CIOBuffer* inbuf){delete[] inbuf->GetBuffer(); delete inbuf; });

				m_socket->Async_RecvSome(buff);

				buff->Release();
			}

#ifdef _RANDOM_TEST_
			{
				char* cbuff = new char[1024];
				CIOBuffer* buff = new CIOBuffer(cbuff, 1024);
				*buff << 10;
				buff->Set_Recycle_Function([](CIOBuffer* inbuf){delete[] inbuf->GetBuffer(); delete inbuf; });

				m_socket->Async_Send(buff);

				buff->Release();
			}

			if (GenRandUINT(0, 10) == 0)
			{
				m_socket->Fini();
			}
#endif // _RANDOM_TEST_
		}
	}

	virtual void On_Send(unsigned int socketid, int errid, CIOBuffer* buffer)
	{
		if (errid)
		{
			XH_LOG_ERROR(logname_base, "accept send err:" << errid << "socketid:" << socketid);
		}
		else
		{
			XH_LOG_INFO(logname_base, "accept send msg socketid:" << socketid);

			if (GenRandUINT(0, 10) == 0)
			{
				m_socket->Fini();
			}
		}
	}

	virtual void On_Recv(unsigned int socketid, int errid, CIOBuffer* buffer)
	{
		if ( errid )
		{
			XH_LOG_ERROR(logname_base, "accept recv err:" << errid << "socketid:" << socketid);
		}
		else
		{
			XH_LOG_INFO(logname_base, "accept recv msg socketid:" << socketid << ", msg:" << buffer->GetCurRead()
				<< ", msglen:" << buffer->AvailRead());

			{
				char* cbuff = new char[1024];
				CIOBuffer* buff = new CIOBuffer(cbuff, 1024);
				buff->Set_Recycle_Function([](CIOBuffer* inbuf){delete[] inbuf->GetBuffer(); delete inbuf; });

				m_socket->Async_RecvSome(buff);

				buff->Release();
			}

#ifdef _RANDOM_TEST_
			{
				char* cbuff = new char[1024];
				CIOBuffer* buff = new CIOBuffer(cbuff, 1024);
				*buff << 10;
				buff->Set_Recycle_Function([](CIOBuffer* inbuf){delete[] inbuf->GetBuffer(); delete inbuf; });

				m_socket->Async_Send(buff);

				buff->Release();
			}

			if (GenRandUINT(0, 10) == 0)
			{
				m_socket->Fini();
			}
#endif // _RANDOM_TEST_
		}
	}

	virtual void On_Close(unsigned int socketid, int errid)
	{
		XH_LOG_INFO(logname_base, "accept closed socketid:" << socketid << "err:" << errid);

		m_socket->Fini();

		auto it = accepters.find(socketid);
		if (it != accepters.end())
		{
			it->second->Release();
			accepters.erase(it);
		}
	}
};

class CListener : public ICBTcpListener
{
public:
	CListener()
	{
		listen_opentimes++;
	}
	virtual ~CListener() 
	{
		listen_closetimes++;
	}

	virtual void On_Listen(unsigned int socketid, int errid)
	{

	}

	virtual void On_Accept(unsigned int socketid, ITcpSocket* socket)
	{
		// check
		CAccepter* cb = new CAccepter(socket);
		socket->Init_Accepter(server, cb);
		accepters[socket->GetSocketID()] = cb;
	}

	virtual void On_Close(unsigned int socketid, int errid)
	{
		XH_LOG_ERROR(logname_base, "listen socket closed err:" << errid);
		if (ls)
		{
			ls->Fini();
		}
	}
};





class CConnecter : public ICBTcpConneter
{
public:
	ITcpSocket* m_socket;

public:
	CConnecter(ITcpSocket* a)
		: m_socket(a)
	{
		connecter_opentimes++;
	}
	virtual ~CConnecter() 
	{ 
		if ( m_socket )
		{
			m_socket->Release();
		}

		connecter_closetimes++;
	}

	virtual void On_Connect(unsigned int socketid, int errid)
	{
		if (errid)
		{
			XH_LOG_ERROR(logname_base, "connecter failed err:" << errid);
		}
		else
		{
			XH_LOG_INFO(logname_base, "connecter succeed socketid:" << socketid);

			{
				char* cbuff = new char[1024];
				CIOBuffer* buff = new CIOBuffer(cbuff, 1024);
				buff->Set_Recycle_Function([](CIOBuffer* inbuf){delete[] inbuf->GetBuffer(); delete inbuf; });

				m_socket->Async_RecvSome(buff);

				buff->Release();
			}

#ifdef _RANDOM_TEST_
			{
				char* cbuff = new char[1024];
				CIOBuffer* buff = new CIOBuffer(cbuff, 1024);
				*buff << 10;
				buff->Set_Recycle_Function([](CIOBuffer* inbuf){delete[] inbuf->GetBuffer(); delete inbuf; });

				m_socket->Async_Send(buff);

				buff->Release();
			}

			if (GenRandUINT(0, 10) == 0)
			{
				m_socket->Fini();
			}
#endif // _RANDOM_TEST_
		}
	}

	virtual void On_Send(unsigned int socketid, int errid, CIOBuffer* buffer)
	{
		if (errid)
		{
			XH_LOG_ERROR(logname_base, "connecter send err:" << errid << "socketid:" << socketid);
		}
		else
		{
			XH_LOG_INFO(logname_base, "connecter send msg socketid:" << socketid);

			if (GenRandUINT(0, 10) == 0)
			{
				m_socket->Fini();
			}
		}
	}

	virtual void On_Recv(unsigned int socketid, int errid, CIOBuffer* buffer)
	{
		if (errid)
		{
			XH_LOG_ERROR(logname_base, "connecter recv err:" << errid << "socketid:" << socketid);
		}
		else
		{
			XH_LOG_INFO(logname_base, "connecter recv msg socketid:" << socketid << ", msg:" << buffer->GetCurRead());

			{
				char* cbuff = new char[1024];
				CIOBuffer* buff = new CIOBuffer(cbuff, 1024);
				buff->Set_Recycle_Function([](CIOBuffer* inbuf){delete[] inbuf->GetBuffer(); delete inbuf; });

				m_socket->Async_RecvSome(buff);

				buff->Release();
			}


#ifdef _RANDOM_TEST_
			{
				char* cbuff = new char[1024];
				CIOBuffer* buff = new CIOBuffer(cbuff, 1024);
				*buff << 10;
				buff->Set_Recycle_Function([](CIOBuffer* inbuf){delete[] inbuf->GetBuffer(); delete inbuf; });

				m_socket->Async_Send(buff);

				buff->Release();
			}

			if (GenRandUINT(0, 10) == 0)
			{
				m_socket->Fini();
			}
#endif // _RANDOM_TEST_
		}
	}

	virtual void On_Close(unsigned int socketid, int errid)
	{
		XH_LOG_INFO(logname_base, "connecter closed socketid:" << socketid << "err:" << errid);

		// 连接失败 重连
		if (GenRandUINT(0, 10) != 0)
		{
			if (m_socket->Connect("127.0.0.1", 5555))
			{
				return;
			}
			else
			{
				XH_LOG_ERROR(logname_base, "connecter reconnect failed, socketid:" << socketid);
			}
		}

		{
			lock_guard<mutex> guard(mutex_connectors);

			auto it = connectors.find(socketid);
			if (it != connectors.end())
			{
				it->second->Release();
				connectors.erase(it);
			}
		}
		m_socket->Fini();
	}
};

void connect_thread(void)
{
#ifdef _RANDOM_TEST_
	int total = 1000;// GenRandUINT(0, 10);
#else
	int total = 10;// GenRandUINT(0, 10);
#endif // _RANDOM_TEST_
	for (int i = 0; i < total; ++i)
	{
		ITcpSocket* connect = ITcpSocket::Create();
		CConnecter* cb = new CConnecter(connect);

		connect->Init_Connecter(server, cb);

		{
			lock_guard<mutex> guard(mutex_connectors);
			connectors[connect->GetSocketID()] = cb;
		}

		connect->Connect("127.0.0.1", 5555);
	}
}

int main( int argc, char** argv )
{
#if defined _PLATFORM_WINDOWS_
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cd = GetLargestConsoleWindowSize(h);
	cd.X = 1024;
	cd.Y = 8096;
	SetConsoleScreenBufferSize(h, cd);
#endif

	XH_OPEN_CONSOLELOGGER();
	XH_ADD_LOGGER(logname_base, "net", LOGLEVEL_ALL);
	//XH_ADD_LOGGER(logname_trace, "trace", LOGLEVEL_ALL);
	
	std::vector<std::string> out = IIOServer::Resolve_DNS("www.sanguosha.com");

	for (int i = 0; i < 1; ++i)
	{

		server = IIOServer::Create();
		ls = ITcpSocket::Create();

		server->Init();

		CListener lscb;
		ls->Init_Listener(server, &lscb);
		ls->Listen("0.0.0.0", 5555);


		thread runio([&]{server->Run(); });
		SleepMilliseconds(100);
		thread connectio(connect_thread);

		int cmd = 0;
		do
		{
			cmd = getchar();

			XH_LOG_INFO(logname_base, "--------------------");
			XH_LOG_INFO(logname_base, "listen opentimes:" << listen_opentimes << " closetimes:" << listen_closetimes);
			XH_LOG_INFO(logname_base, "accepter opentimes:" << accepter_opentimes << " closetimes:" << accepter_closetimes);
			XH_LOG_INFO(logname_base, "connecter opentimes:" << connecter_opentimes << " closetimes:" << connecter_closetimes);
			XH_LOG_INFO(logname_base, "--------------------");
		} while (cmd != 'q');
		
		server->Fini();

		SleepMilliseconds(1000);

		while (!server->IsFinshed())
		{
			;
		}

		ls->Release();
		server->Release();

		runio.join();
		connectio.join();
	}

	XH_LOG_INFO(logname_base, "--------------------");
	XH_LOG_INFO(logname_base, "listen opentimes:" << listen_opentimes << " closetimes:" << listen_closetimes);
	XH_LOG_INFO(logname_base, "accepter opentimes:" << accepter_opentimes << " closetimes:" << accepter_closetimes);
	XH_LOG_INFO(logname_base, "connecter opentimes:" << connecter_opentimes << " closetimes:" << connecter_closetimes);
	XH_LOG_INFO(logname_base, "--------------------");

	
	return 0;
}
