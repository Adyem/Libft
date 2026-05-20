#include "../test_internal.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Template/move.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cpp_class_string_initialize_copy_move_paths)
{
    ft_string source_value;
    ft_string copy_value;
    ft_string moved_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("alpha"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy_value.initialize(source_value));
    FT_ASSERT_EQ(0, ft_strcmp("alpha", copy_value.c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_value.initialize(ft_move(source_value)));
    FT_ASSERT_EQ(0, ft_strcmp("alpha", moved_value.c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_string_initialize_replaces_existing_destination)
{
    ft_string source_value;
    ft_string destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize("source"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize("old"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT_EQ(0, ft_strcmp("source", destination_value.c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_string_copy_and_move_assignment)
{
    ft_string left_value;
    ft_string right_value;
    ft_string temporary_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.initialize("left"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.initialize("right"));
    left_value = right_value;
    FT_ASSERT_EQ(0, ft_strcmp("right", left_value.c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, temporary_value.initialize("moved"));
    right_value = ft_move(temporary_value);
    FT_ASSERT_EQ(0, ft_strcmp("moved", right_value.c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, temporary_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_string_operator_plus_and_plus_equal)
{
    ft_string left_value;
    ft_string right_value;
    ft_string combined_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.initialize("Hello"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.initialize("World"));
    left_value += " ";
    left_value += right_value;
    FT_ASSERT_EQ(0, ft_strcmp("Hello World", left_value.c_str()));
    combined_value = left_value + "!";
    FT_ASSERT_EQ(0, ft_strcmp("Hello World!", combined_value.c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, combined_value.destroy());
    return (1);
}
