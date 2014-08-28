
#pragma once

namespace xhnet
{
	class CNullMutex
	{
	public:
		CNullMutex(void)	{	}
		~CNullMutex(void)	{	}

		void lock(void)		{	}
		void unlock(void)	{	}
	};
};
