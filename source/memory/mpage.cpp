
#include <cassert>
#include <memory>
#include <stdlib.h>
#include <assert.h>

#include "memory/mpage.h"

namespace xhnet
{
	CMPage::CMPage(unsigned long block_size, unsigned long align_size)
		: m_memory(0), m_freeblock_head(0), m_freeblock_num(0)
		, m_allblock_num(0), m_page_size(0)
		, m_align_size(0), m_block_size(0)
	{
		if (block_size < 1)	block_size = 1;
		if (align_size < 1) align_size = 1;

		create(block_size, align_size);
	}

	CMPage::~CMPage(void)
	{
		destory();
	}

	bool CMPage::Reset(unsigned long block_size, unsigned long align_size)
	{
		if (block_size < 0)
			return false;

		if (align_size < 0)
			return false;

		if (!Is_Intact())
			return false;

		if ( m_block_size==block_size && m_align_size==align_size )
		{
			return true;
		}

		create(block_size, align_size);
		return true;
	}

	void CMPage::Clear(void)
	{
		if (!Is_Empty())
		{
			m_freeblock_num = m_allblock_num;

			assert(isaligned(m_block, m_align_size) && "memory not aligned");

			m_freeblock_head = (FreeBlock*)m_block;
			m_freeblock_head->next = 0;
		}
	}

	void* CMPage::Allocate(void)
	{
		// 分配实例对象  
		return allocateblock();
	}

	void CMPage::Free(void* ptr)
	{
		// 检测Pool是否分配以及待释放指针是否为空, 以防止发生错误  
		if (!ptr) return;

		if (Is_ContainsPointer(ptr) && isaligned(ptr, m_align_size))
		{
			// 将释放的内存还给Pool  
			freeblock((FreeBlock*)ptr);
		}
		else
		{
			assert(false && "object not allocated from this pool");
		}
	}

	unsigned long CMPage::Calc_AlignedSize(unsigned long block_size, unsigned long align_size)
	{
		// 内存块的大小要满足内存对齐的要求, 这样才能使CPU寻址最快.  
		// __alignof(T)是为了检测T对齐的粒度, 因为用户可以指定对齐的粒度,  
		// 所以不可以Hard Code, 在早期的STL内存配置器中, 对齐粒度固定为8.  
		// 但是如果用户指定对齐为4, 那么就会出现错误, 这里的动态检测是亮点  
		//  
		// 举个例子, 假设sizeof(T) = 11, __alignof(T) = 4  
		// 那么 blockSize = 11  
		//      diff = 11 % 4 = 3  
		// 因为 diff != 0, 所以  
		//      blockSize += 11 + 4 - 3 = 12  
		// 这样就满足内存对齐的要求了, 很不错的算法  
		//  
		// 这里有一个问题, 如果sizeof(T)比一个指针要小, 那么会浪费内存  
		unsigned long aligned_blocksize = block_size;
		unsigned long diff = aligned_blocksize % align_size;
		if (diff != 0) aligned_blocksize += align_size - diff;

		// 注意: 如果分配的blockSize比一个指针还小, 那么就至少要分配一个指针的大小  
		if (aligned_blocksize < sizeof(FreeBlock*)) aligned_blocksize = sizeof(FreeBlock*);

		return aligned_blocksize;
	}

	unsigned long CMPage::Calc_PageSize(unsigned long block_size, unsigned long align_size)
	{
		unsigned long aligned_block_size = Calc_AlignedSize(block_size, align_size);
		unsigned long maxfilledsize = align_size - 1;

		unsigned long pagesize = DEFAULT_PAGESIZE;

		if (aligned_block_size + maxfilledsize > DEFAULT_PAGESIZE)
		{
			pagesize = aligned_block_size + maxfilledsize;
		}

		return pagesize;
	}

	void CMPage::create(unsigned long block_size, unsigned long align_size)
	{
		unsigned long src_pagesize = Get_PageSize();

		// 得到对齐后的一个block的内存大小，每个block必须大于sizeof(FreeBlock*)
		m_block_size	= Calc_AlignedSize(block_size, align_size);
		m_align_size	= align_size;

		// 首先是维护FreeBlock指针占用的内存大小  
		//unsigned long pointersize	= sizeof(FreeBlock*);
		unsigned long maxfilledsize = m_align_size - 1;

		if (m_block_size + maxfilledsize > DEFAULT_PAGESIZE)
		{
			m_page_size		= m_block_size + maxfilledsize;
			m_allblock_num	= 1;
		}
		else
		{
			m_page_size		= DEFAULT_PAGESIZE;
			m_allblock_num	= (m_page_size - maxfilledsize) / m_block_size;
		}

		m_freeblock_num = m_allblock_num;

		// 如果原来的跟现在的内存一样大小 就不再分配
		if (m_memory || src_pagesize != m_page_size)
		{
			if (m_memory)
			{
				::free(m_memory);
			}

			m_memory = ::malloc(m_page_size);
		}

		m_block = getaligned(m_memory, m_align_size);

		// 检测分配内存是否满足内存对齐条件,
		assert(isaligned(m_block, m_align_size) && "memory not aligned");

		// 将FreeBlock链表头设置为分配的值  
		m_freeblock_head = (FreeBlock*)m_block;
		m_freeblock_head->next = 0;
	}

	void CMPage::destory(void)
	{
		// 释放操作很简单了, 参见上图  
		if (m_memory)
		{
			::free(m_memory);
			m_memory = 0;
		}
	}

	CMPage::FreeBlock* CMPage::allocateblock()
	{
		if (m_freeblock_num == 0)
		{
			return 0;
		}

		// 分配block是一个O(1)的算法，链表头始终是空闲节点  
		FreeBlock* block = m_freeblock_head;

		// 这里维护的是空闲节点的数目, 即Pool的剩余容量  
		if (--m_freeblock_num != 0)
		{
			if (m_freeblock_head->next == NULL)
			{
				// If the block has not been previously allocated its next pointer  
				// will be NULL so just update the list head to the next block in the pool   
				m_freeblock_head = (FreeBlock*)(cast2uint(m_freeblock_head) + m_block_size);
				m_freeblock_head->next = NULL;
			}
			else
			{
				// The block has been previously allocated and freed so it   
				// has a valid link to the next free block  
				m_freeblock_head = m_freeblock_head->next;
			}
		}
		else
		{
			m_freeblock_head = 0;
		}

		return block;
	}

	void CMPage::freeblock(FreeBlock* block)
	{
		// 将内存归还到链表头  
		if (m_freeblock_num > 0) block->next = m_freeblock_head;
		m_freeblock_head = block;

		// 维护空闲节点数目  
		m_freeblock_num++;
	}
};

