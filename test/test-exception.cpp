
#include "stdhead.h"
#include "xh.h"

#ifdef _PLATFORM_WINDOWS_
#ifdef _DEBUG
#pragma comment( lib, "../bins/Debug/xh-net.lib" )
#else
#pragma comment( lib, "../bins/Release/xh-net.lib" )
#endif
#endif


void testtraceback()
{
	xhnet::print_traceback();
}

void test_exception_0()
{
	testtraceback();
}

void test_exception_1()
{
	std::cout << "begin test_exception_1" << std::endl;
	try
	{
		int  div = 0;
		std::cout << "mid1 test_exception_1" << std::endl;
		int c = 100 / div;
		std::cout << "mid2 test_exception_1" << std::endl;
	}
	catch (...)
	{
		std::cout << "catch test_exception_1" << std::endl;
	}

	std::cout << "end test_exception_1" << std::endl;
}

void test_exception_2()
{
	std::cout << "begin test_exception_2" << std::endl;
	try
	{
		int  *div = 0;
		std::cout << "mid1 test_exception_2" << std::endl;
		*div = 10;
		std::cout << "mid2 test_exception_2" << std::endl;
	}
	catch (...)
	{
		std::cout << "catch test_exception_2" << std::endl;
	}

	std::cout << "end test_exception_2" << std::endl;
}

int main( int argc, char** argv )
{
	XH_OPEN_CONSOLELOGGER();
	XH_ADD_LOGGER(xhnet::logname_trace, "ccc", xhnet::LOGLEVEL_ALL);


	XH_OPEN_CETRANS();
	XH_OPEN_DUMP();

	test_exception_0();
	test_exception_1();
	test_exception_2();

	getchar();

	return 0;
}
