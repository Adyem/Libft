#ifndef CMA_HPP
# define CMA_HPP

#include <cstddef>

void    *cma_malloc(std::size_t size) __attribute__ ((warn_unused_result, hot));
void    cma_free(void* ptr) __attribute__ ((hot));
int     cma_checked_free(void* ptr) __attribute__ ((warn_unused_result, hot));
char    *cma_strdup(const char *string) __attribute__ ((warn_unused_result));
void    *cma_memdup(const void *source, size_t size) __attribute__ ((warn_unused_result));
void    *cma_calloc(std::size_t, std::size_t size) __attribute__ ((warn_unused_result));
void    *cma_realloc(void* ptr, std::size_t new_size) __attribute__ ((warn_unused_result));
void    *cma_aligned_alloc(std::size_t alignment, std::size_t size)
            __attribute__ ((warn_unused_result, hot));
std::size_t    cma_alloc_size(const void* ptr)
            __attribute__ ((warn_unused_result, hot));
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

#endif
