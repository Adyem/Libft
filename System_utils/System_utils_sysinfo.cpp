#include "system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

unsigned int    su_get_cpu_count(void)
{
    unsigned int result;

    result = cmp_get_cpu_count();
    return (result);
}

unsigned long long su_get_total_memory(void)
{
    uint64_t result;
    int error_code;

    result = 0;
    error_code = cmp_get_total_memory(&result);
    if (error_code != FT_ERR_SUCCESS)
        return (result);
    return (result);
}
