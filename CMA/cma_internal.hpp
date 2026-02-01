#ifndef CMA_INTERNAL_HPP
# define CMA_INTERNAL_HPP

#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
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
extern unsigned long long    g_cma_metadata_access_depth;

void    cma_record_operation_error(int error_code);
void    cma_record_internal_operation_error(int error_code);

struct Block
{
    uint32_t            magic;
    ft_size_t           size;
    bool                free;
    Block               *next;
    Block               *prev;
    unsigned char       *payload;
#if DEBUG
    unsigned char       *debug_base_pointer;
    ft_size_t           debug_user_size;
#endif
} __attribute__ ((aligned(16)));

struct Page
{
    void                *start;
    ft_size_t           size;
    Page                *next;
    Page                *prev;
    Block               *blocks;
    bool                heap;
    int8_t              alloc_size_type;
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
int     cma_unlock_allocator(bool lock_acquired);
int     cma_backend_is_enabled(void) __attribute__ ((warn_unused_result));
int     cma_backend_owns_pointer(const void *memory_pointer)
            __attribute__ ((warn_unused_result));
void    *cma_backend_allocate(ft_size_t size, int *error_code)
            __attribute__ ((warn_unused_result, hot));
void    *cma_backend_reallocate(void *memory_pointer, ft_size_t size,
            int *error_code)
            __attribute__ ((warn_unused_result, hot));
int     cma_backend_deallocate(void *memory_pointer) __attribute__ ((hot));
void    *cma_backend_aligned_allocate(ft_size_t alignment, ft_size_t size,
            int *error_code)
            __attribute__ ((warn_unused_result, hot));
ft_size_t    cma_backend_block_size(const void *memory_pointer)
            __attribute__ ((warn_unused_result, hot));
int     cma_backend_checked_block_size(const void *memory_pointer,
            ft_size_t *block_size) __attribute__ ((warn_unused_result, hot));
#ifndef CMA_ENABLE_METADATA_PROTECTION
# define CMA_ENABLE_METADATA_PROTECTION 1
#endif

int     cma_metadata_make_writable(void);
void    cma_metadata_make_inaccessible(void);
bool    cma_metadata_guard_increment(void);
bool    cma_metadata_guard_decrement(void);
Block    *cma_metadata_allocate_block(void) __attribute__ ((warn_unused_result));
void    cma_metadata_release_block(Block *block);
void    cma_metadata_reset(void);

#if DEBUG
ft_size_t    cma_debug_allocation_size(ft_size_t requested_size);
void    cma_debug_initialize_block(Block *block);
void    cma_debug_prepare_allocation(Block *block, ft_size_t user_size);
void    cma_debug_release_allocation(Block *block, const char *context,
            const void *user_pointer);
unsigned char    *cma_debug_user_pointer(const Block *block);
ft_size_t    cma_debug_user_size(const Block *block);
ft_size_t    cma_debug_guard_size(void);
#else
inline ft_size_t cma_debug_allocation_size(ft_size_t requested_size)
{
    return (requested_size);
}

inline void cma_debug_initialize_block(Block *block)
{
    (void)block;
    return ;
}

inline void cma_debug_prepare_allocation(Block *block, ft_size_t user_size)
{
    (void)block;
    (void)user_size;
    return ;
}

inline void cma_debug_release_allocation(Block *block, const char *context,
        const void *user_pointer)
{
    (void)block;
    (void)context;
    (void)user_pointer;
    return ;
}

inline unsigned char *cma_debug_user_pointer(const Block *block)
{
    if (!block)
        return (ft_nullptr);
    return (block->payload);
}

inline ft_size_t cma_debug_user_size(const Block *block)
{
    if (!block)
        return (0);
    return (block->size);
}

inline ft_size_t cma_debug_guard_size(void)
{
    return (0);
}
#endif

inline unsigned char *cma_block_user_pointer(const Block *block)
{
    return (cma_debug_user_pointer(block));
}

inline ft_size_t cma_block_user_size(const Block *block)
{
    return (cma_debug_user_size(block));
}

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
