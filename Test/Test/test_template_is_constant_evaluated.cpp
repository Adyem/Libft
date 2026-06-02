#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <type_traits>
#include "../../Modules/Basic/class_nullptr.hpp"

static constexpr int constexpr_sum(int a, int b)
{
    return a + b;
}

constexpr bool g_compile_time_true = constexpr_sum(2, 3) == 5;
static_assert(g_compile_time_true, "constexpr addition should succeed");

FT_TEST(test_constexpr_sum_evaluates_at_compile_time)
{
    FT_ASSERT_EQ(5, constexpr_sum(2, 3));
    return (1);
}

FT_TEST(test_runtime_sum_matches_constexpr)
{
    int result = constexpr_sum(10, 20);
    FT_ASSERT_EQ(30, result);
    return (1);
}

FT_TEST(test_is_constant_expr_type_trait)
{
    constexpr bool same_type = std::is_same<int, int>::value;
    FT_ASSERT(same_type);
    return (1);
}
