#include "CMA.hpp"

void    cma_free_double(char **content)
{
    int    index;
    ft_size_t    pointer_capacity;

    index = 0;
    pointer_capacity = 0;
    if (content)
    {
        pointer_capacity = cma_alloc_size(content);
        if (pointer_capacity != 0)
            pointer_capacity /= sizeof(char *);
        while (content[index])
        {
            cma_free(content[index]);
            index++;
            if (pointer_capacity != 0
                && static_cast<ft_size_t>(index) >= pointer_capacity)
                break ;
        }
        cma_free(content);
    }
    return ;
}
