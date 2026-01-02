#include "../../Template/vector.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_vector_move_constructor_rebuilds_thread_safety,
        "ft_vector move constructor recreates mutex and moves elements")
{
    ft_vector<int> source_vector;

    source_vector.push_back(5);
    source_vector.push_back(11);
    FT_ASSERT(source_vector.is_thread_safe());

    ft_vector<int> moved_vector(ft_move(source_vector));

    FT_ASSERT(moved_vector.is_thread_safe());
    FT_ASSERT(source_vector.is_thread_safe());
    FT_ASSERT_EQ(2u, moved_vector.size());
    FT_ASSERT_EQ(5, moved_vector[0]);
    FT_ASSERT_EQ(11, moved_vector[1]);
    FT_ASSERT_EQ(0, moved_vector.get_error());
    return (1);
}

FT_TEST(test_ft_vector_move_assignment_resets_source_mutex,
        "ft_vector move assignment recreates mutex and replaces contents")
{
    ft_vector<int> destination_vector;
    ft_vector<int> source_vector;

    destination_vector.push_back(1);
    FT_ASSERT(destination_vector.is_thread_safe());

    source_vector.push_back(3);
    source_vector.push_back(4);
    FT_ASSERT(source_vector.is_thread_safe());

    destination_vector = ft_move(source_vector);

    FT_ASSERT(destination_vector.is_thread_safe());
    FT_ASSERT(source_vector.is_thread_safe());
    FT_ASSERT_EQ(2u, destination_vector.size());
    FT_ASSERT_EQ(3, destination_vector[0]);
    FT_ASSERT_EQ(4, destination_vector[1]);
    return (1);
}
