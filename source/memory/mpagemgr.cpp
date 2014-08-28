
#include <cassert>
#include "memory/mpagemgr.h"

namespace xhnet
{
	CPageList::CPageList()
	{

	}

	CPageList::~CPageList()
	{
		Clear();
	}

	void CPageList::Push(CMPage* page)
	{
		m_pages.push_back(page);
	}

	CMPage* CPageList::Pop()
	{
		if (m_pages.empty())
		{
			return 0;
		}

		CMPage* page = m_pages.front();
		m_pages.pop_front();

		return page;
	}

	CMPage* CPageList::Find(std::function<bool(CMPage*)> fun)
	{
		for (xh_page_list<CMPage*>::iterator it = m_pages.begin(); it != m_pages.end(); ++it)
		{
			if (fun(*it))
			{
				return *it;
			}
		}

		return 0;
	}

	void CPageList::Clear(bool bdelete)
	{
		if (bdelete)
		{
			for (xh_page_list<CMPage*>::iterator it = m_pages.begin(); it != m_pages.end(); ++it)
			{
				if (*it)
				{
					delete (*it);
				}
			}
		}

		m_pages.clear();
	}




	CPageSet::CPageSet()
	{

	}

	CPageSet::~CPageSet()
	{
		Clear();
	}

	void CPageSet::Insert(CMPage* page)
	{
		m_pages.insert(page);
	}

	void CPageSet::Erase(CMPage* page)
	{
		m_pages.erase(page);
	}

	CMPage* CPageSet::Find(std::function<bool(CMPage*)> fun)
	{
		for (xh_page_set<CMPage*>::iterator it = m_pages.begin(); it != m_pages.end(); ++it)
		{
			if (fun(*it))
			{
				return *it;
			}
		}

		return 0;
	}

	void CPageSet::Clear(bool bdelete)
	{
		if (bdelete)
		{
			for (xh_page_set<CMPage*>::iterator it = m_pages.begin(); it != m_pages.end(); ++it)
			{
				if (*it)
				{
					delete (*it);
				}
			}
		}

		m_pages.clear();
	}



	CSameSizeMPageMgr::CSameSizeMPageMgr(unsigned int block_size, unsigned int align_size, unsigned int recycle_size)
	{
		m_block_size	= CMPage::Calc_AlignedSize(block_size, align_size);
		m_align_size	= align_size;
		m_recycle_size	= recycle_size;

		assert(m_recycle_size > 0 && "recyle free page size <=0");

		m_all_pages = 0;
		m_free_pages = 0;
		m_using_map = 0;

		m_cur_allocpage = 0;
	}

	CSameSizeMPageMgr::~CSameSizeMPageMgr(void)
	{
		m_using_pages.Clear(false);
		m_cur_allocpage = 0;

		m_recycle_size = DEFAULT_START_RECYCLE_SIZE;
	}

	void* CSameSizeMPageMgr::Allocate(void)
	{
		assert(m_all_pages &&"has not init all pages");
		assert(m_free_pages &&"has not init  free pages");
		assert(m_using_map &&"has not init using map");

		if (m_cur_allocpage&&m_cur_allocpage->Is_Empty())
		{
			m_cur_allocpage = 0;
		}

		if (!m_cur_allocpage)
		{
			m_cur_allocpage = m_using_pages.Find([](CMPage* page){  if (!page) return false; return !page->Is_Empty(); });

			if (!m_cur_allocpage)
			{
				m_cur_allocpage = m_free_pages->Pop();
				if (m_cur_allocpage)
				{
					if (m_cur_allocpage->Reset(m_block_size, m_align_size))
					{
						m_using_pages.Insert(m_cur_allocpage);
					}
					else
					{
						m_cur_allocpage = 0;
						assert(false);
					}
				}
			}

			if (!m_cur_allocpage)
			{
				m_cur_allocpage = new CMPage(m_block_size, m_align_size);
				m_using_pages.Insert(m_cur_allocpage);
				m_all_pages->Insert(m_cur_allocpage);
			}
		}

		if (!m_cur_allocpage)
		{
			return 0;
		}

		void* allocated = m_cur_allocpage->Allocate();
		if (allocated)
		{
			m_using_map->Push(allocated, m_cur_allocpage);
		}

		return allocated;
	}

	void CSameSizeMPageMgr::Free(void* ptr)
	{
		CMPage* needfreepage = m_using_map->Pop(ptr);

		if (!needfreepage)
		{
			assert(false && "not this allocate ptr");
			return;
		}
		needfreepage->Free(ptr);

		// needfreepage 的内容已经回收完整了
		if (!needfreepage->Is_Intact())
		{
			return;
		}

		// 是当前的使用的page，则不回收
		if (m_cur_allocpage == needfreepage)
		{
			return;
		}

		// 从使用队列里剔除
		m_using_pages.Erase(needfreepage);

		// 如果需要释放到操作系统
		if (m_free_pages->Size() >= m_recycle_size)
		{
			m_all_pages->Erase(needfreepage);
			delete needfreepage;
		}
		else
		{
			m_free_pages->Push(needfreepage);
		}
	}

	void CSameSizeMPageMgr::GetMemStat(unsigned int& all, unsigned int& used, unsigned int &free)
	{
		if (!m_all_pages) return;

		unsigned int myall = 0;
		unsigned int myused = 0;
		unsigned int myfree = 0;
		for (xh_page_set<CMPage*>::iterator it = m_all_pages->m_pages.begin(); it != m_all_pages->m_pages.end(); ++it)
		{
			CMPage* page = *it;
			if (page)
			{
				myall += page->Get_PageSize();
				myused += (page->Get_AllBlockNum() - page->Get_FreeBlockNum())*page->Get_BlockSize();
				myfree += page->Get_FreeBlockNum()*page->Get_BlockSize();
			}
		}

		all += myall;
		used += myused;
		free += myfree;
	}



	CMPageMgr::CMPageMgr(unsigned int recycle_size)
		:m_recycle_size( recycle_size )
	{

	}
		
	CMPageMgr::~CMPageMgr(void)
	{
		m_using_pages.Clear(true);

		m_free_pages.Clear(true);
		m_all_pages.Clear(true);

		m_using_map.Clear(false);
	}

	void* CMPageMgr::Allocate(unsigned int block_size, unsigned int align_size)
	{
		unsigned int aligned_block_size = CMPage::Calc_AlignedSize(block_size, align_size);

		CSameSizeMPageMgr* mgr = m_using_pages.Find(aligned_block_size);
		if (!mgr)
		{
			unsigned int page_size = CMPage::Calc_PageSize(block_size, align_size, aligned_block_size);

			CPageList* freepages = m_free_pages.Find(page_size);
			if ( !freepages )
			{
				freepages = new CPageList();
				if (!freepages)
				{
					return 0;
				}
				m_free_pages.Push(page_size, freepages);
			}

			mgr = new CSameSizeMPageMgr(block_size, align_size, m_recycle_size);
			if ( !mgr )
			{
				return 0;
			}

			mgr->Set_AllPages(&m_all_pages);
			mgr->Set_FreePages(freepages);
			mgr->Set_UsingMap(&m_using_map);

			m_using_pages.Push(aligned_block_size, mgr);
		}

		if ( !mgr )
		{
			return 0;
		}

		return mgr->Allocate();
	}

	void CMPageMgr::Free(void* ptr)
	{
		CMPage* page = m_using_map.Find(ptr);
		if ( !page )
		{
			return;
		}

		CSameSizeMPageMgr* mgr = m_using_pages.Find(page->Get_BlockSize());
		if ( mgr )
		{
			mgr->Free(ptr);
		}
	}

	void CMPageMgr::GetMemStat(unsigned int& all, unsigned int& used, unsigned int &free)
	{
		unsigned int myall = 0;
		unsigned int myused = 0;
		unsigned int myfree = 0;
		for (xh_page_set<CMPage*>::iterator it = m_all_pages.m_pages.begin(); it != m_all_pages.m_pages.end(); ++it)
		{
			CMPage* page = *it;
			if ( page )
			{
				myall += page->Get_PageSize();
				myused += (page->Get_AllBlockNum() - page->Get_FreeBlockNum())*page->Get_BlockSize();
				myfree += page->Get_FreeBlockNum()*page->Get_BlockSize();
			}
		}

		all += myall;
		used += myused;
		free += myfree;
	}
};
