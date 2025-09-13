#include "system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"

unsigned int    su_get_cpu_count(void)
{
    return (cmp_get_cpu_count());
}

unsigned long long su_get_total_memory(void)
{
    return (cmp_get_total_memory());
}
