#include "../test_internal.hpp"
#include <string>
#include "../../Modules/Template/variant.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static int variant_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_variant_get_error_returned = FT_FALSE;
static int32_t g_variant_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_variant_get_error_str_returned = FT_FALSE;
static const char *g_variant_get_error_str_result = ft_nullptr;

template<typename T, typename... Types>
static T variant_instance_get(const ft_variant<Types...> &variant_instance)
{
    return (variant_instance.template get<T>());
}

static void variant_get_error_uninitialised_operation(void)
{
    ft_variant<int, std::string> variant_value;

    g_variant_get_error_result = variant_value.get_error();
    g_variant_get_error_returned = FT_TRUE;
    return ;
}

static void variant_get_error_str_uninitialised_operation(void)
{
    ft_variant<int, std::string> variant_value;

    g_variant_get_error_str_result = variant_value.get_error_str();
    g_variant_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_variant_move_constructor_rebuilds_mutex)
{
    ft_variant<int, std::string> source_variant;
    ft_variant<int, std::string> *moved_variant_pointer;

    FT_ASSERT_EQ(0, source_variant.initialize());
    FT_ASSERT_EQ(0, source_variant.enable_thread_safety());
    source_variant.emplace<std::string>("reset");
    FT_ASSERT(source_variant.is_thread_safe());

    moved_variant_pointer = new ft_variant<int, std::string>();
    FT_ASSERT(moved_variant_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_variant_pointer->move(source_variant));
    FT_ASSERT(moved_variant_pointer->is_thread_safe());
    FT_ASSERT(moved_variant_pointer->holds_alternative<std::string>());
    FT_ASSERT_EQ(std::string("reset"), variant_instance_get<std::string>(*moved_variant_pointer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_variant.initialize());
    source_variant.emplace<int>(7);
    FT_ASSERT_EQ(7, variant_instance_get<int>(source_variant));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_variant_pointer->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_variant.destroy());
    delete moved_variant_pointer;
    return (1);
}

FT_TEST(test_variant_move_assignment_rebuilds_mutex)
{
    ft_variant<int, std::string> source_variant;
    ft_variant<int, std::string> destination_variant;

    FT_ASSERT_EQ(0, source_variant.initialize());
    FT_ASSERT_EQ(0, source_variant.enable_thread_safety());
    source_variant.emplace<int>(15);
    FT_ASSERT(source_variant.is_thread_safe());

    FT_ASSERT_EQ(0, destination_variant.initialize());
    destination_variant.emplace<std::string>("text");
    FT_ASSERT_EQ(0, destination_variant.move(source_variant));
    FT_ASSERT(destination_variant.is_thread_safe());
    FT_ASSERT_EQ(FT_TRUE, destination_variant.holds_alternative<int>());
    FT_ASSERT_EQ(15, variant_instance_get<int>(destination_variant));

    FT_ASSERT_EQ(0, source_variant.initialize());
    source_variant.emplace<std::string>("reused");
    FT_ASSERT_EQ(FT_TRUE, source_variant.holds_alternative<std::string>());
    FT_ASSERT_EQ(std::string("reused"), variant_instance_get<std::string>(source_variant));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_variant.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_variant.destroy());
    return (1);
}

FT_TEST(test_variant_move_preserves_disabled_thread_safety)
{
    ft_variant<int, std::string> source_variant;
    ft_variant<int, std::string> *moved_variant_pointer;

    FT_ASSERT_EQ(0, source_variant.initialize());
    source_variant.emplace<int>(5);
    moved_variant_pointer = new ft_variant<int, std::string>();
    FT_ASSERT(moved_variant_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_variant_pointer->move(source_variant));
    FT_ASSERT_EQ(FT_FALSE, moved_variant_pointer->is_thread_safe());
    FT_ASSERT_EQ(5, variant_instance_get<int>(*moved_variant_pointer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_variant.initialize());
    source_variant.emplace<std::string>("reused");
    FT_ASSERT_EQ(std::string("reused"), variant_instance_get<std::string>(source_variant));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_variant_pointer->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_variant.destroy());
    delete moved_variant_pointer;
    return (1);
}

FT_TEST(test_variant_error_queries_follow_lifecycle_contract)
{
    ft_variant<int, std::string> variant_value;

    g_variant_get_error_returned = FT_FALSE;
    g_variant_get_error_result = FT_ERR_SUCCESS;
    g_variant_get_error_str_returned = FT_FALSE;
    g_variant_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, variant_expect_sigabrt(
        variant_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_variant_get_error_returned);
    FT_ASSERT_EQ(1, variant_expect_sigabrt(
        variant_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_variant_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, variant_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, variant_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, variant_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, variant_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(variant_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
