
//#include "../include/stdhead.h"
//#include "../include/xhguard.h"
#include "xhlog.h"

#include "xhexception.h"

void testtraceback()
{
	xhnet::print_traceback();
}

int main( int argc, char** argv )
{
	XH_OPEN_CONSOLELOGGER();
	XH_ADD_LOGGER(xhnet::logname_trace, "ccc", xhnet::LOGLEVEL_ALL);



	XH_OPEN_CETRANS();
	//XH_OPEN_DUMP();
	//testtraceback();

	try
	{
		int  div = 0;
		//*div = 10;
		int c = 100 / div;
	}
	catch (...)
	{
		;
	}

	getchar();

	return 0;
}
