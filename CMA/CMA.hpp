#ifndef CMA_HPP
# define CMA_HPP

#include <cstddef>

void	*cma_malloc(std::size_t size) __attribute__ ((warn_unused_result, hot));
void	cma_free(void* ptr) __attribute__ ((hot));
int     cma_checked_free(void* ptr) __attribute__ ((warn_unused_result, hot));
char	*cma_strdup(const char *string) __attribute__ ((warn_unused_result));
void    *cma_memdup(const void *source, size_t size) __attribute__ ((warn_unused_result));
void	*cma_calloc(std::size_t, std::size_t size) __attribute__ ((warn_unused_result));
void	*cma_realloc(void* ptr, std::size_t new_size) __attribute__ ((warn_unused_result));
char	**cma_split(char const *s, char c) __attribute__ ((warn_unused_result));
char	*cma_itoa(int n) __attribute__ ((warn_unused_result));
char    *cma_itoa_base(int n, int base) __attribute__ ((warn_unused_result));
char	*cma_strjoin(char const *string_1, char const *string_2)
			__attribute__ ((warn_unused_result));
char    *cma_strjoin_multiple(int count, ...)
            __attribute__ ((warn_unused_result));
char    *cma_substr(const char *s, unsigned int start, size_t len)
            __attribute__ ((warn_unused_result));
char    *cma_strtrim(const char *s1, const char *set)
            __attribute__ ((warn_unused_result));
void    cma_free_double(char **content);
void    cma_cleanup();

#endif
