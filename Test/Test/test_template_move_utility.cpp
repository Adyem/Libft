#include "../test_internal.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <memory>
#include <type_traits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_move_utility_produces_rvalue_reference)
{
    int32_t value;
    typedef decltype(ft_move(value)) moved_type;
    ft_bool is_expected_type;

    value = 7;
    is_expected_type = static_cast<ft_bool>(std::is_same<int32_t &&, moved_type>::value);
    FT_ASSERT_EQ(FT_TRUE, is_expected_type);

    int32_t &&rvalue_view = ft_move(value);
    rvalue_view = 21;
    FT_ASSERT_EQ(21, value);
    return (1);
}

FT_TEST(test_template_move_utility_preserves_const_qualification)
{
    const int32_t constant_value = 3;
    typedef decltype(ft_move(constant_value)) moved_type;
    ft_bool is_expected_type;

    is_expected_type = static_cast<ft_bool>(std::is_same<const int32_t &&, moved_type>::value);
    FT_ASSERT_EQ(FT_TRUE, is_expected_type);
    return (1);
}

FT_TEST(test_template_move_utility_transfers_unique_ptr_ownership)
{
    std::unique_ptr<int32_t> source_pointer(new int32_t(42));
    std::unique_ptr<int32_t> destination_pointer;

    FT_ASSERT(source_pointer.get() != ft_nullptr);
    destination_pointer = ft_move(source_pointer);
    FT_ASSERT_EQ(ft_nullptr, source_pointer.get());
    FT_ASSERT(destination_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(42, *destination_pointer);
    return (1);
}
