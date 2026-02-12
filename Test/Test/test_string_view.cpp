#include "../test_internal.hpp"
#include "../../Template/string_view.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"
#include "../../PThread/pthread.hpp"
#include <atomic>

#ifndef LIBFT_TEST_BUILD
#endif

struct string_view_thread_data
{
    ft_string_view<char> *shared;
    const ft_string_view<char> *source;
    std::atomic<bool> *running;
};

static void *string_view_assignment_worker(void *argument)
{
    string_view_thread_data *data;
    size_t index;

    data = static_cast<string_view_thread_data*>(argument);
    index = 0;
    while (index < 1000)
    {
        *(data->shared) = *(data->source);
        index += 1;
    }
    data->running->store(false, std::memory_order_relaxed);
    return (ft_nullptr);
}

static void *string_view_comparison_worker(void *argument)
{
    string_view_thread_data *data;

    data = static_cast<string_view_thread_data*>(argument);
    while (data->running->load(std::memory_order_relaxed))
    {
        data->shared->compare(*(data->source));
    }
    return (ft_nullptr);
}

FT_TEST(test_string_view_basic, "ft_string_view basic")
{
    ft_string_view<char> view("hello");
    FT_ASSERT_EQ(5u, view.size());
    FT_ASSERT_EQ('h', view.data()[0]);
    return (1);
}

FT_TEST(test_string_view_default_constructed_empty, "ft_string_view default constructor yields empty view")
{
    ft_string_view<char> view;

    FT_ASSERT_EQ(0u, view.size());
    FT_ASSERT(view.empty());
    return (1);
}

FT_TEST(test_string_view_compare_substr, "ft_string_view compare and substr")
{
    ft_string_view<char> view("hello");
    ft_string_view<char> other_view("hello");
    FT_ASSERT_EQ(0, view.compare(other_view));
    ft_string_view<char> substring = view.substr(1, 3);
    ft_string_view<char> expected_view("ell");
    FT_ASSERT_EQ(0, substring.compare(expected_view));
    return (1);
}

FT_TEST(test_string_view_substr_oob, "ft_string_view substr out of bounds")
{
    char    *buffer;
    buffer = static_cast<char*>(cma_malloc(6));
    buffer[0] = 'w';
    buffer[1] = 'o';
    buffer[2] = 'r';
    buffer[3] = 'l';
    buffer[4] = 'd';
    buffer[5] = '\0';
    ft_string_view<char> view(buffer);
    ft_string_view<char> substring = view.substr(10, 2);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, view.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, substring.get_error());
    FT_ASSERT_EQ(0u, substring.size());
    cma_free(buffer);
    return (1);
}

FT_TEST(test_string_view_thread_safety, "ft_string_view thread safe copy and compare")
{
    ft_string_view<char> shared("initial");
    ft_string_view<char> source("updated");
    std::atomic<bool> running(true);
    string_view_thread_data data;
    pthread_t assign_thread;
    pthread_t compare_thread;

    data.shared = &shared;
    data.source = &source;
    data.running = &running;
    if (pt_thread_create(&assign_thread, ft_nullptr,
            string_view_assignment_worker, &data) != 0)
        return (0);
    if (pt_thread_create(&compare_thread, ft_nullptr,
            string_view_comparison_worker, &data) != 0)
    {
        pt_thread_join(assign_thread, ft_nullptr);
        return (0);
    }
    pt_thread_join(assign_thread, ft_nullptr);
    pt_thread_join(compare_thread, ft_nullptr);
    FT_ASSERT_EQ(0, shared.compare(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shared.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}
