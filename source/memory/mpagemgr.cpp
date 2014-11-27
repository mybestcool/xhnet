
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
		for (page_list_it it = m_pages.begin(); it != m_pages.end(); ++it)
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
			for (page_list_it it = m_pages.begin(); it != m_pages.end(); ++it)
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
		for (page_set_it it = m_pages.begin(); it != m_pages.end(); ++it)
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
			for (page_set_it it = m_pages.begin(); it != m_pages.end(); ++it)
			{
				if (*it)
				{
					delete (*it);
				}
			}
		}

		m_pages.clear();
	}



	CSameSizeMPageMgr::CSameSizeMPageMgr(unsigned long block_size, unsigned long align_size, unsigned long recycle_size)
	{
		m_block_size	= CMPage::Calc_AlignedSize(block_size, align_size);
		m_align_size	= align_size;
		m_recycle_size	= recycle_size;

		assert(m_recycle_size > 0 && "recyle free page size <=0");

		m_all_pages = 0;

		m_cur_allocpage = 0;
	}

	CSameSizeMPageMgr::~CSameSizeMPageMgr(void)
	{
		m_using_pages.Clear(false);
		m_cur_allocpage = 0;

		m_recycle_size = DEFAULT_START_RECYCLE_SIZE;
	}

	void* CSameSizeMPageMgr::AllocateFromPage(CMPage*& allocfrompage)
	{
		assert(m_all_pages &&"has not init all pages");

		if (m_cur_allocpage&&m_cur_allocpage->Is_Empty())
		{
			m_cur_allocpage = 0;
		}

		if (!m_cur_allocpage)
		{
			m_cur_allocpage = m_using_pages.Find(
				std::bind(
					[](CMPage* page)
					{  
						if (!page) return false; 
						return !page->Is_Empty(); 
					}
					, std::placeholders::_1 )
				);

			if (!m_cur_allocpage)
			{
				m_cur_allocpage = m_free_pages.Pop();
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

		allocfrompage = m_cur_allocpage;

		return m_cur_allocpage->Allocate();
	}

	void CSameSizeMPageMgr::FreeToPage(void* ptr, CMPage* freetopage)
	{
		if (!freetopage)
		{
			assert(false && "not this allocate ptr");
			return;
		}
		freetopage->Free(ptr);

		// freetopage 的内容已经回收完整了
		if (!freetopage->Is_Intact())
		{
			return;
		}

		// 是当前的使用的page，则不回收
		if (m_cur_allocpage == freetopage)
		{
			return;
		}

		// 从使用队列里剔除
		m_using_pages.Erase(freetopage);

		// 如果需要释放到操作系统
		if (m_free_pages.Size() >= m_recycle_size)
		{
			m_all_pages->Erase(freetopage);
			delete freetopage;
		}
		else
		{
			m_free_pages.Push(freetopage);
		}
	}

	void CSameSizeMPageMgr::GetMemStat(unsigned long& all, unsigned long& used, unsigned long &free)
	{
		if (!m_all_pages) return;

		unsigned long myall = 0;
		unsigned long myused = 0;
		unsigned long myfree = 0;
		for (CPageSet::page_set_it it = m_all_pages->m_pages.begin(); it != m_all_pages->m_pages.end(); ++it)
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



	CMPageMgr::CMPageMgr(unsigned long recycle_size)
		:m_recycle_size( recycle_size )
	{

	}
		
	CMPageMgr::~CMPageMgr(void)
	{
		for (xh_page_map<unsigned long, CSameSizeMPageMgr*>::iterator it = m_using_pages.begin(); it != m_using_pages.end(); ++it)
		{
			if ( it->second )
			{
				delete it->second;
			}
		}
		m_using_pages.clear();

		m_all_pages.Clear(true);

		m_using_map.clear();
	}

	void* CMPageMgr::Allocate(unsigned long block_size, unsigned long align_size)
	{
		unsigned long aligned_block_size = CMPage::Calc_AlignedSize(block_size, align_size);

		CSameSizeMPageMgr* mgr = 0;
		xh_page_map<unsigned long, CSameSizeMPageMgr*>::iterator it = m_using_pages.find(aligned_block_size);
		if (it != m_using_pages.end() )
		{
			mgr = it->second;
		}
		
		if (!mgr)
		{
			mgr = new CSameSizeMPageMgr(block_size, align_size, m_recycle_size);
			if ( !mgr )
			{
				return 0;
			}

			mgr->Set_AllPages(&m_all_pages);
			m_using_pages[aligned_block_size] = mgr;
		}

		if ( !mgr )
		{
			return 0;
		}

		CMPage* allocfrompage = 0;
		void* allocated = mgr->AllocateFromPage(allocfrompage);
		if (allocated)
		{
			m_using_map[allocated] = UsingBlockInfo(mgr, allocfrompage);
		}

		return allocated;
	}

	void CMPageMgr::Free(void* ptr)
	{
		CSameSizeMPageMgr* mgr = 0;
		CMPage* page = 0;

		xh_page_map<void*, UsingBlockInfo>::iterator it = m_using_map.find(ptr);
		if (it != m_using_map.end())
		{
			mgr = it->second.ppool;
			page = it->second.ppage;

			m_using_map.erase(it);
		}

		if ( mgr && page )
		{
			mgr->FreeToPage(ptr, page);
		}
	}

	void CMPageMgr::GetMemStat(unsigned long& all, unsigned long& used, unsigned long &free)
	{
		unsigned long myall = 0;
		unsigned long myused = 0;
		unsigned long myfree = 0;
		for (CPageSet::page_set_it it = m_all_pages.m_pages.begin(); it != m_all_pages.m_pages.end(); ++it)
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
