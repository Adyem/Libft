#include "../test_internal.hpp"
#include "../../Template/unordered_map.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_unordered_map<int, int> unordered_map_int_int;

static int unordered_map_expect_sigabrt_uninitialized(void (*operation)(unordered_map_int_int&))
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        alignas(unordered_map_int_int) unsigned char storage[sizeof(unordered_map_int_int)];
        unordered_map_int_int *map_pointer;

        std::memset(storage, 0, sizeof(storage));
        map_pointer = reinterpret_cast<unordered_map_int_int *>(storage);
        operation(*map_pointer);
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (!WIFSIGNALED(child_status))
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static void unordered_map_call_destructor(unordered_map_int_int& map_instance)
{
    map_instance.~unordered_map_int_int();
    return ;
}

static void unordered_map_call_destroy(unordered_map_int_int& map_instance)
{
    (void)map_instance.destroy();
    return ;
}

static void unordered_map_call_enable_thread_safety(unordered_map_int_int& map_instance)
{
    (void)map_instance.enable_thread_safety();
    return ;
}

static void unordered_map_call_disable_thread_safety(unordered_map_int_int& map_instance)
{
    (void)map_instance.disable_thread_safety();
    return ;
}

static void unordered_map_call_is_thread_safe(unordered_map_int_int& map_instance)
{
    (void)map_instance.is_thread_safe();
    return ;
}

static void unordered_map_call_lock(unordered_map_int_int& map_instance)
{
    bool lock_acquired;

    lock_acquired = false;
    (void)map_instance.lock(&lock_acquired);
    return ;
}

static void unordered_map_call_unlock(unordered_map_int_int& map_instance)
{
    map_instance.unlock(false);
    return ;
}

static void unordered_map_call_insert(unordered_map_int_int& map_instance)
{
    map_instance.insert(1, 1);
    return ;
}

static void unordered_map_call_find(unordered_map_int_int& map_instance)
{
    (void)map_instance.find(1);
    return ;
}

static void unordered_map_call_find_const(unordered_map_int_int& map_instance)
{
    const unordered_map_int_int *const_map_pointer;

    const_map_pointer = &map_instance;
    (void)const_map_pointer->find(1);
    return ;
}

static void unordered_map_call_erase(unordered_map_int_int& map_instance)
{
    map_instance.erase(1);
    return ;
}

static void unordered_map_call_empty(unordered_map_int_int& map_instance)
{
    (void)map_instance.empty();
    return ;
}

static void unordered_map_call_clear(unordered_map_int_int& map_instance)
{
    map_instance.clear();
    return ;
}

static void unordered_map_call_size(unordered_map_int_int& map_instance)
{
    (void)map_instance.size();
    return ;
}

static void unordered_map_call_bucket_count(unordered_map_int_int& map_instance)
{
    (void)map_instance.bucket_count();
    return ;
}

static void unordered_map_call_has_valid_storage(unordered_map_int_int& map_instance)
{
    (void)map_instance.has_valid_storage();
    return ;
}

static void unordered_map_call_begin(unordered_map_int_int& map_instance)
{
    (void)map_instance.begin();
    return ;
}

static void unordered_map_call_end(unordered_map_int_int& map_instance)
{
    (void)map_instance.end();
    return ;
}

static void unordered_map_call_begin_const(unordered_map_int_int& map_instance)
{
    const unordered_map_int_int *const_map_pointer;

    const_map_pointer = &map_instance;
    (void)const_map_pointer->begin();
    return ;
}

static void unordered_map_call_end_const(unordered_map_int_int& map_instance)
{
    const unordered_map_int_int *const_map_pointer;

    const_map_pointer = &map_instance;
    (void)const_map_pointer->end();
    return ;
}

static void unordered_map_call_at(unordered_map_int_int& map_instance)
{
    (void)map_instance.at(1);
    return ;
}

static void unordered_map_call_at_const(unordered_map_int_int& map_instance)
{
    const unordered_map_int_int *const_map_pointer;

    const_map_pointer = &map_instance;
    (void)const_map_pointer->at(1);
    return ;
}

static void unordered_map_call_operator_index(unordered_map_int_int& map_instance)
{
    (void)map_instance[1];
    return ;
}

FT_TEST(test_unordered_map_uninitialized_destructor_aborts,
    "unordered_map destructor aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_destructor));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_destroy_aborts,
    "unordered_map destroy aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_destroy));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_enable_thread_safety_aborts,
    "unordered_map enable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_enable_thread_safety));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_disable_thread_safety_aborts,
    "unordered_map disable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_disable_thread_safety));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_is_thread_safe_aborts,
    "unordered_map is_thread_safe aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_is_thread_safe));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_lock_aborts,
    "unordered_map lock aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_lock));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_unlock_aborts,
    "unordered_map unlock aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_unlock));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_insert_aborts,
    "unordered_map insert aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_insert));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_find_aborts,
    "unordered_map find aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_find));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_find_const_aborts,
    "unordered_map const find aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_find_const));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_erase_aborts,
    "unordered_map erase aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_erase));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_empty_aborts,
    "unordered_map empty aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_empty));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_clear_aborts,
    "unordered_map clear aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_clear));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_size_aborts,
    "unordered_map size aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_size));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_bucket_count_aborts,
    "unordered_map bucket_count aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_bucket_count));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_has_valid_storage_aborts,
    "unordered_map has_valid_storage aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_has_valid_storage));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_begin_aborts,
    "unordered_map begin aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_begin));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_end_aborts,
    "unordered_map end aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_end));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_begin_const_aborts,
    "unordered_map const begin aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_begin_const));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_end_const_aborts,
    "unordered_map const end aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_end_const));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_at_aborts,
    "unordered_map at aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_at));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_at_const_aborts,
    "unordered_map const at aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_at_const));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_operator_index_aborts,
    "unordered_map operator[] aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialized(unordered_map_call_operator_index));
    return (1);
}
