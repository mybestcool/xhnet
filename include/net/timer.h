
#pragma once

#include "iobuffer.h"

namespace xhnet
{
	class IIOServer;


	enum timer_err
	{
		timer_ok = 0,
		timer_begin,
		timer_end,
	};

	// 所有回调都会在io线程内
	class ICBTimer : public CPPRef
	{
	public:
		virtual ~ICBTimer() { }

		virtual void On_Timer(unsigned int timerid, int errid) = 0;
	};

	class ITimer : public CPPRef
	{
	public:
		// 初始化进行二段式初始化
		// 创建，只创建了一个对象，
		static ITimer* Create(void);

	public:
		virtual ~ITimer(void) {  }

		// 初始化回调
		virtual bool Init(IIOServer* io, ICBTimer* cb) = 0;
		// 异步关闭socket，异步计数减-，当计数减到0的时候，自动销毁
		virtual void Fini(void) = 0;

		//--不线程安全，只能在io线程内调用
		// 设置每隔ms毫秒，则调用一次callback，函数不能重复调用，
		// 回调产生errid后不再回调
		virtual bool Async_Wait(unsigned int ms) = 0;
		//--end

		virtual unsigned int GetTimerID(void) = 0;
	};
};

