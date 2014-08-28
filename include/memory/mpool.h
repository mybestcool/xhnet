
#pragma once

#include <mutex>
#include "mpagemgr.h"

namespace xhnet
{
	// 
	// memory pool 
	// this class is adapter class
	// 
	template<class M>
	class CMPool
	{
	public:
		CMPool(void) { }
		~CMPool(void) { }

		// 最好不要allocate 大于60k的内存
		void* Allocate(unsigned int size, unsigned int align_size=CMPage::DEFAULT_ALIGNSIZE)
		{
			std::lock_guard<M> guard(m_mutex);
			return m_pool.Allocate(size, align_size);
		}

		template<class T>
		T* Allocate(void)
		{
			std::lock_guard<M> guard(m_mutex);
			//return (T*)(m_pool.Allocate( sizeof(T), __alignof(T) ));
			return static_cast<T*>(m_pool.Allocate(sizeof(T), __alignof(T)));
		}

		void Free(void* ptr)
		{
			std::lock_guard<M> guard(m_mutex);
			m_pool.Free(ptr);
		}

		void GetStat(unsigned int& all, unsigned int& used, unsigned int &free)
		{
			m_pool.GetMemStat(all, used, free);
		}

	private:
		CMPageMgr	m_pool;
		M			m_mutex;
	public:
		CMPool(const CMPool&) = delete;
		CMPool& operator=(const CMPool&) = delete;
	};
};

