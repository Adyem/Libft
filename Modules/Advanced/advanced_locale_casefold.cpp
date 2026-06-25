#include "advanced.hpp"
#include "../System_utils/system_utils.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

ft_string *adv_locale_casefold(const char *input, const char *locale_name)
{
    if (input == ft_nullptr)
        return (ft_nullptr);
    ft_string *folded_result = new (std::nothrow) ft_string();
    if (folded_result == ft_nullptr)
        return (ft_nullptr);
    if (folded_result->initialize() != FT_ERR_SUCCESS)
    {
        delete folded_result;
        return (ft_nullptr);
    }
    if (su_locale_casefold(input, locale_name, *folded_result) != FT_ERR_SUCCESS)
    {
        delete folded_result;
        return (ft_nullptr);
    }
    return (folded_result);
}
