#include "CMA.hpp"

void    cma_free_double(char **content)
{
    int    index;

    index = 0;
    if (content)
    {
        while (content[index])
        {
            cma_free(content[index]);
            index++;
        }
        cma_free(content);
    }
    return ;
}
