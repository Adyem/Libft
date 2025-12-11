#include <errno.h>
#include <time.h>
#include "../../Libft/libft.hpp"
#include "../../PThread/pthread.hpp"
#include "../../PThread/mutex.hpp"
#include "../../PThread/condition.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

struct s_try_lock_for_state
{
    pt_mutex *mutex_pointer;
    int lock_result;
    int lock_error;
    int unlock_result;
    int unlock_error;
};

static void *pt_mutex_try_lock_for_thread(void *argument)
{
    s_try_lock_for_state *state;
    struct timespec relative_time;

    state = static_cast<s_try_lock_for_state*>(argument);
    relative_time.tv_sec = 0;
    relative_time.tv_nsec = 50000000L;
    state->lock_result = state->mutex_pointer->try_lock_for(THREAD_ID, relative_time);
    state->lock_error = state->mutex_pointer->get_error();
    if (state->lock_result == FT_SUCCESS)
    {
        state->unlock_result = state->mutex_pointer->unlock(THREAD_ID);
        state->unlock_error = state->mutex_pointer->get_error();
    }
    else
    {
        state->unlock_result = FT_SUCCESS;
        state->unlock_error = FT_ERR_SUCCESSS;
    }
    return (ft_nullptr);
}

FT_TEST(test_pt_mutex_try_lock_for_times_out_when_mutex_locked, "pt_mutex try_lock_for reports timeout when mutex is owned")
{
    pt_mutex mutex_object;
    pthread_t worker_thread;
    s_try_lock_for_state state;

    state.mutex_pointer = &mutex_object;
    state.lock_result = 0;
    state.lock_error = 0;
    state.unlock_result = 0;
    state.unlock_error = 0;
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.lock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, mutex_object.get_error());
    FT_ASSERT_EQ(0, pthread_create(&worker_thread, ft_nullptr, pt_mutex_try_lock_for_thread, &state));
    FT_ASSERT_EQ(0, pthread_join(worker_thread, ft_nullptr));
    FT_ASSERT_EQ(ETIMEDOUT, state.lock_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, state.lock_error);
    FT_ASSERT_EQ(FT_SUCCESS, state.unlock_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, state.unlock_error);
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, mutex_object.get_error());
    return (1);
}

FT_TEST(test_pt_mutex_try_lock_until_acquires_unlocked_mutex, "pt_mutex try_lock_until acquires the mutex when available")
{
    pt_mutex mutex_object;
    struct timespec absolute_time;
    int lock_result;

    FT_ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME, &absolute_time));
    absolute_time.tv_sec += 1;
    lock_result = mutex_object.try_lock_until(THREAD_ID, absolute_time);
    FT_ASSERT_EQ(FT_SUCCESS, lock_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, mutex_object.get_error());
    FT_ASSERT(mutex_object.lockState());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, mutex_object.get_error());
    return (1);
}

FT_TEST(test_pt_condition_variable_wait_for_times_out_without_signal, "pt_condition_variable wait_for times out without a signal")
{
    pt_condition_variable condition_variable;
    pt_mutex mutex_object;
    struct timespec relative_time;
    int wait_result;

    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.lock(THREAD_ID));
    relative_time.tv_sec = 0;
    relative_time.tv_nsec = 20000000L;
    wait_result = condition_variable.wait_for(mutex_object, relative_time);
    FT_ASSERT_EQ(ETIMEDOUT, wait_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, condition_variable.get_error());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, mutex_object.get_error());
    return (1);
}

