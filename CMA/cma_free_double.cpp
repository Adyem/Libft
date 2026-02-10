#include "CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

void    cma_free_double(char **content)
{
    if (content == ft_nullptr)
        return ;
    ft_size_t index = 0;
    ft_size_t pointer_capacity = cma_alloc_size(content);

    if (pointer_capacity != 0)
        pointer_capacity /= sizeof(char *);
    while (content[index])
    {
        cma_free(content[index]);
        index++;
        if (pointer_capacity != 0 && index >= pointer_capacity)
            break ;
    }
    cma_free(content);
}
