#include "../test_internal.hpp"
#include "../../Modules/Template/unordered_map.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_unordered_map<int, int> unordered_map_int_int;
typedef unordered_map_int_int::iterator unordered_map_iterator;
typedef unordered_map_int_int::const_iterator unordered_map_const_iterator;

static int unordered_map_iterator_expect_sigabrt(void (*operation)())
{
    return (test_expect_sigabrt_signal(operation));
}

static unordered_map_int_int g_unordered_map_iterator_abort_map;
static unordered_map_iterator g_unordered_map_iterator_abort_iterator;
static unordered_map_int_int g_unordered_map_const_iterator_abort_map;
static unordered_map_const_iterator g_unordered_map_const_iterator_abort_iterator;

static void unordered_map_iterator_uninitialised_destructor_aborts_operation()
{
    unordered_map_iterator iterator_instance;

    (void)iterator_instance;
    return ;
}

static void unordered_map_iterator_destroy_uninitialised_aborts_operation()
{
    unordered_map_iterator iterator_instance;

    (void)iterator_instance.destroy();
    return ;
}

static void unordered_map_iterator_initialize_twice_aborts_operation()
{
    (void)g_unordered_map_iterator_abort_map.initialize();
    g_unordered_map_iterator_abort_map.insert(1, 10);
    g_unordered_map_iterator_abort_iterator.initialize(
        g_unordered_map_iterator_abort_map.begin());
    g_unordered_map_iterator_abort_iterator.initialize(
        g_unordered_map_iterator_abort_map.begin());
    return ;
}

static void unordered_map_iterator_destroy_twice_aborts_operation()
{
    unordered_map_int_int map_instance;
    unordered_map_iterator iterator_instance;

    (void)map_instance.initialize();
    map_instance.insert(1, 10);
    iterator_instance.initialize(map_instance.begin());
    (void)iterator_instance.destroy();
    (void)iterator_instance.destroy();
    (void)map_instance.destroy();
    return ;
}

static void unordered_map_iterator_move_self_uninitialised_aborts_operation()
{
    unordered_map_iterator iterator_instance;

    (void)iterator_instance.move(iterator_instance);
    return ;
}

static void unordered_map_const_iterator_uninitialised_destructor_aborts_operation()
{
    unordered_map_const_iterator iterator_instance;

    (void)iterator_instance;
    return ;
}

static void unordered_map_const_iterator_destroy_uninitialised_aborts_operation()
{
    unordered_map_const_iterator iterator_instance;

    (void)iterator_instance.destroy();
    return ;
}

static void unordered_map_const_iterator_initialize_twice_aborts_operation()
{
    (void)g_unordered_map_const_iterator_abort_map.initialize();
    g_unordered_map_const_iterator_abort_map.insert(1, 10);
    g_unordered_map_const_iterator_abort_iterator.initialize(
        static_cast<const unordered_map_int_int &>(
            g_unordered_map_const_iterator_abort_map).begin());
    g_unordered_map_const_iterator_abort_iterator.initialize(
        static_cast<const unordered_map_int_int &>(
            g_unordered_map_const_iterator_abort_map).begin());
    return ;
}

static void unordered_map_const_iterator_destroy_twice_aborts_operation()
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator iterator_instance;

    (void)map_instance.initialize();
    map_instance.insert(1, 10);
    iterator_instance.initialize(static_cast<const unordered_map_int_int &>(map_instance).begin());
    (void)iterator_instance.destroy();
    (void)iterator_instance.destroy();
    (void)map_instance.destroy();
    return ;
}

static void unordered_map_const_iterator_move_self_uninitialised_aborts_operation()
{
    unordered_map_const_iterator iterator_instance;

    (void)iterator_instance.move(iterator_instance);
    return ;
}

FT_TEST(test_unordered_map_iterator_destroy_and_reinitialize_copy_success)
{
    unordered_map_int_int map_instance;
    unordered_map_iterator source_iterator;
    unordered_map_iterator destination_iterator;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_iterator_destroy_and_reinitialize_move_success)
{
    unordered_map_int_int map_instance;
    unordered_map_iterator source_iterator;
    unordered_map_iterator destination_iterator;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_iterator_end_dereference_reports_out_of_range)
{
    unordered_map_int_int map_instance;
    unordered_map_iterator iterator_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    map_instance.insert(10, 100);
    iterator_instance.initialize(map_instance.end());
    (void)(*iterator_instance);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, iterator_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_iterator_end_arrow_reports_out_of_range)
{
    unordered_map_int_int map_instance;
    unordered_map_iterator iterator_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    map_instance.insert(10, 100);
    iterator_instance.initialize(map_instance.end());
    (void)iterator_instance.operator->();
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, iterator_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_destroy_and_reinitialize_copy_success)
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator source_iterator;
    unordered_map_const_iterator destination_iterator;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_destroy_and_reinitialize_move_success)
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator source_iterator;
    unordered_map_const_iterator destination_iterator;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_end_dereference_reports_out_of_range)
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator iterator_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    map_instance.insert(10, 100);
    iterator_instance.initialize(static_cast<const unordered_map_int_int &>(map_instance).end());
    (void)(*iterator_instance);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, iterator_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_end_arrow_reports_out_of_range)
{
    unordered_map_int_int map_instance;
    unordered_map_const_iterator iterator_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    map_instance.insert(10, 100);
    iterator_instance.initialize(static_cast<const unordered_map_int_int &>(map_instance).end());
    (void)iterator_instance.operator->();
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, iterator_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_iterator_uninitialised_destructor_succeeds)
{
    FT_ASSERT_EQ(0, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_uninitialised_destructor_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_iterator_destroy_uninitialised_succeeds)
{
    FT_ASSERT_EQ(0, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_destroy_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_iterator_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_initialize_twice_aborts_operation));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_unordered_map_iterator_abort_iterator.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_unordered_map_iterator_abort_map.destroy());
    return (1);
}

FT_TEST(test_unordered_map_iterator_destroy_twice_succeeds)
{
    FT_ASSERT_EQ(0, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_iterator_move_self_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_iterator_move_self_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_uninitialised_destructor_succeeds)
{
    FT_ASSERT_EQ(0, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_uninitialised_destructor_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_destroy_uninitialised_succeeds)
{
    FT_ASSERT_EQ(0, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_destroy_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_initialize_twice_aborts_operation));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_unordered_map_const_iterator_abort_iterator.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, g_unordered_map_const_iterator_abort_map.destroy());
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_destroy_twice_succeeds)
{
    FT_ASSERT_EQ(0, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_unordered_map_const_iterator_move_self_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, unordered_map_iterator_expect_sigabrt(
        unordered_map_const_iterator_move_self_uninitialised_aborts_operation));
    return (1);
}
