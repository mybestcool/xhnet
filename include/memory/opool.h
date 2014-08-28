
#pragma once

#include <mutex>
#include "mpagemgr.h"

namespace xhnet
{
	// 
	// object pool 
	// this class is adapter class
	// 
	template<class T, class M>
	class COPool
	{
	public:
		COPool(void) 
			: m_pool(sizeof(T), __alignof(T))
		{
			m_pool.Set_AllPages(&m_all_pages);
			m_pool.Set_FreePages(&m_free_pages);
			m_pool.Set_UsingMap(&m_using_pages);
		}
		~COPool(void) 
		{
			m_using_pages.Clear(false);
			m_free_pages.Clear(false);
			m_all_pages.Clear(true);
		}

		T* Allocate(void)
		{
			std::lock_guard<M> guard(m_mutex);
			return static_cast<T*>(m_pool.Allocate());
		}

		void Free(T* ptr)
		{
			std::lock_guard<M> guard(m_mutex);
			m_pool.Free(ptr);
		}


		T* New_Object(void)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T());

			return 0;
		}

		template<class P0>
		T* New_Object(const P0& p0)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0));

			return 0;
		}

		template<class P0>
		T* New_Object(P0& p0)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0));

			return 0;
		}

		template<class P0, class P1>
		T* New_Object(const P0& p0, const P1& p1)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1));

			return 0;
		}

		template<class P0, class P1>
		T* New_Object(P0& p0, P1& p1)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1));

			return 0;
		}

		template<class P0, class P1, class P2>
		T* New_Object(const P0& p0, const P1& p1, const P2& p2)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2));

			return 0;
		}

		template<class P0, class P1, class P2>
		T* New_Object(P0& p0, P1& p1, P2& p2)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2));

			return 0;
		}

		template<class P0, class P1, class P2, class P3>
		T* New_Object(const P0& p0, const P1& p1, const P2& p2, const P3& p3)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3));

			return 0;
		}

		template<class P0, class P1, class P2, class P3>
		T* New_Object(P0& p0, P1& p1, P2& p2, P3& p3)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3));

			return 0;
		}

		template<class P0, class P1, class P2, class P3, class P4>
		T* New_Object(const P0& p0, const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4));

			return 0;
		}

		template<class P0, class P1, class P2, class P3, class P4>
		T* New_Object(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4));

			return 0;
		}

		template<class P0, class P1, class P2, class P3, class P4, class P5>
		T* New_Object(const P0& p0, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4, p5));

			return 0;
		}

		template<class P0, class P1, class P2, class P3, class P4, class P5>
		T* New_Object(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4, p5));

			return 0;
		}

		void Delete_Object(T* object)
		{
			if (object)
			{
				(object)->~T();
				Free_To_Pool(object);
			}
		}

		void GetStat(unsigned int& all, unsigned int& used, unsigned int &free)
		{
			m_pool.GetMemStat(all, used, free);
		}

	private:
		CPageSet			m_all_pages;
		CPageList			m_free_pages;
		CPagesMap<void*, CMPage> m_using_pages;

		CSameSizeMPageMgr	m_pool;

		M					m_mutex;
	public:
		COPool(const COPool&) = delete;
		COPool& operator=(const COPool&) = delete;
	};


	template<class T, class M>
	class CInheritOPool
	{
	public:
		CInheritOPool(void)
		{

		}

		virtual ~CInheritOPool(void)
		{

		}

		static void* operator new(size_t alloclength)
		{
			assert(sizeof(T) == alloclength);
			return m_pool.Allocate();
		}

		static void operator delete(void* deletepointer)
		{
			m_pool.Free((T*)(deletepointer));
		}

		static void GetStat(unsigned int& all, unsigned int& used, unsigned int &free)
		{
			m_pool.GetStat(all, used, free);
		}

	private:
		static void* operator new[](size_t alloclength);
		static void operator delete[](void* deletepointer);

		static COPool<T, M>	m_pool;

	public:
		CInheritOPool(const CInheritOPool&) = delete;
		CInheritOPool& operator=(const CInheritOPool&) = delete;
	};

	template<class T, class M>
	COPool<T,M> CInheritOPool<T,M>::m_pool;
};

