#ifndef CMA_HPP
# define CMA_HPP

#include <cstddef>

void	*cma_malloc(int size) __attribute__ ((warn_unused_result, hot));
void	cma_free(void* ptr) __attribute__ ((hot));
char	*cma_strdup(const char *string) __attribute__ ((warn_unused_result));
void	*cma_calloc(int count, int size) __attribute__ ((warn_unused_result));
void	*cma_realloc(void* ptr, size_t new_size) __attribute__ ((warn_unused_result));
char	**cma_split(char const *s, char c) __attribute__ ((warn_unused_result));
char	*cma_itoa(int n) __attribute__ ((warn_unused_result));
char	*cma_strjoin(char const *string_1, char const *string_2)
						__attribute__ ((warn_unused_result));
void	cma_free_double(char **content);
void	cma_cleanup();

#endif
