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

struct s_unlock_shared_state
{
    pt_mutex *mutex_pointer;
    std::atomic<int> stage;
    std::atomic<int> unlock_result;
    std::atomic<int> unlock_error;
};

struct s_try_lock_shared_state
{
    pt_mutex *mutex_pointer;
    std::atomic<int> stage;
    std::atomic<int> lock_result;
    std::atomic<int> lock_error;
    std::atomic<int> unlock_result;
    std::atomic<int> unlock_error;
};

struct s_foreign_owned_shared_state
{
    pt_mutex *mutex_pointer;
    std::atomic<int> stage;
    std::atomic<int> lock_result;
    std::atomic<int> lock_error;
    std::atomic<int> unlock_result;
    std::atomic<int> unlock_error;
    pt_thread_id_type worker_thread_identifier;
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

static void initialize_unlock_shared_state(s_unlock_shared_state *shared_state, pt_mutex *mutex_pointer)
{
    shared_state->mutex_pointer = mutex_pointer;
    shared_state->stage.store(0);
    shared_state->unlock_result.store(0);
    shared_state->unlock_error.store(0);
    return ;
}

static void *unlock_worker(void *argument)
{
    s_unlock_shared_state *shared_state;

    shared_state = static_cast<s_unlock_shared_state *>(argument);
    shared_state->stage.store(1);
    shared_state->unlock_result.store(shared_state->mutex_pointer->unlock(THREAD_ID));
    shared_state->unlock_error.store(shared_state->mutex_pointer->get_error());
    shared_state->stage.store(2);
    return (ft_nullptr);
}

static void initialize_try_lock_shared_state(s_try_lock_shared_state *shared_state, pt_mutex *mutex_pointer)
{
    shared_state->mutex_pointer = mutex_pointer;
    shared_state->stage.store(0);
    shared_state->lock_result.store(0);
    shared_state->lock_error.store(0);
    shared_state->unlock_result.store(0);
    shared_state->unlock_error.store(0);
    return ;
}

static void initialize_foreign_owned_shared_state(s_foreign_owned_shared_state *shared_state, pt_mutex *mutex_pointer)
{
    shared_state->mutex_pointer = mutex_pointer;
    shared_state->stage.store(0);
    shared_state->lock_result.store(0);
    shared_state->lock_error.store(0);
    shared_state->unlock_result.store(0);
    shared_state->unlock_error.store(0);
    shared_state->worker_thread_identifier = 0;
    return ;
}

static void *try_lock_worker(void *argument)
{
    s_try_lock_shared_state *shared_state;

    shared_state = static_cast<s_try_lock_shared_state *>(argument);
    shared_state->stage.store(1);
    shared_state->lock_result.store(shared_state->mutex_pointer->lock(THREAD_ID));
    shared_state->lock_error.store(shared_state->mutex_pointer->get_error());
    shared_state->stage.store(2);
    while (shared_state->stage.load() < 3)
    {
        pt_thread_sleep(1);
    }
    shared_state->unlock_result.store(shared_state->mutex_pointer->unlock(THREAD_ID));
    shared_state->unlock_error.store(shared_state->mutex_pointer->get_error());
    shared_state->stage.store(4);
    return (ft_nullptr);
}

static void *foreign_owned_mutex_worker(void *argument)
{
    s_foreign_owned_shared_state *shared_state;
    pt_thread_id_type thread_identifier;

    shared_state = static_cast<s_foreign_owned_shared_state *>(argument);
    thread_identifier = THREAD_ID;
    shared_state->worker_thread_identifier = thread_identifier;
    shared_state->stage.store(1);
    shared_state->lock_result.store(shared_state->mutex_pointer->lock(thread_identifier));
    shared_state->lock_error.store(shared_state->mutex_pointer->get_error());
    shared_state->stage.store(2);
    while (shared_state->stage.load() < 3)
    {
        pt_thread_sleep(1);
    }
    shared_state->unlock_result.store(shared_state->mutex_pointer->unlock(thread_identifier));
    shared_state->unlock_error.store(shared_state->mutex_pointer->get_error());
    shared_state->stage.store(4);
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
    pt_mutex_vector owned_mutexes;

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

FT_TEST(test_pt_mutex_unlock_requires_ownership, "pt_mutex unlock fails for threads that do not own the mutex")
{
    pt_mutex mutex_object;
    s_unlock_shared_state shared_state;
    pthread_t worker_thread;

    initialize_unlock_shared_state(&shared_state, &mutex_object);
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT_EQ(0, pt_thread_create(&worker_thread, ft_nullptr, unlock_worker, &shared_state));
    FT_ASSERT(wait_for_stage(&shared_state.stage, 2));
    FT_ASSERT_EQ(FT_SUCCESS, shared_state.unlock_result.load());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, shared_state.unlock_error.load());
    FT_ASSERT_EQ(0, pt_thread_join(worker_thread, ft_nullptr));
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    return (1);
}

FT_TEST(test_pt_mutex_unlock_without_locking, "pt_mutex unlock reports invalid argument when never locked")
{
    pt_mutex mutex_object;

    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, mutex_object.get_error());
    FT_ASSERT_EQ(false, mutex_object.lockState());
    return (1);
}

FT_TEST(test_pt_mutex_try_lock_reports_already_locked, "pt_mutex try_lock reports already locked for reentrant locking")
{
    pt_mutex mutex_object;

    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.try_lock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, mutex_object.get_error());
    FT_ASSERT(mutex_object.lockState());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.try_lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT(mutex_object.lockState());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    return (1);
}

FT_TEST(test_pt_mutex_try_lock_owned_by_other_thread, "pt_mutex try_lock reports already locked when another thread owns the mutex")
{
    pt_mutex mutex_object;
    s_try_lock_shared_state shared_state;
    pthread_t worker_thread;

    initialize_try_lock_shared_state(&shared_state, &mutex_object);
    FT_ASSERT_EQ(0, pt_thread_create(&worker_thread, ft_nullptr, try_lock_worker, &shared_state));
    FT_ASSERT(wait_for_stage(&shared_state.stage, 2));
    FT_ASSERT_EQ(FT_SUCCESS, shared_state.lock_result.load());
    FT_ASSERT_EQ(ER_SUCCESS, shared_state.lock_error.load());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.try_lock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, mutex_object.get_error());
    FT_ASSERT(mutex_object.lockState());
    shared_state.stage.store(3);
    FT_ASSERT(wait_for_stage(&shared_state.stage, 4));
    FT_ASSERT_EQ(FT_SUCCESS, shared_state.unlock_result.load());
    FT_ASSERT_EQ(ER_SUCCESS, shared_state.unlock_error.load());
    FT_ASSERT_EQ(0, pt_thread_join(worker_thread, ft_nullptr));
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT(mutex_object.lockState());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    return (1);
}

FT_TEST(test_pt_mutex_lock_reports_reentrant_lock, "pt_mutex lock reports already locked for the owning thread")
{
    pt_mutex mutex_object;

    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.lock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_MUTEX_ALREADY_LOCKED, mutex_object.get_error());
    FT_ASSERT(mutex_object.lockState());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    return (1);
}

FT_TEST(test_pt_mutex_unlock_twice_reports_invalid_argument, "pt_mutex unlock reports invalid argument when invoked twice")
{
    pt_mutex mutex_object;

    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, mutex_object.get_error());
    FT_ASSERT_EQ(false, mutex_object.lockState());
    return (1);
}

FT_TEST(test_pt_mutex_recovers_after_invalid_unlock, "pt_mutex recovers after invalid unlock attempts")
{
    pt_mutex mutex_object;

    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, mutex_object.get_error());
    FT_ASSERT_EQ(false, mutex_object.lockState());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.lock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT(mutex_object.lockState());
    FT_ASSERT_EQ(FT_SUCCESS, mutex_object.unlock(THREAD_ID));
    FT_ASSERT_EQ(ER_SUCCESS, mutex_object.get_error());
    FT_ASSERT_EQ(false, mutex_object.lockState());
    return (1);
}

FT_TEST(test_pt_lock_tracking_reports_other_thread_mutexes, "pt_lock_tracking reports owned mutexes for another thread")
{
    pt_mutex mutex_object;
    s_foreign_owned_shared_state shared_state;
    pthread_t worker_thread;
    pt_mutex_vector owned_mutexes;

    initialize_foreign_owned_shared_state(&shared_state, &mutex_object);
    FT_ASSERT_EQ(0, pt_thread_create(&worker_thread, ft_nullptr, foreign_owned_mutex_worker, &shared_state));
    FT_ASSERT(wait_for_stage(&shared_state.stage, 2));
    FT_ASSERT_EQ(FT_SUCCESS, shared_state.lock_result.load());
    FT_ASSERT_EQ(ER_SUCCESS, shared_state.lock_error.load());
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(shared_state.worker_thread_identifier);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(1, static_cast<int>(owned_mutexes.size()));
    shared_state.stage.store(3);
    FT_ASSERT(wait_for_stage(&shared_state.stage, 4));
    FT_ASSERT_EQ(FT_SUCCESS, shared_state.unlock_result.load());
    FT_ASSERT_EQ(ER_SUCCESS, shared_state.unlock_error.load());
    FT_ASSERT_EQ(0, pt_thread_join(worker_thread, ft_nullptr));
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(shared_state.worker_thread_identifier);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, static_cast<int>(owned_mutexes.size()));
    return (1);
}
