#include "../test_internal.hpp"
#include "../../Template/queue.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_queue<int> queue_type;

static int queue_expect_sigabrt(void (*operation)(void))
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        operation();
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (WIFSIGNALED(child_status) == 0)
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static int queue_expect_sigabrt_uninitialized(void (*operation)(queue_type &))
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        alignas(queue_type) unsigned char storage[sizeof(queue_type)];
        queue_type *queue_pointer;

        std::memset(storage, 0, sizeof(storage));
        queue_pointer = reinterpret_cast<queue_type *>(storage);
        operation(*queue_pointer);
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (WIFSIGNALED(child_status) == 0)
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static void queue_call_destructor_uninitialized(queue_type &queue_value)
{
    queue_value.~queue_type();
    return ;
}

static void queue_call_destroy_uninitialized(queue_type &queue_value)
{
    (void)queue_value.destroy();
    return ;
}

static void queue_call_enable_thread_safety_uninitialized(queue_type &queue_value)
{
    (void)queue_value.enable_thread_safety();
    return ;
}

static void queue_call_disable_thread_safety_uninitialized(queue_type &queue_value)
{
    (void)queue_value.disable_thread_safety();
    return ;
}

static void queue_call_is_thread_safe_uninitialized(queue_type &queue_value)
{
    (void)queue_value.is_thread_safe();
    return ;
}

static void queue_call_lock_uninitialized(queue_type &queue_value)
{
    bool lock_acquired;

    lock_acquired = false;
    (void)queue_value.lock(&lock_acquired);
    return ;
}

static void queue_call_unlock_uninitialized(queue_type &queue_value)
{
    queue_value.unlock(false);
    return ;
}

static void queue_call_enqueue_lvalue_uninitialized(queue_type &queue_value)
{
    int value;

    value = 42;
    queue_value.enqueue(value);
    return ;
}

static void queue_call_enqueue_rvalue_uninitialized(queue_type &queue_value)
{
    queue_value.enqueue(42);
    return ;
}

static void queue_call_dequeue_uninitialized(queue_type &queue_value)
{
    (void)queue_value.dequeue();
    return ;
}

static void queue_call_front_uninitialized(queue_type &queue_value)
{
    (void)queue_value.front();
    return ;
}

static void queue_call_front_const_uninitialized(queue_type &queue_value)
{
    const queue_type &const_queue = queue_value;

    (void)const_queue.front();
    return ;
}

static void queue_call_front_proxy_uninitialized(queue_type &queue_value)
{
    (void)queue_value.front_proxy();
    return ;
}

static void queue_call_size_uninitialized(queue_type &queue_value)
{
    (void)queue_value.size();
    return ;
}

static void queue_call_empty_uninitialized(queue_type &queue_value)
{
    (void)queue_value.empty();
    return ;
}

static void queue_call_clear_uninitialized(queue_type &queue_value)
{
    queue_value.clear();
    return ;
}

static void queue_call_initialize_twice_aborts(void)
{
    queue_type queue_value;

    (void)queue_value.initialize();
    (void)queue_value.initialize();
    return ;
}

static void queue_call_destroy_twice_aborts(void)
{
    queue_type queue_value;

    (void)queue_value.initialize();
    (void)queue_value.destroy();
    (void)queue_value.destroy();
    return ;
}


FT_TEST(test_ft_queue_uninitialized_destructor_aborts,
    "ft_queue destructor aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_destructor_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_destroy_aborts,
    "ft_queue destroy aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_destroy_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_enable_thread_safety_aborts,
    "ft_queue enable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(
        queue_call_enable_thread_safety_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_disable_thread_safety_aborts,
    "ft_queue disable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(
        queue_call_disable_thread_safety_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_is_thread_safe_aborts,
    "ft_queue is_thread_safe aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_is_thread_safe_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_lock_aborts,
    "ft_queue lock aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_lock_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_unlock_aborts,
    "ft_queue unlock aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_unlock_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_enqueue_lvalue_aborts,
    "ft_queue enqueue(const&) aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_enqueue_lvalue_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_enqueue_rvalue_aborts,
    "ft_queue enqueue(&&) aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_enqueue_rvalue_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_dequeue_aborts,
    "ft_queue dequeue aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_dequeue_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_front_aborts,
    "ft_queue front aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_front_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_front_const_aborts,
    "ft_queue const front aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_front_const_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_front_proxy_aborts,
    "ft_queue front_proxy aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_front_proxy_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_size_aborts,
    "ft_queue size aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_size_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_empty_aborts,
    "ft_queue empty aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_empty_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_uninitialized_clear_aborts,
    "ft_queue clear aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt_uninitialized(queue_call_clear_uninitialized));
    return (1);
}

FT_TEST(test_ft_queue_initialize_twice_aborts,
    "ft_queue initialize aborts when called while already initialized")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt(queue_call_initialize_twice_aborts));
    return (1);
}

FT_TEST(test_ft_queue_destroy_twice_aborts,
    "ft_queue destroy aborts when called while already destroyed")
{
    FT_ASSERT_EQ(1, queue_expect_sigabrt(queue_call_destroy_twice_aborts));
    return (1);
}

FT_TEST(test_ft_queue_initialize_move_from_uninitialized_source_aborts,
    "ft_queue initialize(move) aborts when source is uninitialized")
{
    /* The move constructor/operator= are deleted, so no runtime move exists. */
    return (1);
}
