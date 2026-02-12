#include "../test_internal.hpp"
#include "../../Template/matrix.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_matrix_move_constructor_recreates_mutex,
        "ft_matrix move constructor rebuilds mutex and keeps values intact")
{
    ft_matrix<int> source_matrix(2, 2);

    FT_ASSERT_EQ(0, source_matrix.enable_thread_safety());
    FT_ASSERT(source_matrix.is_thread_safe());
    source_matrix.at(0, 0) = 3;
    source_matrix.at(0, 1) = 4;
    source_matrix.at(1, 0) = 7;
    source_matrix.at(1, 1) = 9;

    ft_matrix<int> moved_matrix(ft_move(source_matrix));

    FT_ASSERT(moved_matrix.is_thread_safe());
    FT_ASSERT_EQ(false, source_matrix.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_matrix.rows());
    FT_ASSERT_EQ(2UL, moved_matrix.cols());
    FT_ASSERT_EQ(3, moved_matrix.at(0, 0));
    FT_ASSERT_EQ(4, moved_matrix.at(0, 1));
    FT_ASSERT_EQ(7, moved_matrix.at(1, 0));
    FT_ASSERT_EQ(9, moved_matrix.at(1, 1));
    FT_ASSERT_EQ(0, moved_matrix.get_error());
    return (1);
}

FT_TEST(test_ft_matrix_move_assignment_allows_reenabling_source,
        "ft_matrix move assignment recreates mutex and leaves source reusable")
{
    ft_matrix<int> destination_matrix(1, 1);
    ft_matrix<int> source_matrix(2, 2);

    destination_matrix.at(0, 0) = 5;
    FT_ASSERT_EQ(0, destination_matrix.enable_thread_safety());
    FT_ASSERT(destination_matrix.is_thread_safe());

    source_matrix.at(0, 0) = 8;
    source_matrix.at(0, 1) = 6;
    source_matrix.at(1, 0) = 1;
    source_matrix.at(1, 1) = 2;
    FT_ASSERT_EQ(0, source_matrix.enable_thread_safety());
    FT_ASSERT(source_matrix.is_thread_safe());

    destination_matrix = ft_move(source_matrix);

    FT_ASSERT(destination_matrix.is_thread_safe());
    FT_ASSERT_EQ(false, source_matrix.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_matrix.rows());
    FT_ASSERT_EQ(2UL, destination_matrix.cols());
    FT_ASSERT_EQ(8, destination_matrix.at(0, 0));
    FT_ASSERT_EQ(6, destination_matrix.at(0, 1));
    FT_ASSERT_EQ(1, destination_matrix.at(1, 0));
    FT_ASSERT_EQ(2, destination_matrix.at(1, 1));
    FT_ASSERT_EQ(0, source_matrix.enable_thread_safety());
    FT_ASSERT(source_matrix.is_thread_safe());
    return (1);
}
