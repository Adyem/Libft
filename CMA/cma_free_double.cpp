#include "CMA.hpp"

void    cma_free_double(char **content)
{
    int    index;
    ft_size_t    pointer_capacity;
    int    error_code;
    int    final_error;

    index = 0;
    pointer_capacity = 0;
    final_error = FT_ERR_SUCCESSS;
    if (content)
    {
        pointer_capacity = cma_alloc_size(content);
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code != FT_ERR_SUCCESSS)
            final_error = error_code;
        if (pointer_capacity != 0)
            pointer_capacity /= sizeof(char *);
        while (content[index])
        {
            cma_free(content[index]);
            error_code = ft_global_error_stack_drop_last_error();
            if (error_code != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
                final_error = error_code;
            index++;
            if (pointer_capacity != 0
                && static_cast<ft_size_t>(index) >= pointer_capacity)
                break ;
        }
        cma_free(content);
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
            final_error = error_code;
    }
    ft_global_error_stack_push(final_error);
    return ;
}
