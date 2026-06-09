#include "regex.hpp"
#include <new>
#include <regex>
#include <string>

static int32_t regex_compile_pattern(const char *pattern,
    std::regex *compiled_regex) noexcept
{
    try
    {
        *compiled_regex = std::regex(pattern, std::regex_constants::ECMAScript);
    }
    catch (const std::bad_alloc &)
    {
        return (FT_ERR_NO_MEMORY);
    }
    catch (const std::regex_error &)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    catch (...)
    {
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESS);
}

int32_t regex_validate_pattern(const char *pattern) noexcept
{
    std::regex compiled_regex;

    if (pattern == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    return (regex_compile_pattern(pattern, &compiled_regex));
}

int32_t regex_is_match(const char *pattern, const char *text,
    ft_bool *is_match) noexcept
{
    std::regex compiled_regex;
    ft_bool match_result;
    int32_t compile_error;

    if (pattern == ft_nullptr || text == ft_nullptr || is_match == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    compile_error = regex_compile_pattern(pattern, &compiled_regex);
    if (compile_error != FT_ERR_SUCCESS)
        return (compile_error);
    try
    {
        if (std::regex_match(text, compiled_regex))
            match_result = FT_TRUE;
        else
            match_result = FT_FALSE;
    }
    catch (const std::bad_alloc &)
    {
        return (FT_ERR_NO_MEMORY);
    }
    catch (...)
    {
        return (FT_ERR_INTERNAL);
    }
    *is_match = match_result;
    return (FT_ERR_SUCCESS);
}

int32_t regex_search(const char *pattern, const char *text,
    ft_bool *is_found) noexcept
{
    std::regex compiled_regex;
    ft_bool found_result;
    int32_t compile_error;

    if (pattern == ft_nullptr || text == ft_nullptr || is_found == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    compile_error = regex_compile_pattern(pattern, &compiled_regex);
    if (compile_error != FT_ERR_SUCCESS)
        return (compile_error);
    try
    {
        if (std::regex_search(text, compiled_regex))
            found_result = FT_TRUE;
        else
            found_result = FT_FALSE;
    }
    catch (const std::bad_alloc &)
    {
        return (FT_ERR_NO_MEMORY);
    }
    catch (...)
    {
        return (FT_ERR_INTERNAL);
    }
    *is_found = found_result;
    return (FT_ERR_SUCCESS);
}

int32_t regex_replace_all(const char *pattern, const char *replacement,
    const char *text, ft_string *output) noexcept
{
    std::regex compiled_regex;
    std::string replaced_text;
    int32_t compile_error;
    int32_t assign_error;

    if (pattern == ft_nullptr || replacement == ft_nullptr || text == ft_nullptr
        || output == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    if (output->is_initialised() == FT_FALSE)
        return (FT_ERR_NOT_INITIALISED);
    compile_error = regex_compile_pattern(pattern, &compiled_regex);
    if (compile_error != FT_ERR_SUCCESS)
        return (compile_error);
    try
    {
        replaced_text = std::regex_replace(text, compiled_regex, replacement);
    }
    catch (const std::bad_alloc &)
    {
        return (FT_ERR_NO_MEMORY);
    }
    catch (...)
    {
        return (FT_ERR_INTERNAL);
    }
    assign_error = output->assign(replaced_text.c_str(), replaced_text.size());
    if (assign_error != FT_ERR_SUCCESS)
        return (assign_error);
    return (FT_ERR_SUCCESS);
}

int32_t regex_capture_count(const char *pattern, ft_size_t *capture_count) noexcept
{
    std::regex compiled_regex;
    int32_t compile_error;

    if (pattern == ft_nullptr || capture_count == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    compile_error = regex_compile_pattern(pattern, &compiled_regex);
    if (compile_error != FT_ERR_SUCCESS)
        return (compile_error);
    *capture_count = compiled_regex.mark_count() + 1U;
    return (FT_ERR_SUCCESS);
}

int32_t regex_capture_groups(const char *pattern, const char *text,
    regex_capture_group *captures, ft_size_t capture_capacity,
    ft_size_t *capture_count) noexcept
{
    std::regex compiled_regex;
    std::cmatch match_result;
    ft_size_t match_count;
    ft_size_t index;
    int32_t compile_error;

    if (pattern == ft_nullptr || text == ft_nullptr || captures == ft_nullptr
        || capture_count == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    compile_error = regex_compile_pattern(pattern, &compiled_regex);
    if (compile_error != FT_ERR_SUCCESS)
        return (compile_error);
    try
    {
        if (std::regex_search(text, match_result, compiled_regex) == false)
        {
            *capture_count = 0U;
            return (FT_ERR_NOT_FOUND);
        }
    }
    catch (const std::bad_alloc &)
    {
        return (FT_ERR_NO_MEMORY);
    }
    catch (...)
    {
        return (FT_ERR_INTERNAL);
    }
    match_count = match_result.size();
    *capture_count = match_count;
    if (capture_capacity < match_count)
        return (FT_ERR_OUT_OF_RANGE);
    index = 0U;
    while (index < match_count)
    {
        if (match_result[index].matched)
            captures[index].matched = FT_TRUE;
        else
            captures[index].matched = FT_FALSE;
        if (captures[index].matched == FT_TRUE)
        {
            captures[index].start_index = match_result.position(index);
            captures[index].length = match_result.length(index);
        }
        else
        {
            captures[index].start_index = 0U;
            captures[index].length = 0U;
        }
        index++;
    }
    return (FT_ERR_SUCCESS);
}
