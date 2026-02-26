#ifndef TEST_CPP_CLASS_STRING_LIFECYCLE_HELPERS_HPP
#define TEST_CPP_CLASS_STRING_LIFECYCLE_HELPERS_HPP

#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <csetjmp>
#include <csignal>
#include <cstring>

inline sigjmp_buf g_string_uninitialized_abort_jump;

inline void string_uninitialized_abort_handler(int /*signal_number*/)
{
    siglongjmp(g_string_uninitialized_abort_jump, 1);
    return ;
}

inline int string_expect_sigabrt_signal_handler(void (*operation)(void))
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

inline int string_expect_sigabrt_signal_handler(int (*operation)(void))
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
        (void)operation();
        result = 0;
    }
    else
        result = 1;
    (void)sigaction(SIGABRT, &backup, ft_nullptr);
    return (result);
}

inline int string_expect_sigabrt_uninitialized(void (*operation)(ft_string &))
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

inline int string_expect_no_sigabrt_uninitialized(void (*operation)(ft_string &))
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

#endif
