
#pragma once

//#define SMART_ASSERT_DEBUG_MODE	1
#include <exception/smart_assert.h>

namespace xhnet
{
	class CInsureInit
	{
	public:
		CInsureInit();
		~CInsureInit();

	private:

		CInsureInit(const CInsureInit&) = delete;
		CInsureInit& operator=(const CInsureInit&) = delete;
	};
};


