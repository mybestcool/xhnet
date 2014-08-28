
#pragma once

#include <string>
#include "xhhead.h"


namespace xhnet
{
	class CStackTrace
	{
		const std::string	m_file;
		size_t				m_line;
		const std::string	m_func;
	public:
		CStackTrace(const char* file, unsigned int line, const char* func);
		~CStackTrace(void);

	private:
		void printstack(int indent, const char* action, const char* file, size_t line, const char* func);

		CStackTrace(const CStackTrace&) = delete;
		CStackTrace& operator=(const CStackTrace&) = delete;
	};
};

#if defined _PLATFORM_WINDOWS_
#define __STACK_TRACE_IN__()	xhnet::CStackTrace XH_ANONYMOUS_VARIABLE(trace)(__FILE__, __LINE__, __FUNCTION__)
#elif defined _PLATFORM_LINUX_
#define __STACK_TRACE_IN__()	xhnet::CStackTrace XH_ANONYMOUS_VARIABLE(trace)(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif
