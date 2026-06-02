#include "math.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

int32_t math_abs(int32_t number)
{
    if (number < 0)
    {
        if (number == FT_INT32_MIN)
            return (FT_INT32_MAX);
        return (-number);
    }
    return (number);
}

int64_t math_abs(int64_t number)
{
    if (number < 0)
    {
        if (number == static_cast<int64_t>(FT_LLONG_MIN))
            return (static_cast<int64_t>(FT_LLONG_MAX));
        return (-number);
    }
    return (number);
}
