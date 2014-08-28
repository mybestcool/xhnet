
#pragma once


#include <set>
#include <map>
//#include <hash_map>
//#include <ext/hash_map>
#include <list>
#include <functional>

#include "mpage.h"

// 提高效率可以使用hash系列
#define xh_page_set		std::set
#define xh_page_map		std::map
//#define xh_page_map		stdext::hash_map
//#define xh_page_map		__gnu_cxx::hash_map
#define xh_page_list	std::list

namespace xhnet
{
	class CPageList
	{
	public:
		CPageList();
		~CPageList();

		void Push(CMPage* page);
		CMPage* Pop();
		CMPage* Find(std::function<bool(CMPage*)> fun);
		void Clear(bool bdelete=false);
		size_t Size() { return m_pages.size(); }

	public:
		xh_page_list<CMPage*> m_pages;

	public:
		CPageList(const CPageList&) = delete;
		CPageList& operator=(const CPageList&) = delete;
	};

	class CPageSet
	{
	public:
		CPageSet();
		~CPageSet();

		void Insert(CMPage* page);
		void Erase(CMPage* page);
		CMPage* Find(std::function<bool(CMPage*)> fun);
		void Clear(bool bdelete = false);
		size_t Size() { return m_pages.size(); }

	public:
		xh_page_set<CMPage*> m_pages;

	public:
		CPageSet(const CPageSet&) = delete;
		CPageSet& operator=(const CPageSet&) = delete;
	};

	template<class T, class D>
	class CPagesMap
	{
	public:
		CPagesMap()
		{
		}
		~CPagesMap()
		{
			Clear();
		}

		void Push(T key, D* page)
		{
			m_pages[key] = page;
		}
		D* Find(T key)
		{
			xh_page_map<T, D*>::iterator it = m_pages.find(key);
			if (it == m_pages.end())
			{
				return 0;
			}

			return it->second;
		}
		D* Pop(T key)
		{
			xh_page_map<T, D*>::iterator it = m_pages.find(key);
			if (it == m_pages.end())
			{
				return 0;
			}

			D* page = it->second;
			m_pages.erase(it);
			return page;
		}
		void Clear(bool bdelete=false)
		{
			if (bdelete)
			{
				for (xh_page_map<T, D*>::iterator it = m_pages.begin(); it != m_pages.end(); ++it)
				{
					if (it->second)
					{
						delete (it->second);
					}
				}
			}

			m_pages.clear();
		}

	public:
		xh_page_map<T, D*> m_pages;

	public:
		CPagesMap(const CPagesMap&) = delete;
		CPagesMap& operator=(const CPagesMap&) = delete;
	};


	class CSameSizeMPageMgr
	{
	public:
		// 当空闲队列的长度大于DEFAULT_START_RECYCLE_SIZE，之后回收的内存全部返回给系统
		enum { DEFAULT_START_RECYCLE_SIZE = 10 };

		CSameSizeMPageMgr(unsigned int block_size, unsigned int align_size, unsigned int recycle_size = DEFAULT_START_RECYCLE_SIZE);
		~CSameSizeMPageMgr(void);

		void* Allocate(void);
		void Free(void* ptr);

		void GetMemStat(unsigned int& all, unsigned int& used, unsigned int &free);

		void Set_AllPages(CPageSet* pages)
		{
			m_all_pages = pages;
		}

		void Set_FreePages(CPageList* pages)
		{
			m_free_pages = pages;
		}

		void Set_UsingMap(CPagesMap<void*, CMPage>* usingmap)
		{
			m_using_map = usingmap;
		}

		unsigned int Get_Block_Size(void)
		{
			return m_block_size;
		}

		unsigned int Get_Align_Size(void)
		{
			return m_align_size;
		}
	private:
		unsigned int		m_block_size;
		unsigned int		m_align_size;
		
		unsigned int		m_recycle_size;

		// 所有的chunk
		CPageSet*			m_all_pages;

		// page大小一样的一个page
		CPageList*			m_free_pages;

		CPageSet			m_using_pages;
		CMPage*				m_cur_allocpage;

		CPagesMap<void*, CMPage>*	m_using_map;

	public:
		CSameSizeMPageMgr(void) = delete;
		CSameSizeMPageMgr(const CSameSizeMPageMgr&) = delete;
		CSameSizeMPageMgr& operator=(const CSameSizeMPageMgr&) = delete;
	};

	class CMPageMgr
	{
	public:
		CMPageMgr(unsigned int recycle_size = CSameSizeMPageMgr::DEFAULT_START_RECYCLE_SIZE);
		~CMPageMgr(void);

		void* Allocate(unsigned int block_size, unsigned int align_size);
		void Free(void* ptr);

		void GetMemStat(unsigned int& all, unsigned int& used, unsigned int &free);

	private:
		unsigned int								m_recycle_size;

		CPageSet									m_all_pages;
		
		// pagesize -> pagelist
		CPagesMap<unsigned int, CPageList>			m_free_pages;

		// alignedblocksize->mgr
		CPagesMap<unsigned int, CSameSizeMPageMgr>	m_using_pages;

		//
		CPagesMap<void*, CMPage>					m_using_map;

	public:
		CMPageMgr(const CMPageMgr&) = delete;
		CMPageMgr& operator=(const CMPageMgr&) = delete;
	};
};
