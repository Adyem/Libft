#include "libft.hpp"

#if LIBFT_HAS_LOCALE_HELPERS

#include "../System_utils/system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

int    ft_locale_compare(const char *left, const char *right, const char *locale_name)
{
    int comparison_result;
    int status;

    comparison_result = 0;
    ft_errno = FT_ERR_SUCCESSS;
    status = su_locale_compare(left, right, locale_name, &comparison_result);
    if (status != 0)
        return (-1);
    return (comparison_result);
}

ft_string    ft_locale_casefold(const char *input, const char *locale_name)
{
    ft_string folded_result;
    int status;
    int error_code;

    ft_errno = FT_ERR_SUCCESSS;
    status = su_locale_casefold(input, locale_name, folded_result);
    if (status != 0)
    {
        error_code = ft_errno;
        return (ft_string(error_code));
    }
    return (folded_result);
}

#endif
