#include "../../Template/deque.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_deque_move_constructor_clears_empty_error,
        "ft_deque move constructor clears empty error and keeps deque usable")
{
    ft_deque<int> source_deque;

    source_deque.pop_front();
    FT_ASSERT_EQ(FT_ERR_EMPTY, source_deque.get_error());

    ft_deque<int> moved_deque(ft_move(source_deque));

    FT_ASSERT_EQ(ER_SUCCESS, moved_deque.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source_deque.get_error());
    moved_deque.push_back(4);
    moved_deque.push_back(6);
    FT_ASSERT_EQ(4, moved_deque.pop_front());
    FT_ASSERT_EQ(6, moved_deque.pop_front());
    source_deque.push_back(8);
    FT_ASSERT_EQ(1u, source_deque.size());
    FT_ASSERT_EQ(8, source_deque.pop_back());
    FT_ASSERT_EQ(0, source_deque.get_error());
    return (1);
}

FT_TEST(test_ft_deque_move_assignment_clears_empty_error,
        "ft_deque move assignment clears empty error and allows reuse")
{
    ft_deque<int> destination_deque;
    ft_deque<int> source_deque;

    destination_deque.push_back(1);
    destination_deque.pop_back();
    FT_ASSERT_EQ(ER_SUCCESS, destination_deque.get_error());

    source_deque.pop_back();
    FT_ASSERT_EQ(FT_ERR_EMPTY, source_deque.get_error());

    destination_deque = ft_move(source_deque);

    FT_ASSERT_EQ(ER_SUCCESS, destination_deque.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source_deque.get_error());
    destination_deque.push_front(3);
    destination_deque.push_back(5);
    FT_ASSERT_EQ(3, destination_deque.pop_front());
    FT_ASSERT_EQ(5, destination_deque.pop_front());
    source_deque.push_front(7);
    source_deque.push_back(9);
    FT_ASSERT_EQ(7, source_deque.pop_front());
    FT_ASSERT_EQ(9, source_deque.pop_front());
    FT_ASSERT_EQ(0, destination_deque.get_error());
    FT_ASSERT_EQ(0, source_deque.get_error());
    return (1);
}
