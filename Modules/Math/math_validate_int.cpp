#include "math.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

int32_t math_validate_int(const char *input)
{
    return (ft_validate_int(input));
}
