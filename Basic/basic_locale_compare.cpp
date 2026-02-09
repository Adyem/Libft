#include "basic.hpp"

int su_locale_compare(const char *left, const char *right, const char *locale_name,
            int *result);

int ft_locale_compare(const char *left, const char *right, const char *locale_name)
{
    if (!left || !right)
        return (-1);
    int comparison_result = 0;
    if (su_locale_compare(left, right, locale_name, &comparison_result) != 0)
        return (-1);
    return (comparison_result);
}
