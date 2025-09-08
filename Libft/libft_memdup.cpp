#include "libft.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"
#include "../CMA/CMA.hpp"

void *ft_memdup(const void *source, size_t size)
{
    if (source == ft_nullptr || size == 0)
        return (ft_nullptr);
    void *duplicate = cma_malloc(size);
    if (duplicate == ft_nullptr)
        return (ft_nullptr);
    ft_memcpy(duplicate, source, size);
    return (duplicate);
}
