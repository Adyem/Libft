#include "../test_internal.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../Game/ft_dialogue_line.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static ft_sharedptr<ft_dialogue_line> make_line(int id, const char *speaker,
        const char *text)
{
    ft_vector<int> next;
    next.push_back(id + 1);
    ft_sharedptr<ft_dialogue_line> stored(new (std::nothrow) ft_dialogue_line());
    FT_ASSERT(stored.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stored->initialize(id, ft_string(speaker),
        ft_string(text), next));
    return (stored);
}

FT_TEST(test_dialogue_script_initial_state, "default dialogue script starts empty")
{
    ft_dialogue_script script;

    FT_ASSERT_EQ(0, script.get_script_id());
    FT_ASSERT_EQ(ft_string(), script.get_title());
    FT_ASSERT_EQ(ft_string(), script.get_summary());
    FT_ASSERT_EQ(0, script.get_start_line_id());
    FT_ASSERT_EQ(0u, script.get_lines().size());
    return (1);
}

FT_TEST(test_dialogue_script_setters_modify_metadata_and_lines, "setters mutate script metadata and lines")
{
    ft_dialogue_script script;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.initialize());
    script.set_script_id(4);
    script.set_title(ft_string("intro"));
    script.set_summary(ft_string("desc"));
    script.set_start_line_id(2);
    ft_vector<ft_sharedptr<ft_dialogue_line>> lines;
    lines.push_back(make_line(2, "npc", "reply"));
    script.set_lines(lines);

    const ft_vector<ft_sharedptr<ft_dialogue_line>> &stored = script.get_lines();
    FT_ASSERT_EQ(1u, stored.size());
    FT_ASSERT_EQ(2, stored[0]->get_line_id());
    FT_ASSERT_EQ(ft_string("reply"), stored[0]->get_text());
    return (1);
}
