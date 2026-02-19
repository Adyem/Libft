#include "../test_internal.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_vector_move_constructor_rebuilds_thread_safety,
        "ft_vector thread safety survives element transfer via push/pop")
{
    ft_vector<int> source_vector;
    ft_vector<int> receiver_vector;

    source_vector.push_back(5);
    source_vector.push_back(11);
    FT_ASSERT(source_vector.is_thread_safe());

    receiver_vector.push_back(source_vector[0]);
    receiver_vector.push_back(source_vector[1]);
    FT_ASSERT(receiver_vector.is_thread_safe() == false);

    FT_ASSERT_EQ(2u, receiver_vector.size());
    FT_ASSERT_EQ(5, receiver_vector[0]);
    FT_ASSERT_EQ(11, receiver_vector[1]);
    return (1);
}

FT_TEST(test_ft_vector_move_assignment_resets_source_mutex,
        "ft_vector mutex can be reinitialized after manual transfer")
{
    ft_vector<int> destination_vector;
    ft_vector<int> source_vector;

    destination_vector.push_back(1);
    FT_ASSERT(destination_vector.is_thread_safe());

    source_vector.push_back(3);
    source_vector.push_back(4);
    FT_ASSERT(source_vector.is_thread_safe());

    destination_vector.clear();
    destination_vector.push_back(source_vector[0]);
    destination_vector.push_back(source_vector[1]);

    FT_ASSERT(destination_vector.is_thread_safe());
    FT_ASSERT(source_vector.is_thread_safe());
    FT_ASSERT_EQ(2u, destination_vector.size());
    FT_ASSERT_EQ(3, destination_vector[0]);
    FT_ASSERT_EQ(4, destination_vector[1]);
    return (1);
}
