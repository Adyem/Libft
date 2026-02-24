#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static int string_expect_sigabrt(void (*operation)(void))
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
    if (!WIFSIGNALED(child_status))
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static int string_expect_sigabrt_uninitialized(void (*operation)(ft_string &))
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        alignas(ft_string) unsigned char storage[sizeof(ft_string)];
        ft_string *string_pointer;

        std::memset(storage, 0, sizeof(storage));
        string_pointer = reinterpret_cast<ft_string *>(storage);
        operation(*string_pointer);
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

static int string_expect_no_sigabrt_uninitialized(void (*operation)(ft_string &))
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        alignas(ft_string) unsigned char storage[sizeof(ft_string)];
        ft_string *string_pointer;

        std::memset(storage, 0, sizeof(storage));
        string_pointer = reinterpret_cast<ft_string *>(storage);
        operation(*string_pointer);
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (WIFSIGNALED(child_status))
        return (0);
    return (1);
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
    FT_ASSERT_EQ(1, string_expect_sigabrt(string_call_initialize_twice));
    return (1);
}

FT_TEST(test_ft_string_destroy_twice_aborts,
    "ft_string destroy aborts when called while destroyed")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt(string_call_destroy_twice));
    return (1);
}

FT_TEST(test_ft_string_copy_ctor_from_uninitialized_source_aborts,
    "ft_string copy constructor aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt(string_call_copy_ctor_from_uninitialized));
    return (1);
}

FT_TEST(test_ft_string_move_ctor_from_uninitialized_source_aborts,
    "ft_string move constructor aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt(string_call_move_ctor_from_uninitialized));
    return (1);
}

FT_TEST(test_ft_string_copy_assign_from_uninitialized_source_aborts,
    "ft_string copy assignment aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt(string_call_copy_assign_from_uninitialized));
    return (1);
}

FT_TEST(test_ft_string_move_assign_from_uninitialized_source_aborts,
    "ft_string move assignment aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt(string_call_move_assign_from_uninitialized));
    return (1);
}
