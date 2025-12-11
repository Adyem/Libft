#ifndef CMA_HPP
# define CMA_HPP

#include <cstddef>
#include "../Libft/libft.hpp"

typedef void    *(*cma_backend_allocate_function)(ft_size_t size, void *user_data);
typedef void    *(*cma_backend_reallocate_function)(void *memory_pointer,
            ft_size_t size, void *user_data);
typedef void    (*cma_backend_deallocate_function)(void *memory_pointer,
            void *user_data);
typedef void    *(*cma_backend_aligned_allocate_function)(ft_size_t alignment,
            ft_size_t size, void *user_data);
typedef ft_size_t    (*cma_backend_get_allocation_size_function)(
            const void *memory_pointer, void *user_data);
typedef int     (*cma_backend_owns_allocation_function)(
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

int     cma_set_backend(const cma_backend_hooks *hooks)
            __attribute__ ((warn_unused_result));
void    cma_clear_backend(void);
int     cma_backend_is_enabled(void) __attribute__ ((warn_unused_result));

void    *cma_malloc(ft_size_t size) __attribute__ ((warn_unused_result, hot));
void    cma_free(void* ptr) __attribute__ ((hot));
int     cma_checked_free(void* ptr) __attribute__ ((warn_unused_result, hot));
char    *cma_strdup(const char *string) __attribute__ ((warn_unused_result));
char    *cma_strndup(const char *string, size_t maximum_length)
            __attribute__ ((warn_unused_result));
void    *cma_memdup(const void *source, size_t size) __attribute__ ((warn_unused_result));
void    *cma_calloc(ft_size_t count, ft_size_t size) __attribute__ ((warn_unused_result));
void    *cma_realloc(void* ptr, ft_size_t new_size) __attribute__ ((warn_unused_result));
void    *cma_aligned_alloc(ft_size_t alignment, ft_size_t size)
            __attribute__ ((warn_unused_result, hot));
ft_size_t    cma_alloc_size(const void* ptr)
            __attribute__ ((warn_unused_result, hot));
ft_size_t    cma_block_size(const void *memory_pointer)
            __attribute__ ((warn_unused_result, hot));
int    cma_checked_block_size(const void *memory_pointer, ft_size_t *block_size)
            __attribute__ ((warn_unused_result, hot));
int    *cma_atoi(const char *string) __attribute__ ((warn_unused_result));
char    **cma_split(char const *string, char delimiter) __attribute__ ((warn_unused_result));
char    *cma_itoa(int number) __attribute__ ((warn_unused_result));
char    *cma_itoa_base(int number, int base) __attribute__ ((warn_unused_result));
char    *cma_strjoin(char const *string_1, char const *string_2)
            __attribute__ ((warn_unused_result));
char    *cma_strjoin_multiple(int count, ...)
            __attribute__ ((warn_unused_result));
char    *cma_substr(const char *source, unsigned int start, size_t length)
            __attribute__ ((warn_unused_result));
char    *cma_strtrim(const char *string, const char *set)
            __attribute__ ((warn_unused_result));
void    cma_free_double(char **content);
void    cma_set_alloc_limit(ft_size_t limit);
void    cma_set_thread_safety(bool enable);
void    cma_get_stats(ft_size_t *allocation_count, ft_size_t *free_count);
void    cma_get_extended_stats(ft_size_t *allocation_count,
            ft_size_t *free_count,
            ft_size_t *current_bytes,
            ft_size_t *peak_bytes);
void    cma_leak_detection_enable(void);
void    cma_leak_detection_disable(void);
void    cma_leak_detection_clear(void);
bool    cma_leak_detection_is_enabled(void);
ft_size_t    cma_leak_detection_outstanding_allocations(void);
ft_size_t    cma_leak_detection_outstanding_bytes(void);
ft_string    cma_leak_detection_report(bool clear_after);
#endif
