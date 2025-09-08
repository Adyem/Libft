#include "libft.hpp"

void    ft_bzero(void *pointer, size_t size)
{
    ft_memset(pointer, 0, size);
    return ;
}
