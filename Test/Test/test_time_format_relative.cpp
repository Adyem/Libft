#include "../test_internal.hpp"
#include "../../Modules/Time/time.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_time_format_relative_labels_future_and_past_durations)
{
    ft_string *relative_text;

    relative_text = time_format_relative(time_duration_ms_create(5000));
    FT_ASSERT(relative_text != ft_nullptr);
    FT_ASSERT_STR_EQ("in PT5S", relative_text->c_str());
    relative_text->destroy();
    delete relative_text;
    relative_text = time_format_relative(time_duration_ms_create(-5000));
    FT_ASSERT(relative_text != ft_nullptr);
    FT_ASSERT_STR_EQ("-PT5S", relative_text->c_str());
    relative_text->destroy();
    delete relative_text;
    return (1);
}

FT_TEST(test_time_format_relative_handles_zero_and_large_durations)
{
    ft_string *relative_text;

    relative_text = time_format_relative(time_duration_ms_create(0));
    FT_ASSERT(relative_text != ft_nullptr);
    FT_ASSERT_STR_EQ("in PT0S", relative_text->c_str());
    relative_text->destroy();
    delete relative_text;
    relative_text = time_format_relative(time_duration_ms_create(INT64_MAX));
    FT_ASSERT(relative_text != ft_nullptr);
    FT_ASSERT(relative_text->size() > 0U);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, relative_text->get_error());
    relative_text->destroy();
    delete relative_text;
    return (1);
}
