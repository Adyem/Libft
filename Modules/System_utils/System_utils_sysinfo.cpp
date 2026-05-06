#include "system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

uint32_t    su_get_cpu_count(void)
{
    uint32_t result;

    result = cmp_get_cpu_count();
    return (result);
}

uint64_t su_get_total_memory(void)
{
    uint64_t result;
    int32_t error_code;

    result = 0;
    error_code = cmp_get_total_memory(&result);
    if (error_code != FT_ERR_SUCCESS)
        return (result);
    return (result);
}
