#include "../test_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include <type_traits>

static constexpr int constexpr_sum(int a, int b)
{
    return a + b;
}

constexpr bool g_compile_time_true = constexpr_sum(2, 3) == 5;
static_assert(g_compile_time_true, "constexpr addition should succeed");

FT_TEST(test_constexpr_sum_evaluates_at_compile_time,
        "constexpr helper runs during compilation")
{
    FT_ASSERT_EQ(5, constexpr_sum(2, 3));
    return (1);
}

FT_TEST(test_runtime_sum_matches_constexpr,
        "constexpr helper behaves the same at runtime")
{
    int result = constexpr_sum(10, 20);
    FT_ASSERT_EQ(30, result);
    return (1);
}

FT_TEST(test_is_constant_expr_type_trait,
        "std::is_same can be used in constexpr contexts")
{
    constexpr bool same_type = std::is_same<int, int>::value;
    FT_ASSERT(same_type);
    return (1);
}
