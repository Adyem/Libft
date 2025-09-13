#include "system_utils.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"

char    *su_get_home_directory(void)
{
    return (cmp_get_home_directory());
}
