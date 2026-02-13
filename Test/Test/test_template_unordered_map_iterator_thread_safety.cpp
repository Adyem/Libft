#include "../test_internal.hpp"
#include "../../Template/unordered_map.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_unordered_map<int, int> unordered_map_int_int;
typedef unordered_map_int_int::iterator unordered_map_iterator;
typedef unordered_map_int_int::const_iterator unordered_map_const_iterator;

static int unordered_map_iterator_expect_sigabrt(void (*operation)())
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

static void unordered_map_iterator_uninitialized_destructor_aborts_operation()
{
    unordered_map_iterator iterator_instance;

    (void)iterator_instance;
    return ;
}

static void unordered_map_iterator_destroy_uninitialized_aborts_operation()
{
    unordered_map_iterator iterator_instance;

    (void)iterator_instance.destroy();
    return ;
}

static void unordered_map_iterator_initialize_twice_aborts_operation()
{
    unordered_map_int_int map_instance;
    unordered_map_iterator iterator_instance;

    map_instance.insert(1, 10);
    iterator_instance.initialize(map_instance.begin());
    iterator_instance.initialize(map_instance.begin());
    return ;
}

static void unordered_map_iterator_destroy_twice_aborts_operation()
{
    unordered_map_int_int map_instance;
    unordered_map_iterator iterator_instance;

    map_instance.insert(1, 10);
    iterator_instance.initialize(map_instance.begin());
    (void)iterator_instance.destroy();
    (void)iterator_instance.destroy();
    return ;
}

static void unordered_map_iterator_move_self_uninitialized_aborts_operation()
{
    unordered_map_iterator iterator_instance;

    (void)iterator_instance.move(iterator_instance);
    return ;
}

static void unordered_map_const_iterator_uninitialized_destructor_aborts_operation()
{
    unordered_map_const_iterator iterator_instance;

    (void)iterator_instance;
    return ;
}

static void unordered_map_const_iterator_destroy_uninitialized_aborts_operation()
{
    unordered_map_const_iterator iterator_instance;

    (void)iterator_instance.destroy();
    return ;
}

static void unordered_map_const_iterator_initialize_twice_aborts_operation()
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator iterator_instance;

    map_instance.insert(1, 10);
    iterator_instance.initialize(static_cast<const unordered_map_int_int &>(map_instance).begin());
    iterator_instance.initialize(static_cast<const unordered_map_int_int &>(map_instance).begin());
    return ;
}

static void unordered_map_const_iterator_destroy_twice_aborts_operation()
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator iterator_instance;

    map_instance.insert(1, 10);
    iterator_instance.initialize(static_cast<const unordered_map_int_int &>(map_instance).begin());
    (void)iterator_instance.destroy();
    (void)iterator_instance.destroy();
    return ;
}

static void unordered_map_const_iterator_move_self_uninitialized_aborts_operation()
{
    unordered_map_const_iterator iterator_instance;

    (void)iterator_instance.move(iterator_instance);
    return ;
}

FT_TEST(test_unordered_map_iterator_destroy_and_reinitialize_copy_success,
    "unordered_map iterator destroy then initialize(copy) succeeds")
{
    unordered_map_int_int map_instance;
    unordered_map_iterator source_iterator;
    unordered_map_iterator destination_iterator;

    map_instance.insert(1, 10);
    map_instance.insert(2, 20);
    source_iterator.initialize(map_instance.begin());
    destination_iterator.initialize(map_instance.end());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.initialize(source_iterator));
    FT_ASSERT_EQ(1, (*destination_iterator).first);
    FT_ASSERT_EQ(10, (*destination_iterator).second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_iterator.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.destroy());
    return (1);
}

FT_TEST(test_unordered_map_iterator_destroy_and_reinitialize_move_success,
    "unordered_map iterator destroy then initialize(move) succeeds")
{
    unordered_map_int_int map_instance;
    unordered_map_iterator source_iterator;
    unordered_map_iterator destination_iterator;

    map_instance.insert(4, 40);
    map_instance.insert(5, 50);
    source_iterator.initialize(map_instance.begin());
    destination_iterator.initialize(map_instance.end());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        destination_iterator.initialize(static_cast<unordered_map_iterator &&>(source_iterator)));
    FT_ASSERT_EQ(4, (*destination_iterator).first);
    FT_ASSERT_EQ(40, (*destination_iterator).second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.destroy());
    return (1);
}

FT_TEST(test_unordered_map_iterator_end_dereference_reports_out_of_range,
    "unordered_map iterator dereference on end reports out of range")
{
    unordered_map_int_int map_instance;
    unordered_map_iterator iterator_instance;

    map_instance.insert(10, 100);
    iterator_instance.initialize(map_instance.end());
    (void)(*iterator_instance);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, iterator_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_iterator_end_arrow_reports_out_of_range,
    "unordered_map iterator arrow on end reports out of range")
{
    unordered_map_int_int map_instance;
    unordered_map_iterator iterator_instance;

    map_instance.insert(10, 100);
    iterator_instance.initialize(map_instance.end());
    (void)iterator_instance.operator->();
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, iterator_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_destroy_and_reinitialize_copy_success,
    "unordered_map const_iterator destroy then initialize(copy) succeeds")
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator source_iterator;
    unordered_map_const_iterator destination_iterator;

    map_instance.insert(1, 10);
    map_instance.insert(2, 20);
    source_iterator.initialize(static_cast<const unordered_map_int_int &>(map_instance).begin());
    destination_iterator.initialize(static_cast<const unordered_map_int_int &>(map_instance).end());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.initialize(source_iterator));
    FT_ASSERT_EQ(1, (*destination_iterator).first);
    FT_ASSERT_EQ(10, (*destination_iterator).second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_iterator.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.destroy());
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_destroy_and_reinitialize_move_success,
    "unordered_map const_iterator destroy then initialize(move) succeeds")
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator source_iterator;
    unordered_map_const_iterator destination_iterator;

    map_instance.insert(4, 40);
    map_instance.insert(5, 50);
    source_iterator.initialize(static_cast<const unordered_map_int_int &>(map_instance).begin());
    destination_iterator.initialize(static_cast<const unordered_map_int_int &>(map_instance).end());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        destination_iterator.initialize(static_cast<unordered_map_const_iterator &&>(source_iterator)));
    FT_ASSERT_EQ(4, (*destination_iterator).first);
    FT_ASSERT_EQ(40, (*destination_iterator).second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.destroy());
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_end_dereference_reports_out_of_range,
    "unordered_map const_iterator dereference on end reports out of range")
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator iterator_instance;

    map_instance.insert(10, 100);
    iterator_instance.initialize(static_cast<const unordered_map_int_int &>(map_instance).end());
    (void)(*iterator_instance);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, iterator_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_end_arrow_reports_out_of_range,
    "unordered_map const_iterator arrow on end reports out of range")
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator iterator_instance;

    map_instance.insert(10, 100);
    iterator_instance.initialize(static_cast<const unordered_map_int_int &>(map_instance).end());
    (void)iterator_instance.operator->();
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, iterator_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_iterator_uninitialized_destructor_aborts,
    "unordered_map iterator destructor aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_uninitialized_destructor_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_iterator_destroy_uninitialized_aborts,
    "unordered_map iterator destroy aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_destroy_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_iterator_initialize_twice_aborts,
    "unordered_map iterator initialize aborts when called while initialized")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_iterator_destroy_twice_aborts,
    "unordered_map iterator destroy aborts when called after destroy")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_iterator_move_self_uninitialized_aborts,
    "unordered_map iterator move(self) aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_move_self_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_uninitialized_destructor_aborts,
    "unordered_map const_iterator destructor aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_uninitialized_destructor_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_destroy_uninitialized_aborts,
    "unordered_map const_iterator destroy aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_destroy_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_initialize_twice_aborts,
    "unordered_map const_iterator initialize aborts when called while initialized")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_destroy_twice_aborts,
    "unordered_map const_iterator destroy aborts when called after destroy")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_move_self_uninitialized_aborts,
    "unordered_map const_iterator move(self) aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_move_self_uninitialized_aborts_operation));
    return (1);
}
