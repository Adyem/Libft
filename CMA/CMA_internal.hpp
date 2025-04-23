#ifndef CMA_INTERNAL_HPP
# define CMA_INTERNAL_HPP

#include "../PThread/mutex.hpp"
#include <cstdint>
#include <stdint.h>

#define PAGE_SIZE 131072
#define BYPASS_ALLOC DEBUG
#define MAGIC_NUMBER 0xDEADBEEF

#define OFFSWITCH 0

#define SIZE 100
#define SMALL_SIZE (SIZE)
#define MEDIUM_SIZE (SIZE * 10)

#define BASE_SIZE 1024
#define SMALL_ALLOC (BASE_SIZE * 1)
#define MEDIUM_ALLOC (BASE_SIZE * 10)

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
	std::size_t	size;
    bool		free;
    Block		*next;
    Block		*prev;
} __attribute__ ((aligned(16)));

struct Page
{
    void		*start;
	std::size_t	size;
    Page		*next;
    Page		*prev;
    Block		*blocks;
	bool		heap;
	int8_t		alloc_size_type;	
} __attribute__ ((aligned(16)));

extern Page *page_list;

Block	*split_block(Block *block, std::size_t size);
Page	*create_page(std::size_t size);
Block	*find_free_block(std::size_t size);
Block	*merge_block(Block *block);
void	print_block_info(Block *block);

inline __attribute__((always_inline, hot)) std::size_t align16(size_t size)
{
    return (size + 15) & ~15;
}

#endif
