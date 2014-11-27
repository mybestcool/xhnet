
#pragma once

#include "iobuffer.h"

// 
// io下所有对象的内存管理机制 都会计数方式
// 

namespace xhnet
{
	typedef std::function<void()> postio;

	//
	// run 只能在一个线程中运行 one thread one run
	//
	class IIOServer : virtual public CPPRef
	{
	public:
		// 初始化进行二段式初始化
		static IIOServer* Create(void);

		static std::vector<std::string> Resolve_DNS(const std::string& hostname);
	public:
		virtual ~IIOServer(void) {  }

		virtual bool Init(void) = 0;
		virtual void Fini(void) = 0;

		virtual void Run(void) = 0;
		virtual bool IsFinshed(void) = 0;

		virtual void Post(postio io) = 0;
	};
};
