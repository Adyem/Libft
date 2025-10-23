#include "../../PThread/pthread.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

struct pt_condition_signal_data
{
    pthread_cond_t *condition;
    pthread_mutex_t *mutex;
    int *ready;
};

static void *pt_condition_signal_thread(void *argument)
{
    pt_condition_signal_data *data;

    data = static_cast<pt_condition_signal_data*>(argument);
    pthread_mutex_lock(data->mutex);
    *(data->ready) = 1;
    pthread_cond_signal(data->condition);
    pthread_mutex_unlock(data->mutex);
    return (ft_nullptr);
}

FT_TEST(test_pt_cond_wait_updates_errno, "pt_cond_wait updates ft_errno on failure and success")
{
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    pt_condition_signal_data data;
    pthread_t thread_identifier;
    int failure_result;
    int wait_result;
    int ready;

    FT_ASSERT_EQ(0, pthread_mutex_init(&mutex, ft_nullptr));
    failure_result = pt_cond_wait(ft_nullptr, &mutex);
    FT_ASSERT_EQ(-1, failure_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(0, pt_cond_init(&condition, ft_nullptr));
    failure_result = pt_cond_wait(&condition, ft_nullptr);
    FT_ASSERT_EQ(-1, failure_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ready = 0;
    data.condition = &condition;
    data.mutex = &mutex;
    data.ready = &ready;
    FT_ASSERT_EQ(0, pthread_mutex_lock(&mutex));
    FT_ASSERT_EQ(0, pthread_create(&thread_identifier, ft_nullptr, pt_condition_signal_thread, &data));
    while (ready == 0)
    {
        wait_result = pt_cond_wait(&condition, &mutex);
        FT_ASSERT_EQ(0, wait_result);
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    }
    int test_failed;
    const char *failure_expression;
    int failure_line;
    int unlock_result;
    int join_result;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    unlock_result = pthread_mutex_unlock(&mutex);
    if (unlock_result != 0 && test_failed == 0)
    {
        test_failed = 1;
        failure_expression = "unlock_result == 0";
        failure_line = __LINE__;
    }
    join_result = pthread_join(thread_identifier, ft_nullptr);
    if (join_result != 0 && test_failed == 0)
    {
        test_failed = 1;
        failure_expression = "join_result == 0";
        failure_line = __LINE__;
    }
    FT_ASSERT_EQ(0, pt_cond_destroy(&condition));
    FT_ASSERT_EQ(0, pthread_mutex_destroy(&mutex));
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
