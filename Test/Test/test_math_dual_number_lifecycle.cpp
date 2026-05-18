#include "../test_internal.hpp"
#include "../../Modules/Math/ft_dual_number.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_dual_number_initialize_destroy_cycle)
{
    ft_dual_number value;

    FT_ASSERT_EQ(FT_CLASS_STATE_UNINITIALISED, value._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, value._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, value._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_math_dual_number_copy_and_move_initialise)
{
    ft_dual_number source;
    ft_dual_number copied;
    ft_dual_number moved;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(3.5, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied.initialize(source));
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, copied._initialised_state);
    FT_ASSERT_EQ(source.value(), copied.value());
    FT_ASSERT_EQ(source.derivative(), copied.derivative());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.initialize(static_cast<ft_dual_number &&>(source)));
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, moved._initialised_state);
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, source._initialised_state);
    return (1);
}

FT_TEST(test_math_dual_number_thread_safety_toggle)
{
    ft_dual_number value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(2.0, 0.0));
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_math_dual_number_proxy_chain_propagates_error)
{
    ft_dual_number numerator;
    ft_dual_number denominator;
    ft_dual_number *result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, numerator.initialize(4.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, denominator.initialize(0.0, 0.0));
    result = static_cast<ft_dual_number *>(numerator / denominator);
    FT_ASSERT(result != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, result->get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result->destroy());
    delete result;
    return (1);
}
