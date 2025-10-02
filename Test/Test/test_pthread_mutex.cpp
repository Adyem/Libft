#include "../../PThread/mutex.hpp"
#include "../../PThread/pthread.hpp"
#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

static void *capture_thread_identifier(void *argument)
{
    pthread_t *destination;

    destination = static_cast<pthread_t *>(argument);
    *destination = pthread_self();
    return (ft_nullptr);
}

FT_TEST(test_pt_mutex_try_lock_initial_success, "pt_mutex::try_lock acquires an unlocked mutex")
{
    pt_mutex mutex;
    pthread_t thread_identifier;
    int try_result;
    int unlock_result;

    thread_identifier = pthread_self();
    try_result = mutex.try_lock(thread_identifier);
    FT_ASSERT_EQ(FT_SUCCESS, try_result);
    FT_ASSERT_EQ(ER_SUCCESS, mutex.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(mutex.lockState());
    unlock_result = mutex.unlock(thread_identifier);
    FT_ASSERT_EQ(FT_SUCCESS, unlock_result);
    FT_ASSERT_EQ(ER_SUCCESS, mutex.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(!mutex.lockState());
    return (1);
}

FT_TEST(test_pt_mutex_try_lock_detects_foreign_owner, "pt_mutex::try_lock rejects locks held by other threads")
{
    pt_mutex mutex;
    pthread_t owning_thread_identifier;
    pthread_t requesting_thread_identifier;
    pthread_t helper_thread_identifier;
    int lock_result;
    int try_result;
    int unlock_result;

    requesting_thread_identifier = pthread_self();
    owning_thread_identifier = 0;
    FT_ASSERT_EQ(0, pthread_create(&helper_thread_identifier, ft_nullptr, capture_thread_identifier, &owning_thread_identifier));
    FT_ASSERT_EQ(0, pthread_join(helper_thread_identifier, ft_nullptr));
    FT_ASSERT(pthread_equal(requesting_thread_identifier, owning_thread_identifier) == 0);
    lock_result = mutex.lock(owning_thread_identifier);
    FT_ASSERT_EQ(FT_SUCCESS, lock_result);
    FT_ASSERT(mutex.lockState());
    try_result = mutex.try_lock(requesting_thread_identifier);
    FT_ASSERT_EQ(PT_ERR_ALREADY_LOCKED, try_result);
    FT_ASSERT_EQ(PT_ERR_ALREADY_LOCKED, mutex.get_error());
    FT_ASSERT_EQ(PT_ERR_ALREADY_LOCKED, ft_errno);
    FT_ASSERT(mutex.lockState());
    unlock_result = mutex.unlock(owning_thread_identifier);
    FT_ASSERT_EQ(FT_SUCCESS, unlock_result);
    FT_ASSERT_EQ(ER_SUCCESS, mutex.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(!mutex.lockState());
    return (1);
}

FT_TEST(test_pt_mutex_try_lock_rejects_reentrant_attempt, "pt_mutex::try_lock blocks reentrant acquisition by owner")
{
    pt_mutex mutex;
    pthread_t thread_identifier;
    int lock_result;
    int try_result;
    int unlock_result;

    thread_identifier = pthread_self();
    lock_result = mutex.lock(thread_identifier);
    FT_ASSERT_EQ(FT_SUCCESS, lock_result);
    FT_ASSERT(mutex.lockState());
    try_result = mutex.try_lock(thread_identifier);
    FT_ASSERT_EQ(-1, try_result);
    FT_ASSERT_EQ(PT_ERR_ALREADY_LOCKED, mutex.get_error());
    FT_ASSERT_EQ(PT_ERR_ALREADY_LOCKED, ft_errno);
    FT_ASSERT(mutex.lockState());
    unlock_result = mutex.unlock(thread_identifier);
    FT_ASSERT_EQ(FT_SUCCESS, unlock_result);
    FT_ASSERT_EQ(ER_SUCCESS, mutex.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(!mutex.lockState());
    return (1);
}

FT_TEST(test_pt_mutex_unlock_requires_ownership, "pt_mutex::unlock enforces ownership before releasing")
{
    pt_mutex mutex;
    pthread_t owning_thread_identifier;
    pthread_t foreign_thread_identifier;
    pthread_t helper_thread_identifier;
    int lock_result;
    int unlock_result;

    owning_thread_identifier = pthread_self();
    lock_result = mutex.lock(owning_thread_identifier);
    FT_ASSERT_EQ(FT_SUCCESS, lock_result);
    FT_ASSERT(mutex.lockState());
    foreign_thread_identifier = 0;
    FT_ASSERT_EQ(0, pthread_create(&helper_thread_identifier, ft_nullptr, capture_thread_identifier, &foreign_thread_identifier));
    FT_ASSERT_EQ(0, pthread_join(helper_thread_identifier, ft_nullptr));
    FT_ASSERT(pthread_equal(owning_thread_identifier, foreign_thread_identifier) == 0);
    unlock_result = mutex.unlock(foreign_thread_identifier);
    FT_ASSERT_EQ(-1, unlock_result);
    FT_ASSERT_EQ(PT_ERR_MUTEX_OWNER, mutex.get_error());
    FT_ASSERT_EQ(PT_ERR_MUTEX_OWNER, ft_errno);
    FT_ASSERT(mutex.lockState());
    unlock_result = mutex.unlock(owning_thread_identifier);
    FT_ASSERT_EQ(FT_SUCCESS, unlock_result);
    FT_ASSERT_EQ(ER_SUCCESS, mutex.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(!mutex.lockState());
    return (1);
}
