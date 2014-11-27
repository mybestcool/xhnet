
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
		}
		~COPool(void) 
		{
			m_using_map.Clear(false);
			m_all_pages.Clear(true);
		}

		T* Allocate(void)
		{
			std::lock_guard<M> guard(m_mutex);
			CMPage* allocfrompage = 0;
			void* allocated = m_pool.AllocateFromPage(allocfrompage);
			m_using_map.Push(allocated, allocfrompage);

			return static_cast<T*>(allocated);
		}

		void Free(T* ptr)
		{
			std::lock_guard<M> guard(m_mutex);

			CMPage* page = m_using_map.Pop(ptr);
			if (!page)
			{
				return;
			}

			m_pool.FreeToPage(ptr, page);
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

		template<class P0, class P1, class P2, class P3, class P4, class P5, class P6>
		T* New_Object(const P0& p0, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4, p5, p6));

			return 0;
		}

		template<class P0, class P1, class P2, class P3, class P4, class P5, class P6>
		T* New_Object(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4, p5, p6));

			return 0;
		}

		template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
		T* New_Object(const P0& p0, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4, p5, p6, p7));

			return 0;
		}

		template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
		T* New_Object(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4, p5, p6, p7));

			return 0;
		}

		template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
		T* New_Object(const P0& p0, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7, const P8& p8)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4, p5, p6, p7, p8));

			return 0;
		}

		template<class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
		T* New_Object(P0& p0, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7, P8& p8)
		{
			void* object = Allocate();

			if (object)	return (T*)(::new(object)T(p0, p1, p2, p3, p4, p5, p6, p7, p8));

			return 0;
		}

		void Delete_Object(T* object)
		{
			if (object)
			{
				(object)->~T();
				Free(object);
			}
		}

		void GetStat(unsigned long& all, unsigned long& used, unsigned long &free)
		{
			m_pool.GetMemStat(all, used, free);
		}

	private:
		CPageSet			m_all_pages;
		CPagesMap<void*>	m_using_map;

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
			//assert(sizeof(T) == alloclength);
			return m_pool.Allocate();
		}

		static void operator delete(void* deletepointer)
		{
			m_pool.Free((T*)(deletepointer));
		}

		static void GetStat(unsigned long& all, unsigned long& used, unsigned long &free)
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

