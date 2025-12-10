#include "../../Template/optional.hpp"
#include "../../Template/variant.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstring>
#include <utility>

FT_TEST(test_ft_optional_reports_empty_state, "ft_optional reports empty state when no value is stored")
{
    ft_optional<int> optional_value;

    FT_ASSERT_EQ(false, optional_value.has_value());
    int &fallback_value = optional_value.value();

    FT_ASSERT_EQ(0, fallback_value);
    FT_ASSERT_EQ(FT_ERR_EMPTY, optional_value.get_error());
    return (1);
}

FT_TEST(test_ft_optional_holds_value_and_resets, "ft_optional stores values and clears them on reset")
{
    ft_optional<int> optional_value(42);

    FT_ASSERT(optional_value.has_value());
    FT_ASSERT_EQ(42, optional_value.value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, optional_value.get_error());
    optional_value.reset();
    FT_ASSERT_EQ(false, optional_value.has_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, optional_value.get_error());
    return (1);
}

FT_TEST(test_ft_optional_move_transfers_state, "ft_optional move assignment transfers ownership and clears source")
{
    ft_optional<int> source_optional(99);
    ft_optional<int> destination_optional;

    destination_optional = std::move(source_optional);
    FT_ASSERT(destination_optional.has_value());
    FT_ASSERT_EQ(99, destination_optional.value());
    FT_ASSERT_EQ(false, source_optional.has_value());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination_optional.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source_optional.get_error());
    return (1);
}

FT_TEST(test_ft_variant_emplace_and_get, "ft_variant emplace selects alternative and returns stored value")
{
    ft_variant<int, const char*> variant_value;

    variant_value.emplace<int>(17);
    FT_ASSERT(variant_value.holds_alternative<int>());
    FT_ASSERT_EQ(17, variant_value.get<int>());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, variant_value.get_error());
    variant_value.emplace<const char*>("hello");
    FT_ASSERT(variant_value.holds_alternative<const char*>());
    const char *string_value = variant_value.get<const char*>();
    FT_ASSERT_EQ(0, std::strcmp("hello", string_value));
    int wrong_access = variant_value.get<int>();
    FT_ASSERT_EQ(0, wrong_access);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, variant_value.get_error());
    return (1);
}

FT_TEST(test_ft_variant_visit_and_reset, "ft_variant visit dispatches to active alternative and handles reset")
{
    ft_variant<int, long> variant_value(12);
    long visit_sum = 0;

    variant_value.visit([&visit_sum](const auto &value){ visit_sum += value; });
    FT_ASSERT_EQ(12L, visit_sum);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, variant_value.get_error());
    variant_value.reset();
    FT_ASSERT_EQ(false, variant_value.holds_alternative<int>());
    visit_sum = 5;
    variant_value.visit([&visit_sum](const auto &value){ visit_sum += value; });
    FT_ASSERT_EQ(5L, visit_sum);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, variant_value.get_error());
    return (1);
}
