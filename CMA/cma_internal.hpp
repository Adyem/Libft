#ifndef CMA_INTERNAL_HPP
# define CMA_INTERNAL_HPP

#include "../PThread/mutex.hpp"
#include "../Libft/libft.hpp"
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
extern bool g_cma_thread_safe;
extern ft_size_t    g_cma_alloc_limit;
extern ft_size_t    g_cma_allocation_count;
extern ft_size_t    g_cma_free_count;
extern ft_size_t    g_cma_current_bytes;
extern ft_size_t    g_cma_peak_bytes;

struct Block
{
    uint32_t    magic;
    ft_size_t    size;
    bool        free;
    Block        *next;
    Block        *prev;
} __attribute__ ((aligned(16)));

struct Page
{
    void        *start;
    ft_size_t    size;
    Page        *next;
    Page        *prev;
    Block        *blocks;
    bool        heap;
    int8_t        alloc_size_type;    
} __attribute__ ((aligned(16)));

extern Page *page_list;

Block    *split_block(Block *block, ft_size_t size);
Page    *create_page(ft_size_t size);
Block    *find_free_block(ft_size_t size);
Block    *merge_block(Block *block);
void    print_block_info(Block *block);
void    dump_block_bytes(Block *block);
Page    *find_page_of_block(Block *block);
void    free_page_if_empty(Page *page);
int     cma_backend_is_enabled(void) __attribute__ ((warn_unused_result));
int     cma_backend_owns_pointer(const void *memory_pointer)
            __attribute__ ((warn_unused_result));
void    *cma_backend_allocate(ft_size_t size)
            __attribute__ ((warn_unused_result, hot));
void    *cma_backend_reallocate(void *memory_pointer, ft_size_t size)
            __attribute__ ((warn_unused_result, hot));
void    cma_backend_deallocate(void *memory_pointer) __attribute__ ((hot));
void    *cma_backend_aligned_allocate(ft_size_t alignment, ft_size_t size)
            __attribute__ ((warn_unused_result, hot));
ft_size_t    cma_backend_block_size(const void *memory_pointer)
            __attribute__ ((warn_unused_result, hot));
int     cma_backend_checked_block_size(const void *memory_pointer,
            ft_size_t *block_size) __attribute__ ((warn_unused_result, hot));

inline __attribute__((always_inline, hot)) ft_size_t align16(ft_size_t size)
{
    return ((size + 15) & ~static_cast<ft_size_t>(15));
}

#endif
