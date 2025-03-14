#ifndef CMA_INTERNAL_HPP
# define CMA_INTERNAL_HPP

#include "../PThread/mutex.hpp"
#include <cstdint>
#include <cstddef>

#define PAGE_SIZE 131072
#define BYPASS_ALLOC DEBUG
#define MAGIC_NUMBER 0xDEADBEEF

#define OFFSWITCH 0

#ifndef DEBUG
# define DEBUG 0
#endif

#if __has_include(<valgrind/memcheck.h>)
#include <valgrind/memcheck.h>
#define PROTECT_METADATA(ptr, size) VALGRIND_MAKE_MEM_NOACCESS(ptr, size)
#define UNPROTECT_METADATA(ptr, size) VALGRIND_MAKE_MEM_DEFINED(ptr, size)
#else
#define PROTECT_METADATA(ptr, size) ((void)0)
#define UNPROTECT_METADATA(ptr, size) ((void)0)
#endif

extern pt_mutex g_malloc_mutex;

struct Block
{
    uint32_t	magic;
    size_t		size;
    bool		free;
    Block		*next;
    Block		*prev;
} __attribute__ ((aligned(8)));

struct Page
{
    void	*start;
    size_t	size;
    Page	*next;
    Page	*prev;
    Block	*blocks;
	bool	heap;
} __attribute__ ((aligned(8)));

extern Page *page_list;

Block	*split_block(Block *block, size_t size);
Page	*create_page(size_t size);
Block	*find_free_block(size_t size);
Block	*merge_block(Block *block);
void	print_block_info(Block *block);

inline size_t	align8(size_t size) __attribute__ ((always_inline, hot));

#endif
