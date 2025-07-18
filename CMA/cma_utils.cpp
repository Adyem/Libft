#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <csignal>
#include "CMA_internal.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Printf/printf.hpp"

Page *page_list = ft_nullptr;
pt_mutex g_malloc_mutex;

static size_t determine_page_size(size_t size)
{
	if (size < SMALL_SIZE)
		return (SMALL_ALLOC);
	else if (size < MEDIUM_SIZE)
		return (MEDIUM_ALLOC);
	return (size);
}

static void determine_page_use(Page *page)
{
	if (page->heap == false)
		page->alloc_size_type = 0;
	else if (page->size == SMALL_ALLOC)
		page->alloc_size_type = 0;
	else if (page->size == MEDIUM_ALLOC)
		page->alloc_size_type = 1;
	else
		page->alloc_size_type = 2;
	return ;
}

static int8_t determine_which_block_to_use(size_t size)
{
	if (size < SMALL_SIZE)
		return (0);
	else if (size < MEDIUM_SIZE)
		return (1);
	return (2);
}

static void *create_stack_block(void)
{
    static char memory_block[PAGE_SIZE];

    if (DEBUG == 1)
        pf_printf("allocating stack memory for CMA\n");
    return (memory_block);
}

Block* split_block(Block* block, size_t size)
{
    if (block->size <= size + sizeof(Block))
        return (block);
    Block* new_block = reinterpret_cast<Block*>(reinterpret_cast<char*>(block) + sizeof(Block)
			+ size);
    new_block->magic = MAGIC_NUMBER;
    new_block->size = block->size - size - sizeof(Block);
    new_block->free = true;
    new_block->next = block->next;
    new_block->prev = block;
    if (new_block->next)
        new_block->next->prev = new_block;
    block->next = new_block;
    block->size = size;
    return (block);
}

Page *create_page(size_t size)
{
    size_t page_size = determine_page_size(size);
    bool use_heap = true;

    if (page_list == ft_nullptr)
    {
        page_size = PAGE_SIZE;
        use_heap = false;
    }
    else
    {
        if (size + sizeof(Block) > determine_page_size(size))
            page_size = size + sizeof(Block);
    }
    void* ptr;
    if (use_heap)
    {
        ptr = std::malloc(page_size);
        if (!ptr)
            return (ft_nullptr);
    }
    else
    {
        ptr = create_stack_block();
        if (!ptr)
            return (ft_nullptr);
    }
    Page* page = static_cast<Page*>(std::malloc(sizeof(Page)));
    if (!page)
    {
        if (use_heap)
            std::free(ptr);
        return (ft_nullptr);
    }
    page->heap = use_heap;
    page->start = ptr;
    page->size = page_size;
    page->next = ft_nullptr;
    page->prev = ft_nullptr;
    page->blocks = static_cast<Block*>(ptr);
    page->blocks->magic = MAGIC_NUMBER;
    page->blocks->size = page_size - sizeof(Block);
    page->blocks->free = true;
    page->blocks->next = ft_nullptr;
    page->blocks->prev = ft_nullptr;
	determine_page_use(page);
    if (!page_list) {
        page_list = page;
    }
    else
    {
        page->next = page_list;
        page_list->prev = page;
        page_list = page;
    }
    return (page);
}

Block *find_free_block(size_t size)
{
    Page* cur_page = page_list;
	int8_t alloc_size_type = determine_which_block_to_use(size);
    while (cur_page)
    {
		if (cur_page->alloc_size_type != alloc_size_type)
		{
			cur_page = cur_page->next;
			continue ;
		}
        Block* cur_block = cur_page->blocks;
        while (cur_block)
        {
            if (cur_block->free && cur_block->size >= size)
                return (cur_block);
            cur_block = cur_block->next;
        }
        cur_page = cur_page->next;
    }
    return (ft_nullptr);
}

Block *merge_block(Block *block)
{
    if (block->next && block->next->free)
    {
        block->size += sizeof(Block) + block->next->size;
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
    }
    if (block->prev && block->prev->free)
    {
        block->prev->size += sizeof(Block) + block->size;
        block->prev->next = block->next;
        if (block->next)
            block->next->prev = block->prev;
        block = block->prev;
    }
    return (block);
}

static inline void print_block_info_impl(Block *block)
{
#ifdef _WIN32
    (void)block;
    return ;
#else
    if (!block)
    {
        pf_printf_fd(2, "Block pointer is NULL.\n");
        return ;
    }
    const char* free_status = block->free ? "Yes" : "No";
    pf_printf_fd(3, "---- Block Information ----\n");
    pf_printf_fd(2, "Address of Block: %p\n", static_cast<void *>(block));
    pf_printf_fd(2, "Magic Number: 0x%X\n", block->magic);
    pf_printf_fd(2, "Size: %zu bytes\n", block->size);
    pf_printf_fd(2, "Free: %s\n", free_status);
    pf_printf_fd(2, "Next Block: %p\n", static_cast<void*>(block->next));
    pf_printf_fd(2, "Previous Block: %p\n", static_cast<void*>(block->prev));
    pf_printf_fd(2, "---------------------------\n");
#endif
    return ;
}

void print_block_info(Block *block)
{
    print_block_info_impl(block);
    return ;
}
