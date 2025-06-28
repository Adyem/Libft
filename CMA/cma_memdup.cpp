#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"

void* cma_memdup(const void* src, size_t size)
{
        if (!src || size == 0)
                return (ft_nullptr);
        void* new_mem = cma_malloc(size);
        if (!new_mem)
                return (ft_nullptr);
        ft_memcpy(new_mem, src, size);
        return (new_mem);
}
