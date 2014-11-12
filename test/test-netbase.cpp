
#include <xh.h>
#include <thread>

#include "../source/net/impl_nethead.h"

using namespace std;

class CTest : public xhnet::CPPRef
{
public:
	CTest() { }
	~CTest() {}
};

CTest* test = new CTest();

void threadaddfun()
{
	test->Retain();
}

void threadsubfun()
{
	test->Release();
}

int main( int argc, char** argv )
{
	// ip v4 test
	{
		xhnet::socketaddr addr;
		ev_socklen_t addrlen = 0;
		bool bret = xhnet::ipport2sockaddr("127.0.0.1", 5555, &addr, addrlen);

		std::string ip;
		unsigned int port = 0;
		bret = xhnet::sockaddr2ipport(&addr, addrlen, ip, port);
	}

	// ip v6 test
	{
		xhnet::socketaddr addr;
		ev_socklen_t addrlen = 0;
		bool bret = xhnet::ipport2sockaddr("[fe80::4c00:bbad:a546:fbf6]", 5555, &addr, addrlen);

		std::string ip;
		unsigned int port = 0;
		bret = xhnet::sockaddr2ipport(&addr, addrlen, ip, port);
	}

	
	// io buffer test
	{
		std::map<int, int> testmap;
		std::vector<int> testvt;
		std::set<int> testset;

		for (int i = 0; i < 10; ++i)
		{
			testmap.insert(std::make_pair(i, i));
			testvt.push_back(i);
			testset.insert(i);
		}

		char buffer[1024];
		xhnet::CIOBuffer iob(buffer, sizeof(buffer));

		iob << testmap << testvt << testset;

		std::map<int, int> testmap2;
		std::vector<int> testvt2;
		std::set<int> testset2;

		iob >> testmap2 >> testvt2 >> testset2;
	}

	// cppref test
	vector<shared_ptr<thread> > addthreads;
	vector<shared_ptr<thread> > subthreads;

	int cnt = 100;
	for (int i = 0; i < cnt; ++i)
	{
		addthreads.push_back(shared_ptr<thread>(new thread(threadaddfun)));
	}

	for (int i = 0; i < cnt; ++i)
	{
		subthreads.push_back(shared_ptr<thread>(new thread(threadsubfun)));
	}

	for (int i = 0; i < cnt; ++i)
	{
		addthreads[i]->join();
		subthreads[i]->join();
	}

	addthreads.clear();
	subthreads.clear();

	test->Release();

	getchar();

	return 0;
}
