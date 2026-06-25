#include "../test_internal.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/GetNextLine/gnl_stream.hpp"
#include "../../Modules/SCMA/SCMA.hpp"
#include "../../Modules/Template/unordered_map.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <cstring>
#include <type_traits>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_unordered_map<int, int> unordered_map_int_int;

static int expect_sigabrt_for_void_operation(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static void string_uninitialised_self_move_assignment_operation(void)
{
    alignas(ft_string) unsigned char storage[sizeof(ft_string)];
    ft_string *string_pointer;

    std::memset(storage, 0, sizeof(storage));
    string_pointer = reinterpret_cast<ft_string *>(storage);
    *string_pointer = static_cast<ft_string &&>(*string_pointer);
    return ;
}

static void unordered_map_uninitialised_self_move_assignment_operation(void)
{
    alignas(unordered_map_int_int) unsigned char storage[sizeof(unordered_map_int_int)];
    unordered_map_int_int *map_pointer;

    std::memset(storage, 0, sizeof(storage));
    map_pointer = reinterpret_cast<unordered_map_int_int *>(storage);
    *map_pointer = static_cast<unordered_map_int_int &&>(*map_pointer);
    return ;
}

FT_TEST(test_string_uninitialised_self_move_assignment_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt_for_void_operation(
        string_uninitialised_self_move_assignment_operation));
    return (1);
}

FT_TEST(test_unordered_map_uninitialised_self_move_assignment_succeeds)
{
    FT_ASSERT_EQ(0, expect_sigabrt_for_void_operation(
        unordered_map_uninitialised_self_move_assignment_operation));
    return (1);
}

FT_TEST(test_scma_proxy_uninitialised_self_move_assignment_aborts)
{
    FT_ASSERT_EQ(false,
        std::is_move_assignable<scma_handle_accessor_element_proxy<int> >::value);
    return (1);
}

FT_TEST(test_gnl_stream_move_operations_are_deleted)
{
    FT_ASSERT_EQ(false, std::is_move_constructible<gnl_stream>::value);
    FT_ASSERT_EQ(false, std::is_move_assignable<gnl_stream>::value);
    return (1);
}
