#include "math.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

void math_swap(int32_t *first_number, int32_t *second_number)
{
    int32_t temporary;

    if (!first_number || !second_number)
        return ;
    temporary = *first_number;
    *first_number = *second_number;
    *second_number = temporary;
    return ;
}
