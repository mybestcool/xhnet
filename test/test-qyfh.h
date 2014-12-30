
#include "qyfh.h"
#include "luaengine.h"

using namespace xhnet_qyfh;

static bool bserver_open = false;

class CTestNetIO
{
public:
	CTestNetIO();
	~CTestNetIO();

	static CTestNetIO* GetNetIO();

	std::string GetConnectIP();

	unsigned int GetConnectPort();

	void Start(const std::string& listenip, unsigned int listenport, const std::string& connectip, unsigned int connectport);

	void Close();

	void Stop();


public:
	bool listener_can_accept(int userid, unsigned int socketid);

	void server_on_status(int userid, unsigned int socketid, unsigned int listener, bool bconnect);

	void server_on_message(int userid, unsigned int socketid, unsigned int listener, const CMessageHead_24& head, CIOBuffer* recvmsg);

	void server_on_heart(int userid, unsigned int socketid, unsigned int listener);


	void client_on_status(int userid, unsigned int socketid, bool bconnect);

	void client_on_message(int userid, unsigned int socketid, const CMessageHead_24& head, CIOBuffer* recvmsg);

	void client_on_heart(int userid, unsigned int socketid);

private:

	std::string		m_ip;
	unsigned int	m_port;

	unsigned int m_listener;

	unsigned int m_connecter;

	CThreadIO					m_mainio;
	CNetIO<CMessageHead_24>		m_netio;

public:
	CLuaEngine		m_luaengine;
};

