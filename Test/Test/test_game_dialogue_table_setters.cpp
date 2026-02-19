#include "../test_internal.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/map.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dialogue_table_setters, "dialogue table setter swaps line maps")
{
    ft_dialogue_table table;
    ft_map<int, ft_sharedptr<ft_dialogue_line>> lines;
    ft_vector<int> followups;
    ft_dialogue_line fetched_line;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, lines.initialize());
    followups.push_back(3);
    lines.insert(2, ft_sharedptr<ft_dialogue_line>(new (std::nothrow)
            ft_dialogue_line(2, ft_string("npc"), ft_string("hi"), followups)));

    table.set_lines(lines);
    FT_ASSERT_EQ(1, table.get_lines().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_line(2, fetched_line));
    FT_ASSERT_EQ(ft_string("hi"), fetched_line.get_text());

    table.get_lines().clear();
    FT_ASSERT_EQ(0, table.get_lines().size());
    table.set_lines(lines);
    FT_ASSERT_EQ(0, table.get_lines().size());
    (void)lines.destroy();
    return (1);
}
