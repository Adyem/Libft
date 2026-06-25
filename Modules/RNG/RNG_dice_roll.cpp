#include <climits>
#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"

int32_t ft_dice_roll(int32_t number, int32_t faces)
{
    int32_t zero_value;

    zero_value = FT_ERR_SUCCESS;
    ft_init_random_engine();
    if (faces == 0 && number == 0)
        return (zero_value);
    if (faces < 1 || number < 1)
        return (FT_ERR_INVALID_ARGUMENT);
    if (faces == 1)
        return (number);
    int32_t result = 0;
    int32_t index = 0;
    int32_t roll_value = 0;
    while (index < number)
    {
        roll_value = ft_random_int();
        if (result > INT_MAX - ((roll_value % faces) + 1))
            return (FT_ERR_OUT_OF_RANGE);
        result += (roll_value % faces) + 1;
        index++;
    }
    if (DEBUG == 1)
        pf_printf_fd(2, "The dice rolled %d on %d faces with %d amount of dice\n",
                result, faces, number);
    return (result);
}
