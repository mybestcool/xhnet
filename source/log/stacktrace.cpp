
#include <stdio.h>

#include "log/stacktrace.h"
#include "xhlog.h"

namespace xhnet
{
#if defined _PLATFORM_WINDOWS_
#define LOCAL_THREAD_VAR	__declspec(thread)
#elif defined _PLATFORM_LINUX_
#define LOCAL_THREAD_VAR	__thread
#endif
	static LOCAL_THREAD_VAR int	_ltv_indent = 0;

	CStackTrace::CStackTrace(const char* file, unsigned int line, const char* func)
		: m_file(file), m_line(line), m_func(func)
	{
		printstack(_ltv_indent, "->", m_file.c_str(), m_line, m_func.c_str());
		_ltv_indent++;
	}

	CStackTrace::~CStackTrace(void)
	{
		_ltv_indent--;
		printstack(_ltv_indent, "<-", m_file.c_str(), m_line, m_func.c_str());
	}

	void CStackTrace::printstack(int indent, const char* action, const char* file, size_t line, const char* func)
	{
		char	buffer[1024];
		char*	p = buffer;
		size_t	remain = sizeof(buffer);

		while (indent-- > 0)
		{
			int n = _snprintf_s(p, remain, remain-1, "  ");
			p += n;
			remain -= n;
		}

#if defined _PLATFORM_WINDOWS_
		_snprintf_s(p, remain, remain-1, "%s %s", action, func);
#elif defined _PLATFORM_LINUX_
		snprintf(p, remain-1, "%s %s", action, func);
#endif

		XH_LOG_TRACE(logname_trace, buffer);
	}
};
