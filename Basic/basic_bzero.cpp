#include "basic.hpp"

void    ft_bzero(void *pointer, ft_size_t size)
{
    ft_memset(pointer, 0, size);
    return ;
}
