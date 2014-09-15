
#pragma once


#include <exception/traceback.h>


// 
#define XH_ASSERT(expr)			SMART_ASSERT(expr)
#define XH_VERIFY(expr)			SMART_VERIFY(expr)


#define XH_OPEN_CETRANS()		xhnet::open_exception_translator()
#define XH_OPEN_DUMP()			xhnet::open_dump()

