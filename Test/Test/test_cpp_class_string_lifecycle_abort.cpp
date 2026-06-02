#include "../test_internal.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csetjmp>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static sigjmp_buf g_string_uninitialised_abort_jump;

static void string_uninitialised_abort_handler(int /*signal_number*/)
{
    siglongjmp(g_string_uninitialised_abort_jump, 1);
    return ;
}

static int string_expect_sigabrt_signal_handler(void (*operation)(void))
{
    struct sigaction action;
    struct sigaction backup;
    int result;

    std::memset(&action, 0, sizeof(action));
    action.sa_handler = string_uninitialised_abort_handler;
    sigemptyset(&action.sa_mask);
    result = 0;
    if (sigaction(SIGABRT, &action, &backup) != 0)
        return (0);
    if (sigsetjmp(g_string_uninitialised_abort_jump, 1) == 0)
    {
        operation();
        result = 0;
    }
    else
        result = 1;
    (void)sigaction(SIGABRT, &backup, ft_nullptr);
    return (result);
}

static int string_expect_sigabrt_uninitialised(void (*operation)(ft_string &))
{
    struct sigaction action;
    struct sigaction backup;
    int result;

    std::memset(&action, 0, sizeof(action));
    action.sa_handler = string_uninitialised_abort_handler;
    sigemptyset(&action.sa_mask);
    result = 0;
    if (sigaction(SIGABRT, &action, &backup) != 0)
        return (0);
    if (sigsetjmp(g_string_uninitialised_abort_jump, 1) == 0)
    {
        alignas(ft_string) unsigned char storage[sizeof(ft_string)];
        ft_string *string_pointer;

        std::memset(storage, 0, sizeof(storage));
        string_pointer = reinterpret_cast<ft_string *>(storage);
        operation(*string_pointer);
        result = 0;
    }
    else
        result = 1;
    (void)sigaction(SIGABRT, &backup, ft_nullptr);
    return (result);
}

static int string_expect_no_sigabrt_uninitialised(void (*operation)(ft_string &))
{
    struct sigaction action;
    struct sigaction backup;
    int result;

    std::memset(&action, 0, sizeof(action));
    action.sa_handler = string_uninitialised_abort_handler;
    sigemptyset(&action.sa_mask);
    result = 0;
    if (sigaction(SIGABRT, &action, &backup) != 0)
        return (0);
    if (sigsetjmp(g_string_uninitialised_abort_jump, 1) == 0)
    {
        alignas(ft_string) unsigned char storage[sizeof(ft_string)];
        ft_string *string_pointer;

        std::memset(storage, 0, sizeof(storage));
        string_pointer = reinterpret_cast<ft_string *>(storage);
        operation(*string_pointer);
        result = 1;
    }
    else
        result = 0;
    (void)sigaction(SIGABRT, &backup, ft_nullptr);
    return (result);
}

static void string_call_destructor_uninitialised(ft_string &string_value)
{
    string_value.~ft_string();
    return ;
}

static void string_call_destroy_uninitialised(ft_string &string_value)
{
    (void)string_value.destroy();
    return ;
}

static void string_call_append_uninitialised(ft_string &string_value)
{
    (void)string_value.append('x');
    return ;
}

static void string_call_enable_thread_safety_uninitialised(ft_string &string_value)
{
    (void)string_value.enable_thread_safety();
    return ;
}

static void string_call_disable_thread_safety_uninitialised(ft_string &string_value)
{
    (void)string_value.disable_thread_safety();
    return ;
}

static void string_call_is_thread_safe_uninitialised(ft_string &string_value)
{
    (void)string_value.is_thread_safe();
    return ;
}

static void string_call_initialize_twice(void)
{
    ft_string string_value;

    (void)string_value.initialize();
    (void)string_value.initialize();
    return ;
}

static void string_call_destroy_twice(void)
{
    ft_string string_value;

    (void)string_value.initialize();
    (void)string_value.destroy();
    (void)string_value.destroy();
    return ;
}

static void string_call_copy_ctor_from_uninitialised(void)
{
    alignas(ft_string) unsigned char storage[sizeof(ft_string)];
    ft_string *source_pointer;

    std::memset(storage, 0, sizeof(storage));
    source_pointer = reinterpret_cast<ft_string *>(storage);
    ft_string copy_value;
    (void)copy_value.initialize(*source_pointer);
    (void)copy_value;
    return ;
}

static void string_call_move_ctor_from_uninitialised(void)
{
    alignas(ft_string) unsigned char storage[sizeof(ft_string)];
    ft_string *source_pointer;

    std::memset(storage, 0, sizeof(storage));
    source_pointer = reinterpret_cast<ft_string *>(storage);
    ft_string moved_value;
    (void)moved_value.initialize(static_cast<ft_string &&>(*source_pointer));
    (void)moved_value;
    return ;
}

static void string_call_copy_assign_from_uninitialised(void)
{
    alignas(ft_string) unsigned char storage[sizeof(ft_string)];
    ft_string *source_pointer;
    ft_string destination;

    std::memset(storage, 0, sizeof(storage));
    source_pointer = reinterpret_cast<ft_string *>(storage);
    (void)destination.initialize();
    destination = *source_pointer;
    return ;
}

static void string_call_move_assign_from_uninitialised(void)
{
    alignas(ft_string) unsigned char storage[sizeof(ft_string)];
    ft_string *source_pointer;
    ft_string destination;

    std::memset(storage, 0, sizeof(storage));
    source_pointer = reinterpret_cast<ft_string *>(storage);
    (void)destination.initialize();
    destination = static_cast<ft_string &&>(*source_pointer);
    return ;
}

static void string_call_literal_assign_uninitialised(void)
{
    ft_string string_value;

    string_value = "value";
    return ;
}

FT_TEST(test_ft_string_uninitialised_destructor_noop)
{
    FT_ASSERT_EQ(1, string_expect_no_sigabrt_uninitialised(string_call_destructor_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_uninitialised_destroy_noop)
{
    FT_ASSERT_EQ(1, string_expect_no_sigabrt_uninitialised(string_call_destroy_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_uninitialised_append_aborts)
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_uninitialised(string_call_append_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_uninitialised_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_uninitialised(
        string_call_enable_thread_safety_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_uninitialised_disable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_uninitialised(
        string_call_disable_thread_safety_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_uninitialised_is_thread_safe_aborts)
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_uninitialised(
        string_call_is_thread_safe_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_initialize_twice));
    return (1);
}

FT_TEST(test_ft_string_destroy_twice_noops)
{
    FT_ASSERT_EQ(0, string_expect_sigabrt_signal_handler(
        string_call_destroy_twice));
    return (1);
}

FT_TEST(test_ft_string_copy_ctor_from_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_copy_ctor_from_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_move_ctor_from_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_move_ctor_from_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_copy_assign_from_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_copy_assign_from_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_move_assign_from_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_move_assign_from_uninitialised));
    return (1);
}

FT_TEST(test_ft_string_literal_assign_uninitialised_destination_initializes)
{
    FT_ASSERT_EQ(0, string_expect_sigabrt_signal_handler(
        string_call_literal_assign_uninitialised));
    return (1);
}
