#include "../test_internal.hpp"
#include "../../Modules/Template/unordered_map.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <cstring>
#include <new>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_unordered_map<int, int> unordered_map_int_int;

static int unordered_map_expect_sigabrt_uninitialised(void (*operation)(unordered_map_int_int&))
{
    return (test_expect_sigabrt_signal_uninitialised<unordered_map_int_int>(operation));
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
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    (void)map_instance.lock(&lock_acquired);
    return ;
}

static void unordered_map_call_unlock(unordered_map_int_int& map_instance)
{
    map_instance.unlock(FT_FALSE);
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

FT_TEST(test_unordered_map_uninitialised_destructor_succeeds)
{
    FT_ASSERT_EQ(0, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_destructor));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_destroy_succeeds)
{
    FT_ASSERT_EQ(0, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_destroy));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_enable_thread_safety));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_disable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_disable_thread_safety));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_is_thread_safe_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_is_thread_safe));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_lock_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_lock));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_unlock_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_unlock));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_insert_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_insert));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_find_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_find));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_find_const_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_find_const));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_erase_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_erase));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_empty_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_empty));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_clear_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_clear));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_size_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_size));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_bucket_count_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_bucket_count));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_has_valid_storage_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_has_valid_storage));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_begin_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_begin));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_end_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_end));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_begin_const_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_begin_const));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_end_const_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_end_const));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_at_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_at));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_at_const_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_at_const));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_operator_index_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_expect_sigabrt_uninitialised(unordered_map_call_operator_index));
    return (1);
}
