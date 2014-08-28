
#pragma once

#include <functional>
#include "xhhead.h"

namespace xhnet
{
	// สนำร
	// HANDLE h = CreateFile(...);
	// CScopeGuard onExit([&] { CloseHandle(h); });
	// CScopeGuard onExit( std::bind(&CloseHandle, h) );
	// 
	class CScopeGuard
	{
	public:
		typedef std::function<void()> exitscope_fun;

		explicit CScopeGuard(exitscope_fun exitscope)
			: m_exitscope(exitscope), m_dismissed(false)
		{ 
		}

		~CScopeGuard()
		{
			if ( !m_dismissed )
			{
				m_exitscope();
			}
		}

		void Dismiss()
		{
			m_dismissed = true;
		}

	private:
		exitscope_fun	m_exitscope;
		bool			m_dismissed;

	public: // noncopyable
		CScopeGuard(CScopeGuard const&) = delete;
		CScopeGuard& operator=(CScopeGuard const&) = delete;
	};

};

#define XH_GUARD(...)	xhnet::CScopeGuard XH_ANONYMOUS_VARIABLE(scopeguard)(__VA_ARGS__)
