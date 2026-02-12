#include "../test_internal.hpp"
#include <string>
#include "../../Template/variant.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_variant_move_constructor_rebuilds_mutex,
        "ft_variant move constructor rebuilds thread safety and transfers state")
{
    ft_variant<int, std::string> source_variant;

    FT_ASSERT_EQ(0, source_variant.enable_thread_safety());
    FT_ASSERT(source_variant.is_thread_safe_enabled());
    source_variant.emplace<int>(42);

    ft_variant<int, std::string> moved_variant(ft_move(source_variant));

    FT_ASSERT(moved_variant.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_variant.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, moved_variant.holds_alternative<int>());
    FT_ASSERT_EQ(42, moved_variant.get<int>());
    FT_ASSERT_EQ(0, source_variant.enable_thread_safety());
    source_variant.emplace<std::string>("reset");
    FT_ASSERT_EQ(true, source_variant.holds_alternative<std::string>());
    FT_ASSERT_EQ(std::string("reset"), source_variant.get<std::string>());
    return (1);
}

FT_TEST(test_variant_move_assignment_rebuilds_mutex,
        "ft_variant move assignment reinitializes thread safety and replaces value")
{
    ft_variant<int, std::string> destination_variant;
    ft_variant<int, std::string> source_variant;

    destination_variant.emplace<int>(8);
    FT_ASSERT_EQ(0, destination_variant.enable_thread_safety());
    FT_ASSERT(destination_variant.is_thread_safe_enabled());

    FT_ASSERT_EQ(0, source_variant.enable_thread_safety());
    source_variant.emplace<std::string>("text");

    destination_variant = ft_move(source_variant);

    FT_ASSERT(destination_variant.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_variant.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, destination_variant.holds_alternative<std::string>());
    FT_ASSERT_EQ(std::string("text"), destination_variant.get<std::string>());
    FT_ASSERT_EQ(0, source_variant.enable_thread_safety());
    source_variant.emplace<int>(15);
    FT_ASSERT_EQ(true, source_variant.holds_alternative<int>());
    FT_ASSERT_EQ(15, source_variant.get<int>());
    return (1);
}

FT_TEST(test_variant_move_preserves_disabled_thread_safety,
        "ft_variant move constructor keeps thread safety disabled when unused")
{
    ft_variant<int, std::string> source_variant;

    source_variant.emplace<int>(5);
    FT_ASSERT_EQ(false, source_variant.is_thread_safe_enabled());

    ft_variant<int, std::string> moved_variant(ft_move(source_variant));

    FT_ASSERT_EQ(false, moved_variant.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_variant.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, moved_variant.holds_alternative<int>());
    FT_ASSERT_EQ(5, moved_variant.get<int>());
    return (1);
}
