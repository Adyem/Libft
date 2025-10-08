#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <thread>

FT_TEST(test_strtok_basic, "ft_strtok basic")
{
    char buffer[14] = "one two three";
    char *token;

    token = ft_strtok(buffer, " ");
    FT_ASSERT_EQ(0, ft_strcmp("one", token));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    token = ft_strtok(ft_nullptr, " ");
    FT_ASSERT_EQ(0, ft_strcmp("two", token));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    token = ft_strtok(ft_nullptr, " ");
    FT_ASSERT_EQ(0, ft_strcmp("three", token));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, " "));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strtok_edge, "ft_strtok edge")
{
    char buffer[4] = ",,,";
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(buffer, ","));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, ","));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    char second[4] = "abc";
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(second, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strtok_switch_delimiters_same_pointer, "ft_strtok rebuilds when delimiters mutate")
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

FT_TEST(test_strtok_reinitialize, "ft_strtok resets when given a new string")
{
    char first_string[16] = "alpha beta";
    char second_string[16] = "gamma delta";
    char *token;

    token = ft_strtok(first_string, " ");
    FT_ASSERT_EQ(0, ft_strcmp("alpha", token));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    token = ft_strtok(second_string, " ");
    FT_ASSERT_EQ(0, ft_strcmp("gamma", token));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    token = ft_strtok(ft_nullptr, " ");
    FT_ASSERT_EQ(0, ft_strcmp("delta", token));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, " "));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strtok_skips_leading_delimiters, "ft_strtok skips leading separators")
{
    char buffer[16] = ",,token";
    char *token;

    token = ft_strtok(buffer, ",");
    FT_ASSERT(token != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("token", token));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, ","));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strtok_thread_local_state, "ft_strtok maintains thread local state")
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

FT_TEST(test_strtok_empty_delimiter_returns_full_string, "ft_strtok treats empty delimiters as whole token")
{
    char buffer[16] = "token";
    char *token;

    token = ft_strtok(buffer, "");
    FT_ASSERT(token != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("token", token));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, ""));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
