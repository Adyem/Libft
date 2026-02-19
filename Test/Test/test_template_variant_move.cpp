#include "../test_internal.hpp"
#include <string>
#include "../../Template/variant.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

template<typename T, typename... Types>
static T variant_instance_get(const ft_variant<Types...> &variant_instance)
{
    return (variant_instance.template get<T>());
}

FT_TEST(test_variant_move_constructor_rebuilds_mutex,
        "ft_variant enabling thread safety handles value retention")
{
    ft_variant<int, std::string> tuple_variant;

    FT_ASSERT_EQ(0, tuple_variant.enable_thread_safety());
    FT_ASSERT(tuple_variant.is_thread_safe());
    tuple_variant.emplace<int>(42);

    FT_ASSERT(tuple_variant.is_thread_safe());
    FT_ASSERT(tuple_variant.holds_alternative<int>());
    FT_ASSERT_EQ(42, variant_instance_get<int>(tuple_variant));

    FT_ASSERT_EQ(0, tuple_variant.enable_thread_safety());
    tuple_variant.emplace<std::string>("reset");
    FT_ASSERT(tuple_variant.holds_alternative<std::string>());
    FT_ASSERT_EQ(std::string("reset"), variant_instance_get<std::string>(tuple_variant));
    return (1);
}

FT_TEST(test_variant_move_assignment_rebuilds_mutex,
        "ft_variant assignment recreates mutex while replacing value")
{
    ft_variant<int, std::string> variant_instance;

    variant_instance.emplace<int>(8);
    FT_ASSERT_EQ(0, variant_instance.enable_thread_safety());
    FT_ASSERT(variant_instance.is_thread_safe());

    variant_instance.emplace<std::string>("text");

    std::string stored_string = variant_instance_get<std::string>(variant_instance);
    FT_ASSERT(variant_instance.is_thread_safe());
    FT_ASSERT(variant_instance.holds_alternative<std::string>());
    FT_ASSERT_EQ(std::string("text"), stored_string);

    FT_ASSERT_EQ(0, variant_instance.enable_thread_safety());
    variant_instance.emplace<int>(15);
    FT_ASSERT_EQ(true, variant_instance.holds_alternative<int>());
    FT_ASSERT_EQ(15, variant_instance_get<int>(variant_instance));
    return (1);
}

FT_TEST(test_variant_move_preserves_disabled_thread_safety,
        "ft_variant handles disabled thread safety when unused")
{
    ft_variant<int, std::string> source_variant;

    source_variant.emplace<int>(5);
    FT_ASSERT_EQ(false, source_variant.is_thread_safe());
    FT_ASSERT_EQ(5, variant_instance_get<int>(source_variant));
    return (1);
}
