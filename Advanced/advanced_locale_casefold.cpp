#include "advanced.hpp"

#if LIBFT_HAS_LOCALE_HELPERS

#include "../System_utils/system_utils.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

ft_string    adv_locale_casefold(const char *input, const char *locale_name)
{
    ft_string folded_result;
    int status;
    int error_code;

    status = su_locale_casefold(input, locale_name, folded_result);
    if (status != 0)
    {
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_CONFIGURATION;
        ft_global_error_stack_push(error_code);
        return (ft_string(error_code));
    }
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_CONFIGURATION;
        ft_global_error_stack_push(error_code);
        return (ft_string(error_code));
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (folded_result);
}

#endif
