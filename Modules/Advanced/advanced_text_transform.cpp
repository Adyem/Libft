#include "advanced.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

static ft_bool adv_is_word_character(char character)
{
    if (ft_isalnum(static_cast<int32_t>(static_cast<unsigned char>(character))))
        return (FT_TRUE);
    return (FT_FALSE);
}

static char adv_to_ascii_lower(char character)
{
    if (character >= 'A' && character <= 'Z')
        return (static_cast<char>(character + ('a' - 'A')));
    return (character);
}

static char adv_to_ascii_upper(char character)
{
    if (character >= 'a' && character <= 'z')
        return (static_cast<char>(character - ('a' - 'A')));
    return (character);
}

static char *adv_allocate_text_transform(ft_size_t length)
{
    char *result;

    result = static_cast<char *>(cma_malloc(length + 1));
    if (result == ft_nullptr)
        return (ft_nullptr);
    result[length] = '\0';
    return (result);
}

char *adv_str_to_title_case(const char *input_string)
{
    char *result;
    ft_size_t input_index;
    ft_size_t output_index;
    ft_bool start_of_word;

    if (input_string == ft_nullptr)
        return (ft_nullptr);
    result = adv_allocate_text_transform(ft_strlen_size_t(input_string));
    if (result == ft_nullptr)
        return (ft_nullptr);
    input_index = 0;
    output_index = 0;
    start_of_word = FT_TRUE;
    while (input_string[input_index] != '\0')
    {
        if (adv_is_word_character(input_string[input_index]) == FT_TRUE)
        {
            if (start_of_word == FT_TRUE)
                result[output_index] = adv_to_ascii_upper(input_string[input_index]);
            else
                result[output_index] = adv_to_ascii_lower(input_string[input_index]);
            start_of_word = FT_FALSE;
        }
        else
        {
            result[output_index] = input_string[input_index];
            start_of_word = FT_TRUE;
        }
        ++input_index;
        ++output_index;
    }
    return (result);
}

char *adv_str_to_snake_case(const char *input_string)
{
    char *result;
    ft_size_t input_index;
    ft_size_t output_index;
    ft_bool start_of_word;
    ft_bool have_written_word;

    if (input_string == ft_nullptr)
        return (ft_nullptr);
    result = adv_allocate_text_transform(ft_strlen_size_t(input_string));
    if (result == ft_nullptr)
        return (ft_nullptr);
    input_index = 0;
    output_index = 0;
    start_of_word = FT_TRUE;
    have_written_word = FT_FALSE;
    while (input_string[input_index] != '\0')
    {
        if (adv_is_word_character(input_string[input_index]) == FT_TRUE)
        {
            if (start_of_word == FT_TRUE && have_written_word == FT_TRUE)
            {
                result[output_index] = '_';
                ++output_index;
            }
            result[output_index] = adv_to_ascii_lower(input_string[input_index]);
            start_of_word = FT_FALSE;
            have_written_word = FT_TRUE;
            ++output_index;
        }
        else
        {
            start_of_word = FT_TRUE;
        }
        ++input_index;
    }
    if (output_index == 0)
        result[0] = '\0';
    else
        result[output_index] = '\0';
    return (result);
}

char *adv_str_to_camel_case(const char *input_string)
{
    char *result;
    ft_size_t input_index;
    ft_size_t output_index;
    ft_bool start_of_word;

    if (input_string == ft_nullptr)
        return (ft_nullptr);
    result = adv_allocate_text_transform(ft_strlen_size_t(input_string));
    if (result == ft_nullptr)
        return (ft_nullptr);
    input_index = 0;
    output_index = 0;
    start_of_word = FT_TRUE;
    while (input_string[input_index] != '\0')
    {
        if (adv_is_word_character(input_string[input_index]) == FT_TRUE)
        {
            if (output_index == 0)
                result[output_index] = adv_to_ascii_lower(input_string[input_index]);
            else if (start_of_word == FT_TRUE)
                result[output_index] = adv_to_ascii_upper(input_string[input_index]);
            else
                result[output_index] = adv_to_ascii_lower(input_string[input_index]);
            start_of_word = FT_FALSE;
            ++output_index;
        }
        else
        {
            start_of_word = FT_TRUE;
        }
        ++input_index;
    }
    result[output_index] = '\0';
    return (result);
}

char *adv_str_normalize_whitespace(const char *input_string)
{
    char *result;
    ft_size_t input_index;
    ft_size_t output_index;
    ft_bool pending_space;

    if (input_string == ft_nullptr)
        return (ft_nullptr);
    result = adv_allocate_text_transform(ft_strlen_size_t(input_string));
    if (result == ft_nullptr)
        return (ft_nullptr);
    input_index = 0;
    output_index = 0;
    pending_space = FT_FALSE;
    while (input_string[input_index] != '\0')
    {
        if (ft_isspace(static_cast<int32_t>(static_cast<unsigned char>(input_string[input_index]))))
        {
            if (output_index > 0)
                pending_space = FT_TRUE;
        }
        else
        {
            if (pending_space == FT_TRUE)
            {
                result[output_index] = ' ';
                ++output_index;
                pending_space = FT_FALSE;
            }
            result[output_index] = input_string[input_index];
            ++output_index;
        }
        ++input_index;
    }
    result[output_index] = '\0';
    return (result);
}
