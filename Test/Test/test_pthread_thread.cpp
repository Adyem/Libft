#include "../../PThread/pthread.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

static void *pthread_test_routine(void *argument)
{
    int *started_flag;

    started_flag = static_cast<int*>(argument);
    if (started_flag != ft_nullptr)
        *started_flag = 1;
    return (ft_nullptr);
}

FT_TEST(test_pt_thread_create_updates_errno, "pt_thread_create updates ft_errno on failure and success")
{
    pthread_t thread;
    int failure_result;
    int success_result;
    int routine_started;
    pthread_attr_t attributes;
    size_t huge_stack_size;
    int set_stack_result;

    routine_started = 0;
    pthread_attr_init(&attributes);
    huge_stack_size = static_cast<size_t>(1ULL) << 40;
    set_stack_result = pthread_attr_setstacksize(&attributes, huge_stack_size);
    FT_ASSERT_EQ(0, set_stack_result);
    failure_result = pt_thread_create(&thread, &attributes, pthread_test_routine, &routine_started);
    FT_ASSERT(failure_result != 0);
    FT_ASSERT_EQ(ft_map_system_error(failure_result), ft_errno);
    pthread_attr_destroy(&attributes);
    success_result = pt_thread_create(&thread, ft_nullptr, pthread_test_routine, &routine_started);
    int join_result;
    int thread_started;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    join_result = 0;
    thread_started = 0;
    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (success_result != 0)
    {
        test_failed = 1;
        failure_expression = "success_result == 0";
        failure_line = __LINE__;
    }
    else
    {
        thread_started = 1;
        if (ft_errno != ER_SUCCESS)
        {
            test_failed = 1;
            failure_expression = "ft_errno == ER_SUCCESS";
            failure_line = __LINE__;
        }
    }
    if (thread_started == 1)
    {
        join_result = pt_thread_join(thread, ft_nullptr);
        if (join_result != 0 && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "join_result == 0";
            failure_line = __LINE__;
        }
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    FT_ASSERT_EQ(1, routine_started);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_pt_thread_join_updates_errno, "pt_thread_join updates ft_errno on failure and success")
{
    pthread_t invalid_thread;
    pthread_t thread;
    int failure_result;
    int routine_started;
    int join_result;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    invalid_thread = 0;
    failure_result = pt_thread_join(invalid_thread, ft_nullptr);
    FT_ASSERT(failure_result != 0);
    FT_ASSERT_EQ(ft_map_system_error(failure_result), ft_errno);
    routine_started = 0;
    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (pt_thread_create(&thread, ft_nullptr, pthread_test_routine, &routine_started) != 0)
    {
        ft_test_fail("pt_thread_create(&thread, ft_nullptr, pthread_test_routine, &routine_started) == 0", __FILE__, __LINE__);
        return (0);
    }
    join_result = pt_thread_join(thread, ft_nullptr);
    if (join_result != 0)
    {
        test_failed = 1;
        failure_expression = "join_result == 0";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    FT_ASSERT_EQ(1, routine_started);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_pt_thread_detach_updates_errno, "pt_thread_detach updates ft_errno on failure and success")
{
    pthread_t invalid_thread;
    pthread_t thread;
    int failure_result;
    int detach_result;
    int routine_started;

    invalid_thread = 0;
    failure_result = pt_thread_detach(invalid_thread);
    FT_ASSERT(failure_result != 0);
    FT_ASSERT_EQ(ft_map_system_error(failure_result), ft_errno);
    routine_started = 0;
    FT_ASSERT_EQ(0, pt_thread_create(&thread, ft_nullptr, pthread_test_routine, &routine_started));
    detach_result = pt_thread_detach(thread);
    FT_ASSERT_EQ(0, detach_result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    while (routine_started == 0)
        pt_thread_sleep(10);
    return (1);
}
