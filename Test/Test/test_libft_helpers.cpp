#include "../test_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include <type_traits>

#ifndef LIBFT_TEST_BUILD
#endif

static constexpr int compile_time_product(int a, int b)
{
    return a * b;
}

FT_TEST(test_compile_time_product_matches_runtime,
        "constexpr multiplication agrees with runtime result")
{
    FT_ASSERT_EQ(12, compile_time_product(3, 4));
    int runtime_result = compile_time_product(5, 6);
    FT_ASSERT_EQ(30, runtime_result);
    return (1);
}

FT_TEST(test_type_trait_in_constexpr_domain,
        "std::is_integral is constexpr-evaluable")
{
    FT_ASSERT(std::is_integral<int>::value);
    return (1);
}
