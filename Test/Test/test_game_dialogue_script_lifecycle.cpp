#include "../test_internal.hpp"

#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_dialogue_script_thread_safe_lifecycle)
{
    game_dialogue_script script;
    ft_string title;
    ft_string summary;
    ft_vector<game_dialogue_line> lines;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, title.initialize("intro"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, summary.initialize("opening scene"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        script.initialize(5, title, summary, 1, lines));
    FT_ASSERT_EQ(5, script.get_script_id());
    FT_ASSERT(script.get_title() == title);
    FT_ASSERT(script.get_summary() == summary);
    FT_ASSERT_EQ(1, script.get_start_line_id());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), script.get_lines().size());
    script.set_script_id(8);
    script.set_start_line_id(3);
    FT_ASSERT_EQ(8, script.get_script_id());
    FT_ASSERT_EQ(3, script.get_start_line_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.enable_thread_safety());
    FT_ASSERT(script.is_thread_safe() == FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, script.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, title.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, summary.destroy());
    return (1);
}
