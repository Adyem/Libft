#include "system_utils.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

char    *su_get_home_directory(void)
{
    char *result;

    result = cmp_get_home_directory();
    return (result);
}
