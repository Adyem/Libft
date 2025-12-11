#include <climits>
#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"

int ft_dice_roll(int number, int faces)
{
    ft_init_random_engine();
    if (faces == 0 && number == 0)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (faces < 1 || number < 1)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (faces == 1)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (number);
    }
    int result = 0;
    int index = 0;
    int roll = 0;
    while (index < number)
    {
        roll = ft_random_int();
        if (ft_errno != FT_ERR_SUCCESSS)
            return (-1);
        if (result > INT_MAX - ((roll % faces) + 1))
        {
            ft_errno = FT_ERR_OUT_OF_RANGE;
            return (-1);
        }
        result += (roll % faces) + 1;
        index++;
    }
    if (DEBUG == 1)
        pf_printf_fd(2, "The dice rolled %d on %d faces with %d amount of dice\n",
                result, faces, number);
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}
