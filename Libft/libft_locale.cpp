#include "libft.hpp"

#if LIBFT_HAS_LOCALE_HELPERS

#include "../System_utils/system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static int report_locale_error(int error_code, int default_error)
{
    if (error_code == FT_ERR_SUCCESSS)
        error_code = default_error;
    ft_global_error_stack_push(error_code);
    return (-1);
}

static ft_string report_locale_string_error(int error_code, int default_error)
{
    if (error_code == FT_ERR_SUCCESSS)
        error_code = default_error;
    ft_global_error_stack_push(error_code);
    return (ft_string(error_code));
}

int    ft_locale_compare(const char *left, const char *right, const char *locale_name)
{
    int comparison_result;
    int status;
    int error_code;

    comparison_result = 0;
    status = su_locale_compare(left, right, locale_name, &comparison_result);
    if (status != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        return (report_locale_error(error_code, FT_ERR_CONFIGURATION));
    }
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
        return (report_locale_error(error_code, FT_ERR_CONFIGURATION));
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (comparison_result);
}

ft_string    ft_locale_casefold(const char *input, const char *locale_name)
{
    ft_string folded_result;
    int status;
    int error_code;

    status = su_locale_casefold(input, locale_name, folded_result);
    if (status != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        return (report_locale_string_error(error_code, FT_ERR_CONFIGURATION));
    }
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
        return (report_locale_string_error(error_code, FT_ERR_CONFIGURATION));
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (folded_result);
}

#endif
