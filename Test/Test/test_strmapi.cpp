#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static char to_upper_map(unsigned int index, char character)
{
    (void)index;
    if (character >= 'a' && character <= 'z')
        return (character - 32);
    return (character);
}

static char g_strmapi_index_capture[8];
static unsigned int g_strmapi_call_count = 0;

static char index_to_digit_map(unsigned int index, char character)
{
    (void)character;
    if (g_strmapi_call_count < 8)
    {
        g_strmapi_index_capture[g_strmapi_call_count] = static_cast<char>('0' + index);
        g_strmapi_call_count++;
    }
    return (static_cast<char>('0' + index));
}

FT_TEST(test_strmapi_basic, "ft_strmapi basic")
{
    char *result;

    result = ft_strmapi("ab", to_upper_map);
    if (result == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp("AB", result));
    cma_free(result);

    result = ft_strmapi("", to_upper_map);
    if (result == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp("", result));
    cma_free(result);

    FT_ASSERT_EQ(ft_nullptr, ft_strmapi(ft_nullptr, to_upper_map));
    FT_ASSERT_EQ(ft_nullptr, ft_strmapi("ab", ft_nullptr));
    return (1);
}

FT_TEST(test_strmapi_uses_index_values, "ft_strmapi forwards index values to callback")
{
    char *result;

    g_strmapi_call_count = 0;
    g_strmapi_index_capture[0] = '\0';
    g_strmapi_index_capture[1] = '\0';
    g_strmapi_index_capture[2] = '\0';
    g_strmapi_index_capture[3] = '\0';
    result = ft_strmapi("qrst", index_to_digit_map);
    if (result == ft_nullptr)
        return (0);
    g_strmapi_index_capture[g_strmapi_call_count] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp("0123", result));
    FT_ASSERT_EQ(4u, g_strmapi_call_count);
    FT_ASSERT_EQ(0, ft_strcmp("0123", g_strmapi_index_capture));
    cma_free(result);
    return (1);
}

FT_TEST(test_strmapi_null_arguments_set_errno, "ft_strmapi null inputs set FT_ERR_INVALID_ARGUMENT")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strmapi(ft_nullptr, to_upper_map));
    FT_ASSERT_EQ(ft_nullptr, ft_strmapi("sample", ft_nullptr));
    return (1);
}

FT_TEST(test_strmapi_success_after_failure_resets_errno, "ft_strmapi clears errno after a prior failure")
{
    char *result;

    FT_ASSERT_EQ(ft_nullptr, ft_strmapi(ft_nullptr, to_upper_map));
    result = ft_strmapi("abc", to_upper_map);
    if (result == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp("ABC", result));
    cma_free(result);
    return (1);
}

FT_TEST(test_strmapi_allocates_new_buffer, "ft_strmapi returns a distinct, transformed string")
{
    const char *source;
    char *result;

    source = "copy";
    result = ft_strmapi(source, to_upper_map);
    if (result == ft_nullptr)
        return (0);
    FT_ASSERT(result != source);
    FT_ASSERT_EQ(0, ft_strcmp("COPY", result));
    FT_ASSERT_EQ('c', source[0]);
    cma_free(result);
    return (1);
}
