#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../GetNextLine/gnl_stream.hpp"
#include "../../SCMA/SCMA.hpp"
#include "../../Template/unordered_map.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <type_traits>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_unordered_map<int, int> unordered_map_int_int;

static int expect_sigabrt_for_void_operation(void (*operation)(void))
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

static void string_uninitialized_self_move_assignment_operation(void)
{
    alignas(ft_string) unsigned char storage[sizeof(ft_string)];
    ft_string *string_pointer;

    std::memset(storage, 0, sizeof(storage));
    string_pointer = reinterpret_cast<ft_string *>(storage);
    *string_pointer = static_cast<ft_string &&>(*string_pointer);
    return ;
}

static void unordered_map_uninitialized_self_move_assignment_operation(void)
{
    alignas(unordered_map_int_int) unsigned char storage[sizeof(unordered_map_int_int)];
    unordered_map_int_int *map_pointer;

    std::memset(storage, 0, sizeof(storage));
    map_pointer = reinterpret_cast<unordered_map_int_int *>(storage);
    *map_pointer = static_cast<unordered_map_int_int &&>(*map_pointer);
    return ;
}

static void scma_proxy_uninitialized_self_move_assignment_operation(void)
{
    typedef scma_handle_accessor_element_proxy<int> scma_proxy_type;
    alignas(scma_proxy_type) unsigned char storage[sizeof(scma_proxy_type)];
    scma_proxy_type *proxy_pointer;

    std::memset(storage, 0, sizeof(storage));
    proxy_pointer = reinterpret_cast<scma_proxy_type *>(storage);
    *proxy_pointer = static_cast<scma_proxy_type &&>(*proxy_pointer);
    return ;
}

FT_TEST(test_string_uninitialized_self_move_assignment_aborts,
    "ft_string move assignment self-move aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_for_void_operation(
        string_uninitialized_self_move_assignment_operation));
    return (1);
}

FT_TEST(test_unordered_map_uninitialized_self_move_assignment_aborts,
    "unordered_map move assignment self-move aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_for_void_operation(
        unordered_map_uninitialized_self_move_assignment_operation));
    return (1);
}

FT_TEST(test_scma_proxy_uninitialized_self_move_assignment_aborts,
    "scma accessor proxy move assignment self-move aborts when proxy is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt_for_void_operation(
        scma_proxy_uninitialized_self_move_assignment_operation));
    return (1);
}

FT_TEST(test_gnl_stream_move_operations_are_deleted,
    "gnl_stream move constructor and move assignment stay deleted")
{
    FT_ASSERT_EQ(false, std::is_move_constructible<gnl_stream>::value);
    FT_ASSERT_EQ(false, std::is_move_assignable<gnl_stream>::value);
    return (1);
}
