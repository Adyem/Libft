#include "../test_internal.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <type_traits>
#include <memory>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_move_produces_rvalue_reference,
    "ft_move casts lvalues to rvalue references")
{
    int value;
    typedef decltype(ft_move(value)) moved_type;
    bool is_rvalue_reference;
    value = 7;
    is_rvalue_reference = std::is_same<int &&, moved_type>::value;
    FT_ASSERT_EQ(true, is_rvalue_reference);
    int &&rvalue_view = ft_move(value);
    rvalue_view = 21;
    FT_ASSERT_EQ(21, value);
    return (1);
}

FT_TEST(test_ft_move_preserves_const_correctness,
    "ft_move preserves const qualifiers in reference types")
{
    const int constant_value = 3;
    typedef decltype(ft_move(constant_value)) moved_type;
    bool is_const_rvalue_reference;

    is_const_rvalue_reference = std::is_same<const int &&, moved_type>::value;
    FT_ASSERT_EQ(true, is_const_rvalue_reference);
    return (1);
}

FT_TEST(test_ft_move_transfers_unique_ptr_ownership,
    "ft_move enables moving unique_ptr instances")
{
    std::unique_ptr<int> source_pointer(new int(42));
    std::unique_ptr<int> destination_pointer;

    FT_ASSERT(source_pointer.get() != ft_nullptr);
    destination_pointer = ft_move(source_pointer);
    FT_ASSERT_EQ(ft_nullptr, source_pointer.get());
    FT_ASSERT(destination_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(42, *destination_pointer);
    return (1);
}
