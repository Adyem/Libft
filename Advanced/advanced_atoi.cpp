#include "advanced.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

int *adv_atoi(const char *string)
{
    if (ft_validate_int(string) != FT_SUCCESS)
        return (ft_nullptr);
    int32_t parsed_value = ft_atoi(string);
    int *result = static_cast<int *>(cma_malloc(sizeof(int)));
    if (!result)
        return (ft_nullptr);
    *result = static_cast<int>(parsed_value);
    return (result);
}
