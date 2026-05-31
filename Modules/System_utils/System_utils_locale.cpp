#include "system_utils.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include <locale>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static int32_t assign_locale(const char *locale_name, std::locale &locale_object)
{
    const char *name_pointer;

    name_pointer = locale_name;
    try
    {
        if (name_pointer == ft_nullptr || *name_pointer == '\0')
        {
            try
            {
                locale_object = std::locale("");
            }
            catch (const std::runtime_error &)
            {
                locale_object = std::locale::classic();
            }
        }
        else
            locale_object = std::locale(name_pointer);
    }
    catch (const std::bad_alloc &)
    {
        return (-1);
    }
    catch (const std::runtime_error &)
    {
        return (-1);
    }
    return (0);
}

int32_t su_locale_compare(const char *left, const char *right, const char *locale_name, int32_t *result)
{
    std::locale locale_object;
    const std::collate<char> *collate_facet;
    ft_size_t left_length;
    ft_size_t right_length;
    int32_t comparison_value;

    if (left == ft_nullptr || right == ft_nullptr || result == ft_nullptr)
        return (-1);
    if (assign_locale(locale_name, locale_object) != 0)
        return (-1);
    try
    {
        collate_facet = &std::use_facet<std::collate<char> >(locale_object);
    }
    catch (const std::bad_cast &)
    {
        return (-1);
    }
    catch (const std::bad_alloc &)
    {
        return (-1);
    }
    left_length = std::char_traits<char>::length(left);
    right_length = std::char_traits<char>::length(right);
    comparison_value = collate_facet->compare(left, left + left_length, right, right + right_length);
    *result = comparison_value;
    return (0);
}

int32_t su_locale_casefold(const char *input, const char *locale_name, ft_string &output)
{
    std::locale locale_object;
    const std::ctype<char> *ctype_facet;
    ft_size_t input_length;
    std::string transformed_string;
    ft_size_t index;
    int32_t output_error;

    if (input == ft_nullptr)
        return (-1);
    if (assign_locale(locale_name, locale_object) != 0)
        return (-1);
    try
    {
        ctype_facet = &std::use_facet<std::ctype<char> >(locale_object);
    }
    catch (const std::bad_cast &)
    {
        return (-1);
    }
    catch (const std::bad_alloc &)
    {
        return (-1);
    }
    input_length = std::char_traits<char>::length(input);
    try
    {
        transformed_string.assign(input, input + input_length);
    }
    catch (const std::bad_alloc &)
    {
        return (-1);
    }
    index = 0;
    while (index < input_length)
    {
        transformed_string[index] = ctype_facet->tolower(transformed_string[index]);
        index++;
    }
    output_error = output.assign(transformed_string.c_str(), transformed_string.size());
    if (output_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}
