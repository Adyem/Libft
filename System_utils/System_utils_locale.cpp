#include "system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include <locale>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <new>

static int assign_locale(const char *locale_name, std::locale &locale_object)
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
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    catch (const std::runtime_error &)
    {
        ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int su_locale_compare(const char *left, const char *right, const char *locale_name, int *result)
{
    std::locale locale_object;
    const std::collate<char> *collate_facet;
    size_t left_length;
    size_t right_length;
    int comparison_value;
    int error_code;

    if (left == ft_nullptr || right == ft_nullptr || result == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (assign_locale(locale_name, locale_object) != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_CONFIGURATION;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    try
    {
        collate_facet = &std::use_facet<std::collate<char> >(locale_object);
    }
    catch (const std::bad_cast &)
    {
        ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (-1);
    }
    catch (const std::bad_alloc &)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    left_length = std::char_traits<char>::length(left);
    right_length = std::char_traits<char>::length(right);
    comparison_value = collate_facet->compare(left, left + left_length, right, right + right_length);
    *result = comparison_value;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int su_locale_casefold(const char *input, const char *locale_name, ft_string &output)
{
    std::locale locale_object;
    const std::ctype<char> *ctype_facet;
    size_t input_length;
    std::string transformed_string;
    size_t index;
    int error_code;

    if (input == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (assign_locale(locale_name, locale_object) != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_CONFIGURATION;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    try
    {
        ctype_facet = &std::use_facet<std::ctype<char> >(locale_object);
    }
    catch (const std::bad_cast &)
    {
        ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (-1);
    }
    catch (const std::bad_alloc &)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    input_length = std::char_traits<char>::length(input);
    try
    {
        transformed_string.assign(input, input + input_length);
    }
    catch (const std::bad_alloc &)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    index = 0;
    while (index < input_length)
    {
        transformed_string[index] = ctype_facet->tolower(transformed_string[index]);
        index++;
    }
    output.assign(transformed_string.c_str(), transformed_string.size());
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(output.get_error());
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
