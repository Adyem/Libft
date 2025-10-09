#ifndef CMA_HPP
# define CMA_HPP

#include <cstddef>
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

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
void    cma_cleanup();
void    cma_set_alloc_limit(ft_size_t limit);
void    cma_set_thread_safety(bool enable);
void    cma_get_stats(ft_size_t *allocation_count, ft_size_t *free_count);

#endif
