#include "../../Template/function.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_function_copy_constructor_recreates_mutex,
        "ft_function copy constructor rebuilds mutex for thread-safe callable")
{
    ft_function<int()> source_function([]() { return (42); });
    bool source_lock_acquired;
    bool copy_lock_acquired;
    void *source_mutex_address;
    void *copy_mutex_address;

    source_lock_acquired = false;
    copy_lock_acquired = false;
    source_mutex_address = ft_nullptr;
    copy_mutex_address = ft_nullptr;
    FT_ASSERT_EQ(0, source_function.enable_thread_safety());
    FT_ASSERT(source_function.is_thread_safe_enabled());
    source_mutex_address = source_function.get_mutex_address_debug();
    ft_function<int()> copied_function(source_function);
    FT_ASSERT(copied_function.is_thread_safe_enabled());
    copy_mutex_address = copied_function.get_mutex_address_debug();
    FT_ASSERT_NE(ft_nullptr, source_mutex_address);
    FT_ASSERT_NE(ft_nullptr, copy_mutex_address);
    FT_ASSERT_NE(source_mutex_address, copy_mutex_address);
    FT_ASSERT_EQ(0, source_function.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    FT_ASSERT_EQ(0, copied_function.lock(&copy_lock_acquired));
    FT_ASSERT(copy_lock_acquired);
    FT_ASSERT_EQ(42, source_function());
    FT_ASSERT_EQ(42, copied_function());
    copied_function.unlock(copy_lock_acquired);
    source_function.unlock(source_lock_acquired);
    return (1);
}

FT_TEST(test_ft_function_copy_assignment_reinitializes_mutex,
        "ft_function copy assignment installs fresh mutex while copying state")
{
    ft_function<int()> source_function([]() { return (7); });
    ft_function<int()> destination_function([]() { return (1); });
    void *destination_mutex_before;
    void *destination_mutex_after;
    void *source_mutex_address;

    destination_mutex_before = ft_nullptr;
    destination_mutex_after = ft_nullptr;
    source_mutex_address = ft_nullptr;
    FT_ASSERT_EQ(0, source_function.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_function.enable_thread_safety());
    FT_ASSERT(source_function.is_thread_safe_enabled());
    FT_ASSERT(destination_function.is_thread_safe_enabled());
    destination_mutex_before = destination_function.get_mutex_address_debug();
    source_mutex_address = source_function.get_mutex_address_debug();
    destination_function = source_function;
    destination_mutex_after = destination_function.get_mutex_address_debug();
    FT_ASSERT(destination_function.is_thread_safe_enabled());
    FT_ASSERT(source_function.is_thread_safe_enabled());
    FT_ASSERT_NE(ft_nullptr, destination_mutex_before);
    FT_ASSERT_NE(ft_nullptr, destination_mutex_after);
    FT_ASSERT_NE(destination_mutex_before, destination_mutex_after);
    FT_ASSERT_NE(source_mutex_address, destination_mutex_after);
    FT_ASSERT_EQ(7, destination_function());
    FT_ASSERT_EQ(7, source_function());
    return (1);
}

FT_TEST(test_ft_function_move_constructor_resets_source_mutex,
        "ft_function move constructor recreates mutex and clears source guard")
{
    ft_function<int()> source_function([]() { return (31); });
    bool moved_lock_acquired;

    moved_lock_acquired = false;
    FT_ASSERT_EQ(0, source_function.enable_thread_safety());
    FT_ASSERT(source_function.is_thread_safe_enabled());
    ft_function<int()> moved_function(ft_move(source_function));
    FT_ASSERT(moved_function.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_function.is_thread_safe_enabled());
    FT_ASSERT_EQ(31, moved_function());
    FT_ASSERT_EQ(0, moved_function.lock(&moved_lock_acquired));
    FT_ASSERT(moved_lock_acquired);
    moved_function.unlock(moved_lock_acquired);
    FT_ASSERT_EQ(0, source_function.enable_thread_safety());
    FT_ASSERT(source_function.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_ft_function_move_assignment_rebuilds_mutex,
        "ft_function move assignment recreates mutex and leaves source reusable")
{
    ft_function<int()> source_function([]() { return (55); });
    ft_function<int()> destination_function([]() { return (12); });
    void *destination_mutex_before;
    void *destination_mutex_after;

    destination_mutex_before = ft_nullptr;
    destination_mutex_after = ft_nullptr;
    FT_ASSERT_EQ(0, source_function.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_function.enable_thread_safety());
    FT_ASSERT(source_function.is_thread_safe_enabled());
    FT_ASSERT(destination_function.is_thread_safe_enabled());
    destination_mutex_before = destination_function.get_mutex_address_debug();
    destination_function = ft_move(source_function);
    destination_mutex_after = destination_function.get_mutex_address_debug();
    FT_ASSERT(destination_function.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_function.is_thread_safe_enabled());
    FT_ASSERT_NE(ft_nullptr, destination_mutex_before);
    FT_ASSERT_NE(ft_nullptr, destination_mutex_after);
    FT_ASSERT_NE(destination_mutex_before, destination_mutex_after);
    FT_ASSERT_EQ(55, destination_function());
    FT_ASSERT_EQ(0, source_function.enable_thread_safety());
    FT_ASSERT(source_function.is_thread_safe_enabled());
    return (1);
}
