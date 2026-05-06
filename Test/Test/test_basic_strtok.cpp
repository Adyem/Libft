#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include <thread>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strtok_basic)
{
    char buffer[14] = "one two three";
    char *token;

    token = ft_strtok(buffer, " ");
    FT_ASSERT_EQ(0, ft_strcmp("one", token));
    token = ft_strtok(ft_nullptr, " ");
    FT_ASSERT_EQ(0, ft_strcmp("two", token));
    token = ft_strtok(ft_nullptr, " ");
    FT_ASSERT_EQ(0, ft_strcmp("three", token));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, " "));
    return (1);
}

FT_TEST(test_basic_strtok_edge)
{
    char buffer[4] = ",,,";
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(buffer, ","));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, ","));

    char second[4] = "abc";
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(second, ft_nullptr));
    return (1);
}

FT_TEST(test_basic_strtok_switch_delimiters_same_pointer)
{
    char buffer[32] = "alpha,beta.gamma";
    char delimiter_buffer[3] = {0};
    char *token;

    delimiter_buffer[0] = ',';
    token = ft_strtok(buffer, delimiter_buffer);
    FT_ASSERT_EQ(0, ft_strcmp("alpha", token));
    delimiter_buffer[0] = '.';
    token = ft_strtok(ft_nullptr, delimiter_buffer);
    FT_ASSERT_EQ(0, ft_strcmp("beta", token));
    token = ft_strtok(ft_nullptr, delimiter_buffer);
    FT_ASSERT_EQ(0, ft_strcmp("gamma", token));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, delimiter_buffer));
    return (1);
}

FT_TEST(test_basic_strtok_reinitialize)
{
    char first_string[16] = "alpha beta";
    char second_string[16] = "gamma delta";
    char *token;

    token = ft_strtok(first_string, " ");
    FT_ASSERT_EQ(0, ft_strcmp("alpha", token));
    token = ft_strtok(second_string, " ");
    FT_ASSERT_EQ(0, ft_strcmp("gamma", token));
    token = ft_strtok(ft_nullptr, " ");
    FT_ASSERT_EQ(0, ft_strcmp("delta", token));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, " "));
    return (1);
}

FT_TEST(test_basic_strtok_skips_leading_delimiters)
{
    char buffer[16] = ",,token";
    char *token;

    token = ft_strtok(buffer, ",");
    FT_ASSERT(token != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("token", token));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, ","));
    return (1);
}

FT_TEST(test_basic_strtok_thread_local_state)
{
    char first_thread_buffer[32] = "red blue";
    char second_thread_buffer[32] = "green yellow";
    char *first_thread_token_one = ft_nullptr;
    char *first_thread_token_two = ft_nullptr;
    char *second_thread_token_one = ft_nullptr;
    char *second_thread_token_two = ft_nullptr;

    std::thread first_thread([
    &first_thread_buffer,
    &first_thread_token_one,
    &first_thread_token_two
    ]()
    {
        first_thread_token_one = ft_strtok(first_thread_buffer, " ");
        first_thread_token_two = ft_strtok(ft_nullptr, " ");
        return ;
    });
    std::thread second_thread([
    &second_thread_buffer,
    &second_thread_token_one,
    &second_thread_token_two
    ]()
    {
        second_thread_token_one = ft_strtok(second_thread_buffer, " ");
        second_thread_token_two = ft_strtok(ft_nullptr, " ");
        return ;
    });

    first_thread.join();
    second_thread.join();
    FT_ASSERT_EQ(0, ft_strcmp("red", first_thread_token_one));
    FT_ASSERT_EQ(0, ft_strcmp("blue", first_thread_token_two));
    FT_ASSERT_EQ(0, ft_strcmp("green", second_thread_token_one));
    FT_ASSERT_EQ(0, ft_strcmp("yellow", second_thread_token_two));
    return (1);
}

FT_TEST(test_basic_strtok_empty_delimiter_returns_full_string)
{
    char buffer[16] = "token";
    char *token;

    token = ft_strtok(buffer, "");
    FT_ASSERT(token != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("token", token));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, ""));
    return (1);
}

FT_TEST(test_basic_strtok_recovers_after_null_delimiter)
{
    char buffer[16] = "alpha beta";
    char *token;

    FT_ASSERT_EQ(ft_nullptr, ft_strtok(buffer, ft_nullptr));
    token = ft_strtok(buffer, " ");
    FT_ASSERT(token != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("alpha", token));
    token = ft_strtok(ft_nullptr, " ");
    FT_ASSERT(token != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("beta", token));
    return (1);
}

FT_TEST(test_basic_strtok_trailing_delimiter_yields_single_token)
{
    char buffer[16] = "token,";
    char *token;

    token = ft_strtok(buffer, ",");
    FT_ASSERT(token != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("token", token));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, ","));
    return (1);
}
