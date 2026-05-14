#ifndef COMPATEBILITY_CMA_PLATFORM_HPP
#define COMPATEBILITY_CMA_PLATFORM_HPP


#ifndef LIBFT_INTERNAL_HEADERS
# error "This is a libft internal header. Define LIBFT_INTERNAL_HEADERS only when building libft internals."
#endif
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"

int32_t cmp_cma_get_page_size(ft_size_t *page_size_out);
void *cmp_cma_memory_map_read_write(ft_size_t mapping_size);
int32_t cmp_cma_memory_protect_none(void *memory_pointer, ft_size_t mapping_size);
int32_t cmp_cma_memory_protect_read_write(void *memory_pointer,
    ft_size_t mapping_size);
int32_t cmp_cma_memory_unmap(void *memory_pointer, ft_size_t mapping_size);

#endif
