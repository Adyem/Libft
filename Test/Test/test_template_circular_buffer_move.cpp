#include "../test_internal.hpp"
#include "../../Modules/Template/circular_buffer.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_circular_buffer_move_constructor_preserves_state)
{
    ft_circular_buffer<int> source_buffer(3);
    ft_circular_buffer<int> *moved_buffer_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.enable_thread_safety());
    source_buffer.push(11);
    source_buffer.push(22);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.get_error());
    FT_ASSERT_EQ(FT_TRUE, source_buffer.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.get_error());

    moved_buffer_pointer = new ft_circular_buffer<int>(3);
    FT_ASSERT(moved_buffer_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_buffer_pointer->move(source_buffer));
    FT_ASSERT_EQ(FT_TRUE, moved_buffer_pointer->is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_buffer_pointer->get_error());
    FT_ASSERT_EQ(11, moved_buffer_pointer->pop());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_buffer_pointer->get_error());
    FT_ASSERT_EQ(22, moved_buffer_pointer->pop());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_buffer_pointer->get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    source_buffer.push(33);
    FT_ASSERT_EQ(33, source_buffer.pop());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_buffer_pointer->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.destroy());
    delete moved_buffer_pointer;
    return (1);
}

FT_TEST(test_ft_circular_buffer_move_method_preserves_disabled_thread_safety)
{
    ft_circular_buffer<int> source_buffer(2);
    ft_circular_buffer<int> destination_buffer(2);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    source_buffer.push(1);
    source_buffer.push(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.get_error());
    FT_ASSERT_EQ(FT_FALSE, source_buffer.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.initialize());
    destination_buffer.push(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.move(source_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.get_error());
    FT_ASSERT_EQ(FT_FALSE, destination_buffer.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.get_error());
    FT_ASSERT_EQ(1, destination_buffer.pop());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.get_error());
    FT_ASSERT_EQ(2, destination_buffer.pop());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    source_buffer.push(7);
    FT_ASSERT_EQ(7, source_buffer.pop());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.destroy());
    return (1);
}
