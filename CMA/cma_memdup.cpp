#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

void* cma_memdup(const void* src, size_t size)
{
        if (size == 0)
        {
                void *duplicate_zero;

                duplicate_zero = cma_malloc(0);
                return (duplicate_zero);
        }
        if (!src)
                return (ft_nullptr);
        void* new_mem = cma_malloc(size);
        if (!new_mem)
                return (ft_nullptr);
        ft_memcpy(new_mem, src, size);
        return (new_mem);
}
