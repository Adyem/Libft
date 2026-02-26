#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csetjmp>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static sigjmp_buf g_string_uninitialized_abort_jump;

static void string_uninitialized_abort_handler(int /*signal_number*/)
{
    siglongjmp(g_string_uninitialized_abort_jump, 1);
    return ;
}

static int string_expect_sigabrt_signal_handler(void (*operation)(void))
{
    struct sigaction action;
    struct sigaction backup;
    int result;

    std::memset(&action, 0, sizeof(action));
    action.sa_handler = string_uninitialized_abort_handler;
    sigemptyset(&action.sa_mask);
    result = 0;
    if (sigaction(SIGABRT, &action, &backup) != 0)
        return (0);
    if (sigsetjmp(g_string_uninitialized_abort_jump, 1) == 0)
    {
        operation();
        result = 0;
    }
    else
        result = 1;
    (void)sigaction(SIGABRT, &backup, ft_nullptr);
    return (result);
}

static int string_expect_sigabrt_uninitialized(void (*operation)(ft_string &))
{
    struct sigaction action;
    struct sigaction backup;
    int result;

    std::memset(&action, 0, sizeof(action));
    action.sa_handler = string_uninitialized_abort_handler;
    sigemptyset(&action.sa_mask);
    result = 0;
    if (sigaction(SIGABRT, &action, &backup) != 0)
        return (0);
    if (sigsetjmp(g_string_uninitialized_abort_jump, 1) == 0)
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

static int string_expect_no_sigabrt_uninitialized(void (*operation)(ft_string &))
{
    struct sigaction action;
    struct sigaction backup;
    int result;

    std::memset(&action, 0, sizeof(action));
    action.sa_handler = string_uninitialized_abort_handler;
    sigemptyset(&action.sa_mask);
    result = 0;
    if (sigaction(SIGABRT, &action, &backup) != 0)
        return (0);
    if (sigsetjmp(g_string_uninitialized_abort_jump, 1) == 0)
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

static void string_call_destructor_uninitialized(ft_string &string_value)
{
    string_value.~ft_string();
    return ;
}

static void string_call_destroy_uninitialized(ft_string &string_value)
{
    (void)string_value.destroy();
    return ;
}

static void string_call_append_uninitialized(ft_string &string_value)
{
    (void)string_value.append('x');
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

static void string_call_copy_ctor_from_uninitialized(void)
{
    alignas(ft_string) unsigned char storage[sizeof(ft_string)];
    ft_string *source_pointer;

    std::memset(storage, 0, sizeof(storage));
    source_pointer = reinterpret_cast<ft_string *>(storage);
    ft_string copy_value(*source_pointer);
    (void)copy_value;
    return ;
}

static void string_call_move_ctor_from_uninitialized(void)
{
    alignas(ft_string) unsigned char storage[sizeof(ft_string)];
    ft_string *source_pointer;

    std::memset(storage, 0, sizeof(storage));
    source_pointer = reinterpret_cast<ft_string *>(storage);
    ft_string moved_value(static_cast<ft_string &&>(*source_pointer));
    (void)moved_value;
    return ;
}

static void string_call_copy_assign_from_uninitialized(void)
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

static void string_call_move_assign_from_uninitialized(void)
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

FT_TEST(test_ft_string_uninitialized_destructor_noop,
    "ft_string destructor tolerates uninitialized instance")
{
    FT_ASSERT_EQ(1, string_expect_no_sigabrt_uninitialized(string_call_destructor_uninitialized));
    return (1);
}

FT_TEST(test_ft_string_uninitialized_destroy_noop,
    "ft_string destroy tolerates uninitialized instance")
{
    FT_ASSERT_EQ(1, string_expect_no_sigabrt_uninitialized(string_call_destroy_uninitialized));
    return (1);
}

FT_TEST(test_ft_string_uninitialized_append_aborts,
    "ft_string append aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_uninitialized(string_call_append_uninitialized));
    return (1);
}

FT_TEST(test_ft_string_initialize_twice_aborts,
    "ft_string initialize aborts when called while initialized")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_initialize_twice));
    return (1);
}

FT_TEST(test_ft_string_destroy_twice_noops,
    "ft_string destroy twice becomes a no-op")
{
    FT_ASSERT_EQ(0, string_expect_sigabrt_signal_handler(
        string_call_destroy_twice));
    return (1);
}

FT_TEST(test_ft_string_copy_ctor_from_uninitialized_source_aborts,
    "ft_string copy constructor aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_copy_ctor_from_uninitialized));
    return (1);
}

FT_TEST(test_ft_string_move_ctor_from_uninitialized_source_aborts,
    "ft_string move constructor aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_move_ctor_from_uninitialized));
    return (1);
}

FT_TEST(test_ft_string_copy_assign_from_uninitialized_source_aborts,
    "ft_string copy assignment aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_copy_assign_from_uninitialized));
    return (1);
}

FT_TEST(test_ft_string_move_assign_from_uninitialized_source_aborts,
    "ft_string move assignment aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
        string_call_move_assign_from_uninitialized));
    return (1);
}
