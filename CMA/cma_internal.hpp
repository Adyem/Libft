#ifndef CMA_INTERNAL_HPP
# define CMA_INTERNAL_HPP

#include "../PThread/pthread.hpp"
#include "../Libft/libft.hpp"
#include <cstdint>
#include <stdint.h>

#define PAGE_SIZE 131072
#define BYPASS_ALLOC DEBUG
#define MAGIC_NUMBER_ALLOCATED 0xDEADBEEF
#define MAGIC_NUMBER_FREE 0xBEEFDEAD

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

extern ft_size_t    g_cma_alloc_limit;
extern ft_size_t    g_cma_allocation_count;
extern ft_size_t    g_cma_free_count;
extern ft_size_t    g_cma_current_bytes;
extern ft_size_t    g_cma_peak_bytes;

class cma_allocator_guard
{
    private:
        bool _lock_acquired;
        bool _active;
        mutable int _error_code;
        mutable bool _failure_logged;

        void set_error(int error) const;
        void log_inactive_guard(void *return_address) const;

    public:
        cma_allocator_guard();
        ~cma_allocator_guard();

        bool is_active() const;
        bool lock_acquired() const;
        void unlock();
        int get_error() const;
        const char *get_error_str() const;
};

struct Block
{
    uint32_t    magic;
    ft_size_t    size;
    bool        free;
    Block        *next;
    Block        *prev;
    unsigned char    *payload;
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
Page    *find_page_of_block(Block *block);
void    free_page_if_empty(Page *page);
void    cma_validate_block(Block *block, const char *context, void *user_pointer);
Block    *cma_find_block_for_pointer(const void *memory_pointer);
int     cma_lock_allocator(bool *lock_acquired);
void    cma_unlock_allocator(bool lock_acquired);
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
#ifndef CMA_ENABLE_METADATA_PROTECTION
# define CMA_ENABLE_METADATA_PROTECTION 0
#endif

int     cma_metadata_make_writable(void);
void    cma_metadata_make_inaccessible(void);
Block    *cma_metadata_allocate_block(void) __attribute__ ((warn_unused_result));
void    cma_metadata_release_block(Block *block);
void    cma_metadata_reset(void);

inline __attribute__((always_inline, hot)) ft_size_t align16(ft_size_t size)
{
    return ((size + 15) & ~static_cast<ft_size_t>(15));
}

inline __attribute__((always_inline, hot)) bool cma_block_is_free(const Block *block)
{
    if (!block)
        return (false);
    if (block->magic == MAGIC_NUMBER_FREE)
        return (true);
    return (false);
}

inline __attribute__((always_inline, hot)) void cma_mark_block_free(Block *block)
{
    if (!block)
        return ;
    block->free = true;
    block->magic = MAGIC_NUMBER_FREE;
    return ;
}

inline __attribute__((always_inline, hot)) void cma_mark_block_allocated(Block *block)
{
    if (!block)
        return ;
    block->free = false;
    block->magic = MAGIC_NUMBER_ALLOCATED;
    return ;
}

inline __attribute__((always_inline, hot)) void cma_update_block_magic(Block *block)
{
    if (!block)
        return ;
    if (cma_block_is_free(block))
    {
        cma_mark_block_free(block);
        return ;
    }
    cma_mark_block_allocated(block);
    return ;
}

#endif
