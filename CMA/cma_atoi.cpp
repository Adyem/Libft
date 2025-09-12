#include "CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

int *cma_atoi(const char *string)
{
    int *number;

    if (ft_validate_int(string))
        return (ft_nullptr);
    number = static_cast<int *>(cma_malloc(sizeof(int)));
    if (!number)
        return (ft_nullptr);
    *number = ft_atoi(string);
    return (number);
}
