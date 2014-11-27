
#pragma once

#include "qyfhhead.h"

namespace xhnet_qyfh
{
	class CThreadIO : virtual public CPPRef
	{
	public:
		CThreadIO()
		{
			m_io = IIOServer::Create();

			XH_ASSERT(m_io)(m_io);
		}

		virtual ~CThreadIO()
		{
			if ( m_io )
			{
				m_io->Release();
				m_io = 0;
			}
		}

		IIOServer* GetIOServer()
		{
			return m_io;
		}

		bool Run()
		{
			if (m_thread)
			{
				return false;
			}

			if (!m_io->Init())
			{
				return false;
			}

			m_thread = shared_ptr<thread>(new thread([&]{m_io->Run(); }));

			return true;
		}

		void Stop()
		{
			if ( m_io )
			{
				m_io->Fini();
			}
		}

		void WaitForFinish()
		{
			if (m_thread)
			{
				m_thread->join();
			}
		}

	private:
		shared_ptr<thread>	m_thread;
		IIOServer*			m_io;
	};
};
