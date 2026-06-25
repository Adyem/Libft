#include "system_utils.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

char    *su_get_home_directory(void)
{
    char *result;

    result = cmp_get_home_directory();
    return (result);
}
