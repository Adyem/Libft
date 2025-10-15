#include <atomic>
#include <vector>
#include "../../Libft/libft.hpp"
#include "../../PThread/mutex.hpp"
#include "../../PThread/pthread_lock_tracking.hpp"
#include "../../PThread/pthread.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

struct s_lock_cycle_shared
{
    pt_mutex *first_mutex;
    pt_mutex *second_mutex;
    std::atomic<int> stage;
    std::atomic<int> first_lock_result;
    std::atomic<int> first_lock_error;
    std::atomic<int> second_lock_result;
    std::atomic<int> second_lock_error;
};

static void initialize_shared_state(s_lock_cycle_shared *shared, pt_mutex *first_mutex, pt_mutex *second_mutex)
{
    shared->first_mutex = first_mutex;
    shared->second_mutex = second_mutex;
    shared->stage.store(0);
    shared->first_lock_result.store(0);
    shared->first_lock_error.store(0);
    shared->second_lock_result.store(0);
    shared->second_lock_error.store(0);
    return ;
}

static bool wait_for_stage(std::atomic<int> *stage, int expected_stage)
{
    int attempts;

    attempts = 0;
    while (stage->load() < expected_stage && attempts < 5000)
    {
        pt_thread_sleep(1);
        attempts += 1;
    }
    if (stage->load() < expected_stage)
        return (false);
    return (true);
}

static void *deadlock_worker(void *argument)
{
    s_lock_cycle_shared *shared;

    shared = static_cast<s_lock_cycle_shared *>(argument);
    shared->stage.store(1);
    shared->first_lock_result.store(shared->second_mutex->lock(THREAD_ID));
    shared->first_lock_error.store(shared->second_mutex->get_error());
    shared->stage.store(2);
    shared->second_lock_result.store(shared->first_mutex->lock(THREAD_ID));
    shared->second_lock_error.store(shared->first_mutex->get_error());
    shared->stage.store(3);
    shared->first_mutex->unlock(THREAD_ID);
    shared->stage.store(4);
    shared->second_mutex->unlock(THREAD_ID);
    shared->stage.store(5);
    return (ft_nullptr);
}

FT_TEST(test_pt_lock_tracking_detects_cycle, "pt_lock_tracking prevents circular waits from blocking")
{
    pt_mutex first_mutex;
    pt_mutex second_mutex;
    s_lock_cycle_shared shared;
    pthread_t worker_thread;

    initialize_shared_state(&shared, &first_mutex, &second_mutex);
    FT_ASSERT_EQ(FT_SUCCESS, first_mutex.lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, first_mutex.get_error());
    FT_ASSERT_EQ(0, pt_thread_create(&worker_thread, ft_nullptr, deadlock_worker, &shared));
    FT_ASSERT(wait_for_stage(&shared.stage, 2));
    FT_ASSERT_EQ(FT_SUCCESS, shared.first_lock_result.load());
    FT_ASSERT_EQ(ER_SUCCESS, shared.first_lock_error.load());
    FT_ASSERT_EQ(FT_SUCCESS, second_mutex.lock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, second_mutex.get_error());
    FT_ASSERT(second_mutex.lockState());
    FT_ASSERT_EQ(FT_SUCCESS, first_mutex.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, first_mutex.get_error());
    FT_ASSERT(wait_for_stage(&shared.stage, 5));
    FT_ASSERT_EQ(FT_SUCCESS, shared.second_lock_result.load());
    FT_ASSERT_EQ(ER_SUCCESS, shared.second_lock_error.load());
    FT_ASSERT_EQ(0, pt_thread_join(worker_thread, ft_nullptr));
    FT_ASSERT_EQ(false, second_mutex.lockState());
    return (1);
}

FT_TEST(test_pt_lock_tracking_reports_owned_mutexes, "pt_lock_tracking reports the mutexes owned by a thread")
{
    pt_mutex first_mutex;
    pt_mutex second_mutex;
    std::vector<pthread_mutex_t *> owned_mutexes;

    FT_ASSERT_EQ(FT_SUCCESS, first_mutex.lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, first_mutex.get_error());
    FT_ASSERT_EQ(FT_SUCCESS, second_mutex.lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, second_mutex.get_error());
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(THREAD_ID);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(2, static_cast<int>(owned_mutexes.size()));
    FT_ASSERT_EQ(FT_SUCCESS, second_mutex.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, second_mutex.get_error());
    FT_ASSERT_EQ(FT_SUCCESS, first_mutex.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, first_mutex.get_error());
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(THREAD_ID);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, static_cast<int>(owned_mutexes.size()));
    return (1);
}
