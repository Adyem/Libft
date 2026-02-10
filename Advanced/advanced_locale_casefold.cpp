#include "advanced.hpp"
#include "../System_utils/system_utils.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <new>

ft_string *adv_locale_casefold(const char *input, const char *locale_name)
{
    if (input == ft_nullptr)
        return (ft_nullptr);
    ft_string *folded_result = new (std::nothrow) ft_string();
    if (folded_result == ft_nullptr)
        return (ft_nullptr);
    if (folded_result->initialize() != FT_ERR_SUCCESSS)
    {
        delete folded_result;
        return (ft_nullptr);
    }
    if (su_locale_casefold(input, locale_name, *folded_result) != 0)
    {
        delete folded_result;
        return (ft_nullptr);
    }
    return (folded_result);
}
