#ifndef CMA_HPP
# define CMA_HPP

#include "../Basic/basic.hpp"

typedef void    *(*cma_backend_allocate_function)(ft_size_t size, void *user_data);
typedef void    *(*cma_backend_reallocate_function)(void *memory_pointer,
            ft_size_t size, void *user_data);
typedef void    (*cma_backend_deallocate_function)(void *memory_pointer,
            void *user_data);
typedef void    *(*cma_backend_aligned_allocate_function)(ft_size_t alignment,
            ft_size_t size, void *user_data);
typedef ft_size_t    (*cma_backend_get_allocation_size_function)(
            const void *memory_pointer, void *user_data);
typedef int32_t (*cma_backend_owns_allocation_function)(
            const void *memory_pointer, void *user_data);

struct cma_backend_hooks
{
    cma_backend_allocate_function allocate;
    cma_backend_reallocate_function reallocate;
    cma_backend_deallocate_function deallocate;
    cma_backend_aligned_allocate_function aligned_allocate;
    cma_backend_get_allocation_size_function get_allocation_size;
    cma_backend_owns_allocation_function owns_allocation;
    void    *user_data;
};

int32_t     cma_set_backend(const cma_backend_hooks *hooks)
                __attribute__ ((warn_unused_result));
int32_t     cma_clear_backend(void);
int32_t     cma_backend_is_enabled(void) __attribute__ ((warn_unused_result));
void        *cma_malloc(ft_size_t size) __attribute__ ((warn_unused_result, hot));
void        *cma_realloc(void *memory_pointer, ft_size_t size)
                __attribute__ ((warn_unused_result, hot));
void        *cma_aligned_alloc(ft_size_t alignment, ft_size_t size)
                __attribute__ ((warn_unused_result, hot));
void        cma_free(void* ptr) __attribute__ ((hot));
int32_t      cma_checked_free(void* ptr) __attribute__ ((warn_unused_result, hot));
void        cma_free_double(char **content);
ft_size_t    cma_block_size(const void *memory_pointer)
                __attribute__ ((warn_unused_result, hot));
int32_t     cma_checked_block_size(const void *memory_pointer,
                ft_size_t *block_size) __attribute__ ((warn_unused_result, hot));
ft_size_t   cma_alloc_size(const void *memory_pointer)
                __attribute__ ((warn_unused_result, hot));
int32_t     cma_set_alloc_limit(ft_size_t limit);
int32_t     cma_set_thread_safety(bool enable);
int32_t     cma_enable_thread_safety(void);
int32_t     cma_disable_thread_safety(void);
bool        cma_is_thread_safe_enabled(void);
int32_t     cma_get_stats(ft_size_t *allocation_count, ft_size_t *free_count);
int32_t     cma_get_extended_stats(ft_size_t *allocation_count,
                ft_size_t *free_count, ft_size_t *current_bytes,
                ft_size_t *peak_bytes);
#endif
