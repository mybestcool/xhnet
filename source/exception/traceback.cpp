/// @license MIT license http://opensource.org/licenses/MIT
/// @author David Siroky <siroky@dasir.cz>
/// @file

#include <stdio.h>
#include <xhhead.h>
#include <xhlog.h>
#include <exception/traceback.h>
#include <xhutility.h>
//##########################################################################
//##########################################################################

#if defined _PLATFORM_WINDOWS_

#include "windows/StackWalker.h"
#include <eh.h>
#include <tchar.h>
#include <time.h>
#include <dbghelp.h>
#include <string>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

#ifdef UNICODE
#define tstring wstring 
#else
#define tstring string 
#endif


class StackWalkerToLog : public StackWalker
{
protected:
  // do not print modules initialization
  virtual void OnLoadModule(LPCSTR, LPCSTR, DWORD64, DWORD, DWORD, LPCSTR, LPCSTR, ULONGLONG) {}
  // do not print symbols initialization
  virtual void OnSymInit(LPCSTR, DWORD, LPCSTR) {}
  virtual void OnOutput(LPCSTR szText) 
  {
	  XH_LOG_TRACE(xhnet::logname_trace, szText);
  }
};

namespace xhnet
{
	void print_traceback()
	{
		StackWalkerToLog sw; sw.ShowCallstack();
	}

	class CCPlusPlusException
	{
	public:
		CCPlusPlusException(unsigned int errid, EXCEPTION_POINTERS* pExp)
		{
			XH_LOG_TRACE(xhnet::logname_trace, "#####--beg stack err:"<<errid<<"--#####");
			StackWalkerToLog sw;
			sw.ShowCallstack(GetCurrentThread(), pExp->ContextRecord);
			//print_traceback();
			XH_LOG_TRACE(xhnet::logname_trace, "#####--end stack err:"<<errid<<"--#####");
		}

		~CCPlusPlusException(void)
		{

		}
	};

	void trans_func(unsigned int code, EXCEPTION_POINTERS* pExp)
	{
		throw CCPlusPlusException(code, pExp);
	}

	void open_exception_translator()
	{
		::_set_se_translator(trans_func);
	}

	class CMiniFileDump
	{
	public:
		CMiniFileDump(void)
		{
			m_OriginalFilter = ::SetUnhandledExceptionFilter(ExceptionFilter);
		}

		~CMiniFileDump(void)
		{
			::SetUnhandledExceptionFilter(m_OriginalFilter);
		}

	private:
		LPTOP_LEVEL_EXCEPTION_FILTER m_OriginalFilter;

		static LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
		{
			TCHAR szPath[MAX_PATH];

			if (GetModuleFileName(NULL, szPath, sizeof(szPath)))
			{
				struct tm nowtm = xhnet::GetNowTM();

				TCHAR filename[MAX_PATH];
				_stprintf_s(filename, sizeof(filename)-1, _T("_%4d-%02d-%02d_%02d-%02d-%02d.dmp"), nowtm.tm_year + 1900,
					nowtm.tm_mon + 1, nowtm.tm_mday, nowtm.tm_hour, nowtm.tm_min,
					nowtm.tm_sec);

				TCHAR drive[_MAX_DRIVE];
				TCHAR dir[_MAX_DIR];
				TCHAR fname[_MAX_FNAME];
				TCHAR ext[_MAX_EXT];
				_tsplitpath_s(szPath, drive, dir, fname, ext);

				std::tstring strDumpFileName = drive;
				strDumpFileName += dir;
				strDumpFileName += fname;
				strDumpFileName += filename;

				HANDLE hFile = CreateFile(strDumpFileName.c_str(), FILE_ALL_ACCESS,
					0, NULL, CREATE_ALWAYS, NULL, NULL);

				if (hFile != INVALID_HANDLE_VALUE)
				{
					MINIDUMP_EXCEPTION_INFORMATION exception_information;
					exception_information.ThreadId = GetCurrentThreadId();
					exception_information.ExceptionPointers = ExceptionInfo;
					exception_information.ClientPointers = TRUE;

					MINIDUMP_TYPE MiniDumpWithDataSegs = (MINIDUMP_TYPE)(MiniDumpNormal
						| MiniDumpWithHandleData
						| MiniDumpWithUnloadedModules
						| MiniDumpWithIndirectlyReferencedMemory
						| MiniDumpScanMemory
						| MiniDumpWithProcessThreadData
						| MiniDumpWithThreadInfo);

					//MINIDUMP_TYPE MiniDumpWithDataSegs = (MINIDUMP_TYPE)
					//	(MiniDumpWithPrivateReadWriteMemory |
					//	MiniDumpWithDataSegs |
					//	MiniDumpWithHandleData |
					//	MiniDumpWithFullMemory |
					//	MiniDumpWithThreadInfo |
					//	MiniDumpWithUnloadedModules);

					MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
						hFile,MiniDumpWithDataSegs, &exception_information, NULL, NULL);
					CloseHandle(hFile);
				}
			}

			return EXCEPTION_EXECUTE_HANDLER;
		}
	};

	void open_dump()
	{
		static CMiniFileDump dump;
	}

	// 如果某些异常无法捕获，可以使用detours
	/*#include "../detours/detours.h"

	LONG WINAPI NewUnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo){
	OutputDebugString(L"NewUnhandledExceptionFilter\n");

	CreateDump(ExceptionInfo);
	return EXCEPTION_EXECUTE_HANDLER;
	}

	CAutoDump::CAutoDump(void)
	{
	m_lpUnhandledExceptionFilter = NULL;
	do {
	SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	m_lpUnhandledExceptionFilter = DetourFindFunction("KERNEL32.DLL", "UnhandledExceptionFilter");

	if (NULL == m_lpUnhandledExceptionFilter) {
	break;
	}
	LONG lRes = NO_ERROR;
	lRes = DetourTransactionBegin();
	if (NO_ERROR != lRes) {
	break;
	}

	lRes = DetourAttach(&m_lpUnhandledExceptionFilter, NewUnhandledExceptionFilter);
	if (NO_ERROR != lRes) {
	break;
	}

	lRes = DetourTransactionCommit();
	if (NO_ERROR != lRes) {
	break;
	}
	} while (0);
	}


	CAutoDump::~CAutoDump(void)
	{
	if (m_lpUnhandledExceptionFilter) {
	do {
	LONG lRes = NO_ERROR;
	lRes = DetourTransactionBegin();
	if (NO_ERROR != lRes) {
	break;
	}

	lRes = DetourDetach(&m_lpUnhandledExceptionFilter, NewUnhandledExceptionFilter);
	if (NO_ERROR != lRes) {
	break;
	}

	lRes = DetourTransactionCommit();
	if (NO_ERROR != lRes) {
	break;
	}
	} while (0);
	}
	}*/
};

//##########################################################################
//##########################################################################

#elif defined _PLATFORM_LINUX_

#include <execinfo.h>
#include <stdlib.h>
#include <cxxabi.h>
#include <signal.h>
#include <string.h>

//#include <cxxabi.h>
//#include <libunwind.h>

namespace xhnet
{
	void print_traceback()
	{
		// storage array for stack trace address data
		void* addrlist[100];

		// retrieve current stack addresses
		int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

		if (addrlen == 0) {
			XH_LOG_TRACE(xhnet::logname_trace, "  <empty, possibly corrupt>");
			return;
		}

		// buffer
		char tempbuff[10240] = { 0 };
		unsigned int usedbuff = 0;

		// resolve addresses into strings containing "filename(function+address)",
		// this array must be free()-ed
		char** symbollist = backtrace_symbols(addrlist, addrlen);

		// allocate string which will be filled with the demangled function name
		size_t funcnamesize = 256;
		char* funcname = (char*)malloc(funcnamesize);

		// iterate over the returned symbol lines. skip the first, it is the
		// address of this function.
		for (int i = 1; i < addrlen; i++)
		{
			char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

			// find parentheses and +address offset surrounding the mangled name:
			// ./module(function+0x15c) [0x8048a6d]
			for (char *p = symbollist[i]; *p; ++p)
			{
				if (*p == '(')
					begin_name = p;
				else if (*p == '+')
					begin_offset = p;
				else if (*p == ')' && begin_offset) {
					end_offset = p;
					break;
				}
			}

			if (begin_name && begin_offset && end_offset
				&& begin_name < begin_offset)
			{
				*begin_name++ = '\0';
				*begin_offset++ = '\0';
				*end_offset = '\0';

				// mangled name is now in [begin_name, begin_offset) and caller
				// offset in [begin_offset, end_offset). now apply
				// __cxa_demangle():

				int status;
				char* ret = abi::__cxa_demangle(begin_name,
					funcname, &funcnamesize, &status);
				if (status == 0) {
					funcname = ret; // use possibly realloc()-ed string

					usedbuff = strlen(tempbuff);
					snprintf(tempbuff+usedbuff, sizeof(tempbuff)-1-usedbuff, "  %s : %s+%s\n",
						symbollist[i], funcname, begin_offset );
				}
				else {
					// demangling failed. Output function name as a C function with
					// no arguments.
					usedbuff = strlen(tempbuff);
					snprintf(tempbuff+usedbuff, sizeof(tempbuff)-1-usedbuff, "  %s : %s()+%s\n",
						symbollist[i], begin_name, begin_offset );
				}
			}
			else
			{
				// couldn't parse the line? print the whole line.
				usedbuff = strlen(tempbuff);
				snprintf(tempbuff+usedbuff, sizeof(tempbuff)-1-usedbuff, "  %s\n", symbollist[i] );
			}
		}

		free(funcname);
		free(symbollist);

		if ( strlen(tempbuff)>0 )
		{
			XH_LOG_TRACE(xhnet::logname_trace, tempbuff);
		}
	}

	//void print_traceback2()
	//{
	//	unw_cursor_t cursor;
	//	unw_context_t context;

	//	unw_getcontext(&context);
	//	unw_init_local(&cursor, &context);

	//	// buffer
	//	char tempbuff[10240] = { 0 };
	//	unsigned int usedbuff = 0;

	//	while (unw_step(&cursor) > 0) {
	//		unw_word_t offset, pc;
	//		char fname[200];
	//		size_t demangledSize = 200;
	//		char* demangled = (char*) malloc(demangledSize);

	//		unw_get_reg(&cursor, UNW_REG_IP, &pc);
	//		fname[0] = '\0';
	//		unw_get_proc_name(&cursor, fname, sizeof(fname), &offset);

	//		int status;

	//		char *ret = abi::__cxa_demangle(fname, demangled, &demangledSize, &status);
	//		if (ret) {
	//			// return value may be a realloc() of the input
	//			demangled = ret;
	//		}
	//		else {
	//			// demangling failed, just pretend it's a C demangled with no args
	//			strncpy(demangled, fname, demangledSize);
	//			strncat(demangled, "()", demangledSize);
	//			demangled[demangledSize-1] = '\0';
	//		}

	//		usedbuff = strlen(tempbuff);
	//		snprintf(tempbuff+usedbuff, sizeof(tempbuff)-1-usedbuff, "%s+0x%lx [%lx]\n", demangled, offset, pc );

	//		free(demangled);
	//	}
	//	
	//	XH_LOG_TRACE(xhnet::logname_trace, tempbuff);
	//}

	class CCPlusPlusException : public std::exception
	{
	public:
		CCPlusPlusException( int sig )
		{
			XH_LOG_TRACE(xhnet::logname_trace, "#####--beg stack err:"<<sig<<"--#####");
			// 需要加-rdynamic才会打印出函数，不然用gdb list *地址 来得到异常的地方
			xhnet::print_traceback();
			XH_LOG_TRACE(xhnet::logname_trace, "#####--end stack err:"<<sig<<"--#####");
		}

		~CCPlusPlusException(void)
		{

		}
	};

	void abort_handler( int sig )
	{
		throw CCPlusPlusException( sig ); 
	}

	void open_exception_translator()
	{
		static bool binit = false;
		if ( !binit )
		{
			binit = true;

			signal( SIGINT, abort_handler );
			signal( SIGQUIT, abort_handler );

			signal( SIGBUS,  abort_handler );
			signal( SIGSEGV, abort_handler );
			
			signal( SIGFPE,  abort_handler );

			signal( SIGPIPE,  NULL );
		}
	}

	void open_dump()
	{
		XH_LOG_TRACE(xhnet::logname_trace, "make sure use shell cmd befor exe start:ulimit -c 1024000");
	}
};

#endif


