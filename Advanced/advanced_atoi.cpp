#include "advanced.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

int32_t *adv_atoi(const char *string)
{
    if (ft_validate_int(string) != FT_SUCCESS)
        return (ft_nullptr);
    int32_t parsed_value = ft_atoi(string);
    int32_t *result = static_cast<int32_t *>(cma_malloc(sizeof(int32_t)));
    if (!result)
        return (ft_nullptr);
    *result = static_cast<int32_t>(parsed_value);
    return (result);
}
